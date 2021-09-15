// Fill out your copyright notice in the Description page of Project Settings.
#include "AzureKinectDevice.h"
#include "Runtime/RHI/Public/RHI.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "AzureKinectSkeltonAnim.h"

DEFINE_LOG_CATEGORY(AzureKinectDeviceLog);

UAzureKinectDevice::UAzureKinectDevice() :
	NativeDevice(nullptr),
	Thread(nullptr),
	DeviceIndex(-1),
	bOpen(false),
	NumTrackedBodies(0)
{
	LoadDevice();
}

UAzureKinectDevice::UAzureKinectDevice(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	LoadDevice();
}

void UAzureKinectDevice::LoadDevice()
{
	
	int32 NumKinect = GetNumConnectedDevices();
	
	DeviceList.Empty(NumKinect + 1);
	DeviceList.Add(MakeShared<FString>("No Device"));

	if (NumKinect > 0)
	{
		for (int32 i = 0; i < NumKinect; i++)
		{
			try
			{
				// Open connection to the device.
				k4a::device Device = k4a::device::open(i);
				// Get and store the device serial number
				DeviceList.Add(MakeShared<FString>(Device.get_serialnum().c_str()));
				Device.close();
			}
			catch (const k4a::error& Err)
			{
				UE_LOG(AzureKinectDeviceLog, Error, TEXT("Can't load: %s"), TCHAR_TO_UTF8(ANSI_TO_TCHAR(Err.what())));
			}
		}
	}
	
}

void UAzureKinectDevice::StartDevice()
{
	if (bOpen)
	{
		UE_LOG(AzureKinectDeviceLog, Warning, TEXT("This Device has been open."));
		return;
	}

	
	if (DeviceIndex == -1)
	{
		UE_LOG(AzureKinectDeviceLog, Warning, TEXT("No Device is selected."));
		return;
	}

	CalcFrameCount();

	try
	{
		// Open connection to the device.
		NativeDevice = k4a::device::open(DeviceIndex);

		// Start the Camera and make sure the Depth Camera is Enabled
		k4a_device_configuration_t DeviceConfig = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
		DeviceConfig.depth_mode = static_cast<k4a_depth_mode_t>(DepthMode);
		DeviceConfig.color_resolution = static_cast<k4a_color_resolution_t>(ColorMode);
		DeviceConfig.camera_fps = static_cast<k4a_fps_t>(Fps);
		DeviceConfig.color_format = k4a_image_format_t::K4A_IMAGE_FORMAT_COLOR_BGRA32;
		DeviceConfig.synchronized_images_only = true;
		DeviceConfig.wired_sync_mode = K4A_WIRED_SYNC_MODE_STANDALONE;

		NativeDevice.start_cameras(&DeviceConfig);

		KinectCalibration = NativeDevice.get_calibration(DeviceConfig.depth_mode, DeviceConfig.color_resolution);
		KinectTransformation = k4a::transformation(KinectCalibration);

		k4abt_tracker_configuration_t TrackerConfig = K4ABT_TRACKER_CONFIG_DEFAULT;
		TrackerConfig.sensor_orientation = static_cast<k4abt_sensor_orientation_t>(SensorOrientation);

		// Retain body tracker
		BodyTracker = k4abt::tracker::create(KinectCalibration, TrackerConfig);
	}
	catch (const k4a::error& Err)
	{
		if (NativeDevice)
		{
			NativeDevice.close();
		}

		FString Msg(ANSI_TO_TCHAR(Err.what()));
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("Cant't open: %s"), *Msg);
		return;
	}
	
	Thread = new FAzureKinectDeviceThread(this);

	bOpen = true;
}

void UAzureKinectDevice::StopDevice()
{

	if (!bOpen)
	{
		UE_LOG(AzureKinectDeviceLog, Warning, TEXT("KinectDevice is not running."));
		return;
	}

	if (Thread)
	{
		Thread->EnsureCompletion();
		Thread = nullptr;
	}

	if (NativeDevice)
	{
		NativeDevice.stop_cameras();
		NativeDevice.close();
		NativeDevice = nullptr;
		UE_LOG(AzureKinectDeviceLog, Verbose, TEXT("KinectDevice Camera is Stopped and Closed"));
	}

	bOpen = false;

}

