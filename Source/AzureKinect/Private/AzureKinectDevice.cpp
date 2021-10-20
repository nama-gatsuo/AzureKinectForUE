// Fill out your copyright notice in the Description page of Project Settings.
#include "AzureKinectDevice.h"
#include "Runtime/RHI/Public/RHI.h"

DEFINE_LOG_CATEGORY(AzureKinectDeviceLog);

UAzureKinectDevice::UAzureKinectDevice() :
	NativeDevice(nullptr),
	Thread(nullptr),
	DeviceIndex(-1),
	bOpen(false),
	NumTrackedSkeletons(0),
	DepthMode(EKinectDepthMode::NFOV_2X2BINNED),
	ColorMode(EKinectColorResolution::RESOLUTION_720P),
	Fps(EKinectFps::PER_SECOND_30),
	SensorOrientation(EKinectSensorOrientation::DEFAULT),
	bSkeletonTracking(false)
{
	LoadDevices();
}

UAzureKinectDevice::UAzureKinectDevice(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	LoadDevices();
}

void UAzureKinectDevice::LoadDevices()
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

bool UAzureKinectDevice::StartDevice()
{
	if (bOpen)
	{
		UE_LOG(AzureKinectDeviceLog, Warning, TEXT("This Device has been open."));
		return false;
	}

	
	if (DeviceIndex == -1)
	{
		UE_LOG(AzureKinectDeviceLog, Warning, TEXT("No Device is selected."));
		return false;
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

		if (bSkeletonTracking)
		{
			k4abt_tracker_configuration_t TrackerConfig = K4ABT_TRACKER_CONFIG_DEFAULT;
			TrackerConfig.sensor_orientation = static_cast<k4abt_sensor_orientation_t>(SensorOrientation);

			// Retain body tracker
			BodyTracker = k4abt::tracker::create(KinectCalibration, TrackerConfig);
		}
		
	}
	catch (const k4a::error& Err)
	{
		if (NativeDevice)
		{
			NativeDevice.close();
		}

		FString Msg(ANSI_TO_TCHAR(Err.what()));
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("Cant't open: %s"), *Msg);
		return false;
	}
	
	Thread = new FAzureKinectDeviceThread(this);

	bOpen = true;

	return true;
}

bool UAzureKinectDevice::StopDevice()
{

	if (!bOpen)
	{
		UE_LOG(AzureKinectDeviceLog, Warning, TEXT("KinectDevice is not running."));
		return false;
	}

	if (Thread)
	{
		Thread->EnsureCompletion();
		Thread = nullptr;
	}

	if (BodyTracker)
	{
		BodyTracker.shutdown();
		BodyTracker.destroy();
		BodyTracker = nullptr;
	}

	if (NativeDevice)
	{
		NativeDevice.stop_cameras();
		NativeDevice.close();
		NativeDevice = nullptr;
		UE_LOG(AzureKinectDeviceLog, Verbose, TEXT("KinectDevice Camera is Stopped and Closed."));
	}

	bOpen = false;
	return true;
}

int32 UAzureKinectDevice::GetNumConnectedDevices()
{
	return k4a_device_get_installed_count();
}

int32 UAzureKinectDevice::GetNumTrackedSkeletons() const
{
	if (!bOpen)
	{
		return 0;
	}	
	if (!bSkeletonTracking)
	{
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("GetNumTrackedBodies: Skeleton Tracking is disabled!"));
		return 0;
	}

	FScopeLock Lock(Thread->GetCriticalSection());
	return NumTrackedSkeletons;
}

FAzureKinectSkeleton UAzureKinectDevice::GetSkeleton(int32 Index) const
{
	if (bOpen)
	{
		if (!bSkeletonTracking)
		{
			UE_LOG(AzureKinectDeviceLog, Error, TEXT("GetSkeleton: Skeleton Tracking is disabled!"));
			return FAzureKinectSkeleton();
		}

		FScopeLock Lock(Thread->GetCriticalSection());
		if (Skeletons.IsValidIndex(Index))
		{
			return Skeletons[Index];
		}
		else
		{
			UE_LOG(AzureKinectDeviceLog, Error, TEXT("GetSkeleton: Index is out of range!"));
			return FAzureKinectSkeleton();
		}
	}
	else
	{
		return FAzureKinectSkeleton();
	}
	
}

const TArray<FAzureKinectSkeleton>& UAzureKinectDevice::GetSkeletons() const {
	if (bOpen)
	{
		FScopeLock Lock(Thread->GetCriticalSection());
		return Skeletons;
	}
	else
	{
		return Skeletons;
	}
}

