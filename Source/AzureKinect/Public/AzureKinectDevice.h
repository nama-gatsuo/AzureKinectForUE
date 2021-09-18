#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Animation/SkeletalMeshActor.h"

#include "k4a/k4a.hpp"
#include "k4abt.hpp"
#include "AzureKinectEnum.h"
#include "AzureKinectDeviceThread.h"

#include "AzureKinectDevice.generated.h"

USTRUCT(BlueprintType)
struct FAzureKinectSkeleton
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 ID;

	UPROPERTY(BlueprintReadWrite)
	TArray<FTransform> Joints;
};

DECLARE_LOG_CATEGORY_EXTERN(AzureKinectDeviceLog, Log, All);

UCLASS(BlueprintType, hidecategories=(Object))
class AZUREKINECT_API UAzureKinectDevice : public UObject
{
	GENERATED_BODY()
public:
	UAzureKinectDevice();
	UAzureKinectDevice(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IO")
	UTextureRenderTarget2D* DepthTexture;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IO")
	UTextureRenderTarget2D* ColorTexture;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IO")
	UTextureRenderTarget2D* InflaredTexture;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IO")
	UTextureRenderTarget2D* BodyIndexTexture;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	EKinectDepthMode DepthMode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	EKinectColorResolution ColorMode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	EKinectFps Fps;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	EKinectSensorOrientation SensorOrientation = EKinectSensorOrientation::DEFAULT;

	UPROPERTY(BlueprintReadWrite, Category = "Config")
	int32 DeviceIndex;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	bool bSkeletonTracking;

	UFUNCTION(BlueprintCallable, Category = "IO")
	static int32 GetNumConnectedDevices();

	UFUNCTION(BlueprintCallable, Category = "IO")
	void LoadDevices();

	/**
	 * Call "open" and "start_camara" to Native Kinect Device
	 * and return result. Then start a thread for Kinect's data feed.
	 * Device Index should be specified in advance;
	 */
	UFUNCTION(BlueprintCallable, Category = "IO")
	bool StartDevice();
	
	/**
	 * Call "stop_camara" and "close" to Native Kinect Device,
	 * and release all instaces about Native Kinect.
	 */
	UFUNCTION(BlueprintCallable, Category = "IO")
	bool StopDevice();
	
	/**
	 * Check if Kinect Device is open.
	 */
	UFUNCTION(BlueprintCallable, Category = "IO")
	bool IsOpen() const { return bOpen; }

	/**
	 * Return a number of Skeletons currently aquired and stored.
	 */
	UFUNCTION(BlueprintCallable, Category = "Skeletons")
	int32 GetNumTrackedSkeletons() const;

	/**
	 * Return an array of Skeletons currently aquired and stored.
	 */
	UFUNCTION(BlueprintCallable, Category = "Skeletons")
	const TArray<FAzureKinectSkeleton>& GetSkeletons() const { return Skeletons; }

	/**
	 * Return a Skeleton struct by Index (not Skeleton ID).
	 * If given Index is out of range, return a null struct.
	 */
	UFUNCTION(BlueprintCallable, Category = "Skeletons")
	FAzureKinectSkeleton GetSkeleton(int32 Index) const;
	
	/** 
	 * Update and process raw feed from Kinect Device asynchronously. 
	 * Should be called out of main thread.
	 */
	void UpdateAsync();

	TArray<TSharedPtr<FString>> DeviceList;

private:
	bool bOpen;

	void CaptureColorImage();
	void CaptureDepthImage();
	void CaptureInflaredImage();
	void CaptureBodyIndexImage(const k4abt::frame& BodyFrame);

	static FTransform JointToTransform(const k4abt_joint_t& Joint, int32 Index);
	static FQuat AlignJointOrientation(const FQuat& Quat, int32 Index);
	void UpdateSkeletons();
	
	void CalcFrameCount();

	k4a::device NativeDevice;
	k4a::capture Capture;
	std::chrono::milliseconds FrameTime;
	k4a::image DepthRemapped;
	k4a::calibration KinectCalibration;
	k4a::transformation KinectTransformation;
	k4abt::tracker BodyTracker;

	FAzureKinectDeviceThread* Thread;
	
	int32 NumTrackedSkeletons;
	TArray<FAzureKinectSkeleton> Skeletons;
};