int32 UAzureKinectDevice::GetNumConnectedDevices()
{
	return k4a_device_get_installed_count();
}

int32 UAzureKinectDevice::GetNumTrackedBodies() const
{
	return NumTrackedBodies;
}

TArray<FTransform> UAzureKinectDevice::GetSkeltonJoints(int32 BodyIndex) const
{
	if (BodyIndex >= TrackedJoints.Num())
	{
		return TArray<FTransform>();
	}
	else
	{
		return TrackedJoints[BodyIndex];
	}
	
}

void UAzureKinectDevice::Update()
{
	// Threaded function
	try
	{
		if (!NativeDevice.get_capture(&Capture, FrameTime))
		{
			UE_LOG(AzureKinectDeviceLog, Verbose, TEXT("Timed out waiting for capture."));
		}
	}
	catch (const k4a::error& Err)
	{
		FString Msg(ANSI_TO_TCHAR(Err.what()));
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("Can't capture frame: %s"), *Msg);
		return;
	}

	if (ColorTexture)
	{
		CaptureColorImage();
	}

	if (DepthTexture)
	{
		CaptureDepthImage();
	}
	
	if (InflaredTexture)
	{
		CaptureInflaredImage();
	}

	if (true)
	{
		UpdateSkeltons();
	}

	Capture.reset();

}

void UAzureKinectDevice::CaptureColorImage()
{
	k4a::image ColorCapture = Capture.get_color_image();
	if (!ColorCapture.is_valid()) return;
	
	int32 Width = ColorCapture.get_width_pixels(), Height = ColorCapture.get_height_pixels();
	
	if (ColorTexture->GetSurfaceWidth() != Width || ColorTexture->GetSurfaceHeight() != Height)
	{
		ColorTexture->InitCustomFormat(Width, Height, EPixelFormat::PF_B8G8R8A8, false);
		ColorTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		ColorTexture->UpdateResource();
	}	
	else
	{
		const uint8* SrcData = ColorCapture.get_buffer();
		FTextureResource* TextureResource = ColorTexture->Resource;
		auto Region = FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);

		ENQUEUE_RENDER_COMMAND(UpdateTextureData)(
			[TextureResource, Region, SrcData](FRHICommandListImmediate& RHICmdList) {
				FTexture2DRHIRef Texture2D = TextureResource->TextureRHI ? TextureResource->TextureRHI->GetTexture2D() : nullptr;
				if (!Texture2D)
				{
					return;
				}
				RHIUpdateTexture2D(Texture2D, 0, Region, 4 * Region.Width, SrcData);
			});
	}

	ColorCapture.reset();

}

