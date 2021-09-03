// Fill out your copyright notice in the Description page of Project Settings.


#include "AzureKinectDevice.h"

DEFINE_LOG_CATEGORY(AzureKinectDeviceLog);

UAzureKinectDevice::UAzureKinectDevice()
{
	UE_LOG(AzureKinectDeviceLog, Verbose, TEXT("Constructed."));
	LoadDevice();
}

UAzureKinectDevice::UAzureKinectDevice(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	UE_LOG(AzureKinectDeviceLog, Verbose, TEXT("Constructed. (OI)"));
	LoadDevice();
}

void UAzureKinectDevice::LoadDevice()
{

	// https://docs.microsoft.com/en-us/azure/kinect-dk/find-then-open-device
	int32 NumKinect = k4a_device_get_installed_count();
	
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
				UE_LOG(AzureKinectDeviceLog, Error, TEXT("%s"), *ErrStr);
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
		UE_LOG(AzureKinectDeviceLog, Error, TEXT("%s"), *ErrStr);
		return;
	}

}

void UAzureKinectDevice::StopDevice()
{
	if (NativeDevice)
	{
		NativeDevice.stop_cameras();
		NativeDevice.close();
		NativeDevice = nullptr;
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

	CaptureColorImage();
	CaptureDepthImage();
}

void UAzureKinectDevice::CaptureColorImage()
{
	const k4a::image& ColorCapture = Capture.get_color_image();
	// reinterpret_cast<uint8_t*>(ColorCapture.get_buffer());
}

void UAzureKinectDevice::CaptureDepthImage()
{
	const k4a::image& DepthCapture = Capture.get_depth_image();
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



