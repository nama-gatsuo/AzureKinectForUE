// Fill out your copyright notice in the Description page of Project Settings.
#include "AzureKinectDevice.h"
#include "Runtime/RHI/Public/RHI.h"

DEFINE_LOG_CATEGORY(AzureKinectDeviceLog);

UAzureKinectDevice::UAzureKinectDevice():
	NativeDevice(nullptr),
	Thread(nullptr),
	DeviceIndex(-1)
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
	CalcFrameCount();

	if (DeviceIndex == -1)
	{
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("No Device is selected."));
		return;
	}

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
		
		NativeDevice.start_cameras(&DeviceConfig);

	}
	catch (const k4a::error& e)
	{
		FString ErrStr(e.what());
		if (NativeDevice)
		{
			NativeDevice.close();
		}

		UE_LOG(AzureKinectDeviceLog, Error, TEXT("k4a::error: %s"), *ErrStr);
		return;
	}
	
	Thread = new FAzureKinectDeviceThread(this);

}

void UAzureKinectDevice::StopDevice()
{

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
		UE_LOG(AzureKinectDeviceLog, Warning, TEXT("KinectDevice Camera is Stopped and Closed"));
	}

	if (ColorTexture)
	{
		ColorTexture->ReleaseResource();
		ColorTexture->ConditionalBeginDestroy();
		ColorTexture = nullptr;
	}

	if (DepthTexture)
	{
		DepthTexture->ReleaseResource();
		DepthTexture->ConditionalBeginDestroy();
		DepthTexture = nullptr;
	}
	
	if (InflaredTexture)
	{
		InflaredTexture->ReleaseResource();
		InflaredTexture->ConditionalBeginDestroy();
		InflaredTexture = nullptr;
	}

}

int32 UAzureKinectDevice::GetNumConnectedDevices()
{
	return k4a_device_get_installed_count();
}

void UAzureKinectDevice::Update()
{
	// Should be in threaded function

	try
	{
		if (!NativeDevice.get_capture(&Capture, FrameTime))
		{
			UE_LOG(AzureKinectDeviceLog, Error, TEXT("Timed out waiting for capture."));
		}
	}
	catch (const k4a::error& e)
	{
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("%s"), e.what());
		return;
	}

	if (true)
	{
		CaptureColorImage();
	}

	if (true)
	{
		CaptureDepthImage();
	}
	
	if (false)
	{
		CaptureInflaredImage();
	}

}

void UAzureKinectDevice::CaptureColorImage()
{
	const k4a::image& ColorCapture = Capture.get_color_image();
	if (!ColorCapture.is_valid()) return;
	
	int32 Width = ColorCapture.get_width_pixels(), Height = ColorCapture.get_height_pixels();
	
	if (!ColorTexture)
	{
		ColorTexture = NewObject<UTexture2D>();
		ColorTexture = UTexture2D::CreateTransient(Width, Height, EPixelFormat::PF_B8G8R8A8, TEXT("AzureKinectColor"));
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
	
	

}

void UAzureKinectDevice::CaptureDepthImage()
{
	const k4a::image& DepthCapture = Capture.get_depth_image();
	if (!DepthCapture.is_valid()) return;

	int32 Width = DepthCapture.get_width_pixels(), Height = DepthCapture.get_height_pixels();
	

	if (!DepthTexture)
	{
		DepthTexture = NewObject<UTexture2D>();
		DepthTexture = UTexture2D::CreateTransient(Width, Height, EPixelFormat::PF_R8G8B8A8, TEXT("AzureKinectDepth"));
		DepthTexture->UpdateResource();
	}
	else
	{
		const uint8* SrcData = DepthCapture.get_buffer();
		FTextureResource* TextureResource = DepthTexture->Resource;
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

}

void UAzureKinectDevice::CaptureInflaredImage()
{
	const k4a::image& InflaredCapture = Capture.get_ir_image();
	if (!InflaredCapture.is_valid()) return;

	int32 Width = InflaredCapture.get_width_pixels(), Height = InflaredCapture.get_height_pixels();

	if (!InflaredTexture)
	{
		InflaredTexture = NewObject<UTexture2D>();
		InflaredTexture = UTexture2D::CreateTransient(Width, Height, EPixelFormat::PF_R8G8, TEXT("AzureKinectInflared"));
		InflaredTexture->UpdateResource();
	}
	else
	{
		const uint8* SrcData = InflaredCapture.get_buffer();
		FTextureResource* TextureResource = InflaredTexture->Resource;
		auto Region = FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);

		ENQUEUE_RENDER_COMMAND(UpdateTextureData)(
			[TextureResource, Region, SrcData](FRHICommandListImmediate& RHICmdList) {
				FTexture2DRHIRef Texture2D = TextureResource->TextureRHI ? TextureResource->TextureRHI->GetTexture2D() : nullptr;
				if (!Texture2D)
				{
					return;
				}

				RHIUpdateTexture2D(Texture2D, 0, Region, 2 * Region.Width, SrcData);
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
	FrameTime = std::chrono::milliseconds((int)FrameTimeInMilli);
}
