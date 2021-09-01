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

	if (NumKinect > 0)
	{
		for (int32 i = 0; i < NumKinect; i++)
		{
			try
			{
				// Open connection to the device.
				k4a::device Device = k4a::device::open(0);
				// Get the device serial number.
				FString SerialNumber(Device.get_serialnum().c_str());
				
				DeviceList.Add(SerialNumber);

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