void UAzureKinectDevice::CaptureDepthImage()
{
	k4a::image DepthCapture = Capture.get_depth_image();
	k4a::image ColorCapture = Capture.get_color_image();

	if (!DepthCapture.is_valid() || !ColorCapture.is_valid()) return;

	int32 Width = ColorCapture.get_width_pixels(), Height = ColorCapture.get_height_pixels();
	
	if (!DepthRemapped || !DepthRemapped.is_valid())
	{
		DepthRemapped = k4a::image::create(K4A_IMAGE_FORMAT_DEPTH16, Width, Height, Width * static_cast<int>(sizeof(uint16_t)));
	}

	try
	{
		KinectTransformation.depth_image_to_color_camera(DepthCapture, &DepthRemapped);
	}
	catch (const k4a::error& Err)
	{
		FString Msg(ANSI_TO_TCHAR(Err.what()));
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("Cant't transform Depth to Color: %s"), *Msg);
		return;
	}

	if (DepthTexture->GetSurfaceWidth() != Width || DepthTexture->GetSurfaceHeight() != Height)
	{
		DepthTexture->InitCustomFormat(Width, Height, EPixelFormat::PF_R8G8B8A8, true);
		DepthTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		DepthTexture->UpdateResource();
	}
	else
	{
		uint8* S = DepthRemapped.get_buffer();
		TArray<uint8> SrcData;
		SrcData.Reset(Width * Height * 4);
		for (int hi = 0; hi < Height; hi++)
		{
			for (int wi = 0; wi < Width; wi++)
			{
				int index = hi * Width + wi;
				uint16 R = S[index * 2];
				uint16 G = S[index * 2 + 1];
				
				uint16 Sample = R << 8 | G;
				if (Sample > 0)
				{
					SrcData.Push(S[index * 2]);
					SrcData.Push(S[index * 2 + 1]);
					SrcData.Push(0);
					SrcData.Push(255);
				}
				else
				{
					SrcData.Push(S[index * 2]);
					SrcData.Push(S[index * 2 + 1]);
					SrcData.Push(255);
					SrcData.Push(255);
				}	
			}
		}
		
		FTextureResource* TextureResource = DepthTexture->Resource;
		auto Region = FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);

		ENQUEUE_RENDER_COMMAND(UpdateTextureData)(
			[TextureResource, Region, SrcData](FRHICommandListImmediate& RHICmdList) {
				FTexture2DRHIRef Texture2D = TextureResource->TextureRHI ? TextureResource->TextureRHI->GetTexture2D() : nullptr;
				if (!Texture2D)
				{
					return;
				}
				RHIUpdateTexture2D(Texture2D, 0, Region, 4 * Region.Width, SrcData.GetData());

			});
	}

	DepthCapture.reset();
	ColorCapture.reset();

}

void UAzureKinectDevice::CaptureInflaredImage()
{
	const k4a::image& InflaredCapture = Capture.get_ir_image();
	if (!InflaredCapture.is_valid()) return;

	int32 Width = InflaredCapture.get_width_pixels(), Height = InflaredCapture.get_height_pixels();

	if (InflaredTexture->GetSurfaceWidth() != Width || InflaredTexture->GetSurfaceWidth() != Height)
	{
		InflaredTexture->InitCustomFormat(Width, Height, EPixelFormat::PF_R8G8B8A8, true);
		InflaredTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		InflaredTexture->UpdateResource();
	}
	else
	{
		const uint8* S = InflaredCapture.get_buffer();
		TArray<uint8> SrcData;
		SrcData.Reset(Width * Height * 4);
		for (int hi = 0; hi < Height; hi++)
		{
			for (int wi = 0; wi < Width; wi++)
			{
				int index = hi * Width + wi;

				if (S[index * 2] + S[index * 2 + 1] > 0)
				{
					SrcData.Push(S[index * 2]);
					SrcData.Push(S[index * 2 + 1]);
					SrcData.Push(0);
					SrcData.Push(255);
				}
				else
				{
					SrcData.Push(0);
					SrcData.Push(0);
					SrcData.Push(255);
					SrcData.Push(255);
				}
			}
		}
		
		FTextureResource* TextureResource = InflaredTexture->Resource;
		auto Region = FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);

		ENQUEUE_RENDER_COMMAND(UpdateTextureData)(
			[TextureResource, Region, SrcData](FRHICommandListImmediate& RHICmdList) {
				FTexture2DRHIRef Texture2D = TextureResource->TextureRHI ? TextureResource->TextureRHI->GetTexture2D() : nullptr;
				if (!Texture2D)
				{
					return;
				}

				RHIUpdateTexture2D(Texture2D, 0, Region, 2 * Region.Width, SrcData.GetData());
			});
	}
	
}

