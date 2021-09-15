#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

DECLARE_LOG_CATEGORY_EXTERN(AzureKinectThreadLog, Log, All);

class UAzureKinectDevice;

class FAzureKinectDeviceThread : public FRunnable
{
public:
	
	FAzureKinectDeviceThread(UAzureKinectDevice* Device);

	virtual ~FAzureKinectDeviceThread();

	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();

	/** Stops the threadand waits for its completion. */
	void EnsureCompletion();

	FCriticalSection* GetCriticalSection();

private:
	/** Thread handle.Control the thread using this, with operators like Killand Suspend */
	FRunnableThread* Thread;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;

	/** The device that starts this thread. */
	UAzureKinectDevice* KinectDevice;

	/** To be used for UScopeLock */
	FCriticalSection CriticalSection;

};