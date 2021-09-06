// Fill out your copyright notice in the Description page of Project Settings.
#include "AzureKinectDevice.h"
#include "Runtime/RHI/Public/RHI.h"
#include "GenericPlatform/GenericPlatformMath.h"

DEFINE_LOG_CATEGORY(AzureKinectDeviceLog);

UAzureKinectDevice::UAzureKinectDevice() :
	NativeDevice(nullptr),
	Thread(nullptr),
	DeviceIndex(-1),
	bOpened(false)
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
			catch (const k4a::error& e)
			{
				FString ErrStr(e.what());
				UE_LOG(AzureKinectDeviceLog, Error, TEXT("k4a::error: %s"), *ErrStr);
			}
		}
	}
}

void UAzureKinectDevice::StartDevice()
{
	if (bOpened)
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

	}
	catch (const k4a::error& Err)
	{
		if (NativeDevice)
		{
			NativeDevice.close();
		}

		UE_LOG(AzureKinectDeviceLog, Error, TEXT("k4a::error: %s"), Err.what());
		return;
	}
	
	Thread = new FAzureKinectDeviceThread(this);

	bOpened = true;
}

void UAzureKinectDevice::StopDevice()
{

	if (!bOpened)
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

	bOpened = false;

}

int32 UAzureKinectDevice::GetNumConnectedDevices()
{
	return k4a_device_get_installed_count();
}

void UAzureKinectDevice::Update()
{
	try
	{
		if (!NativeDevice.get_capture(&Capture, FrameTime))
		{
			UE_LOG(AzureKinectDeviceLog, Verbose, TEXT("Timed out waiting for capture."));
		}
	}
	catch (const k4a::error& Err)
	{
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("%s"), Err.what());
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
	
	if (false)
	{
		CaptureInflaredImage();
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
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("k4a::error: %s"), Err.what());
		return;
	}

	if (DepthTexture->GetSurfaceWidth() != Width || DepthTexture->GetSurfaceHeight() != Height)
	{
		DepthTexture->InitCustomFormat(Width, Height, EPixelFormat::PF_R8G8B8A8, true);
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
		InflaredTexture->InitCustomFormat(Width, Height, EPixelFormat::PF_R8G8, true);
		DepthTexture->UpdateResource();
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