bool UAzureKinectDevice::ValidateSkelton(TSoftObjectPtr<ASkeletalMeshActor> Skelton)
{
	if (!Skelton)
	{
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("Skelton is null."));
		return false;
	}

	USkeletalMeshComponent* Component = Skelton->GetSkeletalMeshComponent();
	if (Component->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
	{
		FString Name = Skelton->GetName();
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("Skelton %s Animation mode is not AnimBP."), *Name);
		return false;
	}
	UAnimInstance* AnimInstance = Component->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("AnimInstance is null."));
		return false;
	}

	UAzureKinectSkeltonAnim* AzureKinectAnim = Cast<UAzureKinectSkeltonAnim>(AnimInstance);
	if (!AzureKinectAnim)
	{
		FString SkeltonName = Skelton->GetName();
		FString AnimClassName = AnimInstance->GetName();

		UE_LOG(AzureKinectDeviceLog, Error, 
			TEXT("Skelton's AnimBP ( %s [ %s ] ) is not inherited from AzureKinectSkeltonAnim."),
			*SkeltonName, *AnimClassName);
		return false;
	}
	
	return true;
}

void UAzureKinectDevice::UpdateSkeltons()
{
	if (SkeltalMeshes.Num() > 0)
	{
		k4abt::frame BodyFrame = nullptr;
		TArray<int32> BodyIDs;
		
		try
		{
			if (!BodyTracker.enqueue_capture(Capture, FrameTime))
			{
				UE_LOG(AzureKinectDeviceLog, Warning, TEXT("Failed adding capture to tracker process queue"));
				return;
			}
			
			if (!BodyTracker.pop_result(&BodyFrame, FrameTime))
			{
				UE_LOG(AzureKinectDeviceLog, Warning, TEXT("Failed Tracker pop body frame"));
				return;
			}
		}
		catch (const k4a::error& Err)
		{
			FString Msg(ANSI_TO_TCHAR(Err.what()));
			UE_LOG(AzureKinectDeviceLog, Error, TEXT("Couldn't get Body Frame: %s"), *Msg);
		}

		int32 NumBodies = BodyFrame.get_num_bodies();

		TArray<TArray<FTransform>> BodyJoints;
		BodyJoints.Empty(NumBodies);
		// UE_LOG(AzureKinectDeviceLog, Verbose, TEXT("%d body(s) deteceted"), NumTrackedBodies);

		for (int32 i = 0; i < FMath::Min(NumBodies, SkeltalMeshes.Num()); i++)
		{

			k4abt_body_t Body;
			try
			{
				BodyFrame.get_body_skeleton(i, Body.skeleton);
				Body.id = BodyFrame.get_body_id(i);
			}
			catch (const k4a::error& Err)
			{
				FString Msg(ANSI_TO_TCHAR(Err.what()));
				UE_LOG(AzureKinectDeviceLog, Error, TEXT("Couldn't get Body Skeleton: %s"), *Msg);
				continue;
			}

			{
				TArray<FTransform> Joints;
				Joints.Empty(K4ABT_JOINT_COUNT);
				for (int32 j = 0; j < K4ABT_JOINT_COUNT; j++)
				{
					Joints.Push(JointToTransform(Body.skeleton.joints[j], j));
				}
				
				BodyJoints.Push(Joints);
				BodyIDs.Push(Body.id);
			}

		}
		
		BodyFrame.reset();
		
		{
			// Assure updating AnimInstance is thread-safe
			FScopeLock Lock(Thread->GetCriticalSection());

			TrackedJoints.Empty();
			int32 i = 0;
			for (; i < FMath::Min(NumBodies, SkeltalMeshes.Num()); i++)
			{
				if (!ValidateSkelton(SkeltalMeshes[i]))
				{
					continue;
				}
				UAzureKinectSkeltonAnim* Anim = Cast<UAzureKinectSkeltonAnim>(SkeltalMeshes[i]->GetSkeletalMeshComponent()->GetAnimInstance());
				Anim->SetBodyID(BodyIDs[i]);
				Anim->SetJoints(BodyJoints[i]);

				TrackedJoints.Push(BodyJoints[i]);
			}
			NumTrackedBodies = i;
		}

	}
}