void UAzureKinectDevice::UpdateAsync()
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

	if (ColorMode != EKinectColorResolution::RESOLUTION_OFF && ColorTexture)
	{
		CaptureColorImage();
	}

	if (DepthMode != EKinectDepthMode::OFF && DepthTexture)
	{
		CaptureDepthImage();
	}
	
	if (DepthMode != EKinectDepthMode::OFF && InflaredTexture)
	{
		CaptureInflaredImage();
	}

	if (bSkeletonTracking && BodyTracker)
	{
		UpdateSkeletons();
	}

	Capture.reset();

}

void UAzureKinectDevice::CaptureColorImage()
{
	int32 Width = 0, Height = 0;
	uint8* SourceBuffer;

	if (RemapMode == EKinectRemap::COLOR_TO_DEPTH)
	{
		k4a::image DepthCapture = Capture.get_depth_image();
		k4a::image ColorCapture = Capture.get_color_image();

		if (!DepthCapture.is_valid() || !ColorCapture.is_valid()) return;

		Width = DepthCapture.get_width_pixels();
		Height = DepthCapture.get_height_pixels();

		if (Width == 0 || Height == 0) return;

		//
		if (!RemapImage || !RemapImage.is_valid())
		{
			RemapImage = k4a::image::create(K4A_IMAGE_FORMAT_COLOR_BGRA32, Width, Height, Width * static_cast<int>(sizeof(uint8) * 4));
		}

		try
		{
			KinectTransformation.color_image_to_depth_camera(DepthCapture, ColorCapture, &RemapImage);
		}
		catch (const k4a::error& Err)
		{
			FString Msg(ANSI_TO_TCHAR(Err.what()));
			UE_LOG(AzureKinectDeviceLog, Error, TEXT("Cant't transform Color to Depth: %s"), *Msg);
			return;
		}

		SourceBuffer = RemapImage.get_buffer();

		DepthCapture.reset();
		ColorCapture.reset();
	}
	else
	{
		k4a::image ColorCapture = Capture.get_color_image();

		if (!ColorCapture.is_valid()) return;

		Width = ColorCapture.get_width_pixels();
		Height = ColorCapture.get_height_pixels();
		if (Width == 0 || Height == 0) return;
		
		SourceBuffer = ColorCapture.get_buffer();

		ColorCapture.reset();
	}

	if (ColorTexture->GetSurfaceWidth() != Width || ColorTexture->GetSurfaceHeight() != Height)
	{
		ColorTexture->InitCustomFormat(Width, Height, EPixelFormat::PF_B8G8R8A8, false);
		ColorTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		ColorTexture->UpdateResource();
	}	
	else
	{

		FTextureResource* TextureResource = ColorTexture->Resource;
		auto Region = FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);

		ENQUEUE_RENDER_COMMAND(UpdateTextureData)(
			[TextureResource, Region, SourceBuffer](FRHICommandListImmediate& RHICmdList) {
				FTexture2DRHIRef Texture2D = TextureResource->TextureRHI ? TextureResource->TextureRHI->GetTexture2D() : nullptr;
				if (!Texture2D)
				{
					return;
				}
				RHIUpdateTexture2D(Texture2D, 0, Region, 4 * Region.Width, SourceBuffer);
			});
	}

}

void UAzureKinectDevice::CaptureDepthImage()
{
	int32 Width = 0, Height = 0;
	uint8* SourceBuffer;
	if (RemapMode == EKinectRemap::DEPTH_TO_COLOR)
	{
		k4a::image DepthCapture = Capture.get_depth_image();
		k4a::image ColorCapture = Capture.get_color_image();

		if (!DepthCapture.is_valid() || !ColorCapture.is_valid()) return;

		Width = ColorCapture.get_width_pixels();
		Height = ColorCapture.get_height_pixels();
		
		if (Width == 0 || Height == 0) return;

		//
		if (!RemapImage || !RemapImage.is_valid())
		{
			RemapImage = k4a::image::create(K4A_IMAGE_FORMAT_DEPTH16, Width, Height, Width * static_cast<int>(sizeof(uint16)));
		}

		try
		{
			KinectTransformation.depth_image_to_color_camera(DepthCapture, &RemapImage);
		}
		catch (const k4a::error& Err)
		{
			FString Msg(ANSI_TO_TCHAR(Err.what()));
			UE_LOG(AzureKinectDeviceLog, Error, TEXT("Cant't transform Depth to Color: %s"), *Msg);
			return;
		}

		SourceBuffer = RemapImage.get_buffer();

		DepthCapture.reset();
		ColorCapture.reset();
	}
	else
	{
		k4a::image DepthCapture = Capture.get_depth_image();
		if (!DepthCapture.is_valid()) return;

		Width = DepthCapture.get_width_pixels();
		Height = DepthCapture.get_height_pixels();

		if (Width == 0 || Height == 0) return;

		SourceBuffer = DepthCapture.get_buffer();

		DepthCapture.reset();
	}

	if (DepthTexture->GetSurfaceWidth() != Width || DepthTexture->GetSurfaceHeight() != Height)
	{
		DepthTexture->InitCustomFormat(Width, Height, EPixelFormat::PF_R8G8B8A8, true);
		DepthTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		DepthTexture->UpdateResource();
	}
	else
	{
		
		TArray<uint8> SrcData;
		SrcData.Reset(Width * Height * 4);
		for (int hi = 0; hi < Height; hi++)
		{
			for (int wi = 0; wi < Width; wi++)
			{
				int index = hi * Width + wi;
				uint16 R = SourceBuffer[index * 2];
				uint16 G = SourceBuffer[index * 2 + 1];
				
				uint16 Sample = G << 8 | R;

				SrcData.Push(SourceBuffer[index * 2]);
				SrcData.Push(SourceBuffer[index * 2 + 1]);
				SrcData.Push(Sample > 0 ? 0x00 : 0xFF);
				SrcData.Push(0xFF);
				
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

}

void UAzureKinectDevice::CaptureInflaredImage()
{
	const k4a::image& InflaredCapture = Capture.get_ir_image();
	if (!InflaredCapture.is_valid()) return;

	int32 Width = InflaredCapture.get_width_pixels(), Height = InflaredCapture.get_height_pixels();
	if (Width == 0 || Height == 0) return;

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
					SrcData.Push(0x00);
					SrcData.Push(0xff);
				}
				else
				{
					SrcData.Push(0x00);
					SrcData.Push(0x00);
					SrcData.Push(0xff);
					SrcData.Push(0xff);
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

				RHIUpdateTexture2D(Texture2D, 0, Region, 4 * Region.Width, SrcData.GetData());
			});
	}
	
}

