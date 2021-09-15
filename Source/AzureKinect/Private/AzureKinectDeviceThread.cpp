#include "AzureKinectDeviceThread.h"
#include "HAL/PlatformProcess.h"
#include "AzureKinectDevice.h"

DEFINE_LOG_CATEGORY(AzureKinectThreadLog);

FAzureKinectDeviceThread::FAzureKinectDeviceThread(UAzureKinectDevice* Device) :
	KinectDevice(Device),
	Thread(nullptr),
	StopTaskCounter(0)
{
	Thread = FRunnableThread::Create(this, TEXT("FAzureKinectDeviceThread"), 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
	if (!Thread)
	{
		UE_LOG(AzureKinectThreadLog, Error, TEXT("Failed to create Azure Kinect thread."));
	}

}

FAzureKinectDeviceThread::~FAzureKinectDeviceThread()
{
	if (Thread)
	{
		delete Thread;
		Thread = nullptr;
	}
}

bool FAzureKinectDeviceThread::Init()
{
	UE_LOG(AzureKinectThreadLog, Verbose, TEXT("Azure Kinect thread started."));
	return true;
}

uint32 FAzureKinectDeviceThread::Run()
{
	if (!KinectDevice)
	{
		UE_LOG(AzureKinectThreadLog, Error, TEXT("KinectDevice is null, could not run the thread"));
		return 1;
	}

	while (StopTaskCounter.GetValue() == 0)
	{
		// Do the Kinect capture, enqueue, pop body frame stuff
		KinectDevice->Update();
	}

	return 0;
}

void FAzureKinectDeviceThread::Stop()
{
	StopTaskCounter.Increment();
}

void FAzureKinectDeviceThread::EnsureCompletion()
{
	Stop();
	if (Thread)
	{
		Thread->WaitForCompletion();
	}

}

FCriticalSection* FAzureKinectDeviceThread::GetCriticalSection()
{
	return &CriticalSection;
}