FTransform UAzureKinectDevice::JointToTransform(const k4abt_joint_t& Joint, int32 Index)
{

	/**
	 * Convert Azure Kinect Depth and Color camera co-ordinate system
	 * to Unreal co-ordinate system
	 * @see https://docs.microsoft.com/en-us/azure/kinect-dk/coordinate-systems
	 *
	 * Kinect [mm]				Unreal [cm]
	 * --------------------------------------
	 * +ve X-axis		Right		+ve Y-axis
	 * +ve Y-axis		Down		-ve Z-axis
	 * +ve Z-axis		Forward		+ve X-axis
	*/
	FVector Position(Joint.position.xyz.x, Joint.position.xyz.z, - Joint.position.xyz.y);
	Position *= 0.1f;

	/**
	 * Convert the Orientation from Kinect co-ordinate system to Unreal co-ordinate system.
	 * We negate the x, y components of the JointQuaternion since we are converting from
	 * Kinect's Right Hand orientation to Unreal's Left Hand orientation.
	 */
	FQuat Quat(
		- Joint.orientation.wxyz.z,
		- Joint.orientation.wxyz.x,
		Joint.orientation.wxyz.y,
		Joint.orientation.wxyz.w
	);
	
	Quat = AlignJointOrientation(Quat, Index);

	
	return FTransform(Quat, Position);
}

FQuat UAzureKinectDevice::AlignJointOrientation(const FQuat& Quat, int32 Index)
{
	/**
	 * Map the Azure Kinect joint orientation to Unreal Mannequin.
	 * @see https://docs.microsoft.com/en-us/azure/kinect-dk/body-joints
	 *
	 * The pelvis, spine, chest, legs, neck and head joints extend along X-axis which is
	 * along the Z-axis in UE4 and the joints forward axis (Y-axis) is along the X-axis in UE4.
	 *
	 * The shoulder joints extend along X-axis, their forward axis (Y-axis) is along the
	 * X-axis in UE4 and their up axis (Z-axis) is along Z-axis in UE4.
	 *
	 * The elbow joints are similar to the shoulders, but the same mapping didn't work for them.
	 *
	 * The wrist joints are a bit tricky and I couldn't get them to work correctly.
	*/
#define JOINT_ID(Name) static_cast<uint8>(EKinectBodyJoint::Name)

	if (
		Index <= JOINT_ID(NECK) ||
		(Index >= JOINT_ID(HEAD) && Index <= JOINT_ID(EAR_RIGHT)) ||
		(Index >= JOINT_ID(HIP_LEFT) && Index <= JOINT_ID(FOOT_LEFT)) ||
		(Index >= JOINT_ID(HIP_RIGHT) && Index <= JOINT_ID(FOOT_RIGHT))
		)
	{
		return FRotationMatrix::MakeFromXZ(Quat.GetAxisY(), - Quat.GetAxisX()).ToQuat();
	}
	else if (
		Index == JOINT_ID(CLAVICLE_LEFT) ||
		Index == JOINT_ID(CLAVICLE_RIGHT) ||
		Index == JOINT_ID(SHOULDER_LEFT) ||
		Index == JOINT_ID(SHOULDER_RIGHT))
	{
		return FRotationMatrix::MakeFromXZ(Quat.GetAxisY(), Quat.GetAxisZ()).ToQuat();
	}
	else if (
		Index == JOINT_ID(ELBOW_LEFT) || 
		Index == JOINT_ID(WRIST_LEFT) ||
		Index == JOINT_ID(ELBOW_RIGHT) ||
		Index == JOINT_ID(WRIST_RIGHT))
	{
		return FRotationMatrix::MakeFromXZ(Quat.GetAxisY(), - Quat.GetAxisZ()).ToQuat();
	}
	
	return Quat;

#undef JOINT_ID
}

void UAzureKinectDevice::CalcFrameCount()
{
	float FrameTimeInMilli = 0.0f;
	switch (Fps)
	{
	case EKinectFps::PER_SECOND_5:
		FrameTimeInMilli = 1000.f / 5.f;
		break;
	case EKinectFps::PER_SECOND_15:
		FrameTimeInMilli = 1000.f / 15.f;
		break;
	case EKinectFps::PER_SECOND_30:
		FrameTimeInMilli = 1000.f / 30.f;
		break;
	default:
		break;
	}
	FrameTime = std::chrono::milliseconds(FGenericPlatformMath::CeilToInt(FrameTimeInMilli));
}