void UAzureKinectDevice::CaptureBodyIndexImage(const k4abt::frame& BodyFrame)
{
	k4a::image BodyIndexMap = BodyFrame.get_body_index_map();

	int32 Width = BodyIndexMap.get_width_pixels(), Height = BodyIndexMap.get_height_pixels();
	if (Width == 0 || Height == 0) return;

	if (BodyIndexTexture->GetSurfaceWidth() != Width || BodyIndexTexture->GetSurfaceHeight() != Height)
	{
		BodyIndexTexture->InitCustomFormat(Width, Height, EPixelFormat::PF_R8G8B8A8, true);
		BodyIndexTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		BodyIndexTexture->UpdateResource();
	}
	else
	{
		uint8* S = BodyIndexMap.get_buffer();
		TArray<uint8> SrcData;
		SrcData.Reset(Width * Height * 4);
		for (int i = 0; i < Width * Height; i++)
		{
			SrcData.Push(S[i]);
			SrcData.Push(S[i]);
			SrcData.Push(S[i]);
			SrcData.Push(0xff);
		}

		FTextureResource* TextureResource = BodyIndexTexture->Resource;
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

}

void UAzureKinectDevice::UpdateSkeletons()
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

	if (BodyIndexTexture)
	{
		CaptureBodyIndexImage(BodyFrame);
	}

	{
		FScopeLock Lock(Thread->GetCriticalSection());

		NumTrackedSkeletons = BodyFrame.get_num_bodies();
		Skeletons.Reset(NumTrackedSkeletons);

		for (int32 i = 0; i < NumTrackedSkeletons; i++)
		{
			k4abt_body_t Body;
			FAzureKinectSkeleton Skeleton;

			BodyFrame.get_body_skeleton(i, Body.skeleton);
			Skeleton.ID = BodyFrame.get_body_id(i);

			Skeleton.Joints.Reset(K4ABT_JOINT_COUNT);

			for (int32 j = 0; j < K4ABT_JOINT_COUNT; j++)
			{
				Skeleton.Joints.Push(JointToTransform(Body.skeleton.joints[j], j));
			}

			Skeletons.Push(Skeleton);
		}
	}
	
		
	BodyFrame.reset();
	
}

FTransform UAzureKinectDevice::JointToTransform(const k4abt_joint_t& Joint, int32 Index)
{

	// This transform algorithm is introdeced from 
	// https://github.com/secretlocation/azure-kinect-unreal/
	// Still there is room to refactor...

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
	FVector Position(Joint.position.xyz.z, Joint.position.xyz.x, - Joint.position.xyz.y);
	Position *= 0.1f;

	/**
	 * Convert the Orientation from Kinect co-ordinate system to Unreal co-ordinate system.
	 * We negate the x, y components of the JointQuaternion since we are converting from
	 * Kinect's Right Hand orientation to Unreal's Left Hand orientation.
	 */
	FQuat Quat(
		-Joint.orientation.wxyz.x,
		-Joint.orientation.wxyz.y,
		Joint.orientation.wxyz.z,
		Joint.orientation.wxyz.w
	);
	
	return FTransform(Quat, Position);
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
	FrameTime = std::chrono::milliseconds(FMath::CeilToInt(FrameTimeInMilli));
}
