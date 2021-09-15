#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Animation/SkeletalMeshActor.h"

#include "k4a/k4a.hpp"
#include "k4abt.hpp"
#include "AzureKinectEnum.h"
#include "AzureKinectDeviceThread.h"

#include "AzureKinectDevice.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AzureKinectDeviceLog, Log, All);

UCLASS(BlueprintType, hidecategories=(Object))
class AZUREKINECT_API UAzureKinectDevice : public UObject
{

	GENERATED_BODY()
public:
	UAzureKinectDevice();
	UAzureKinectDevice(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "Azure Kinect")
	UTextureRenderTarget2D* DepthTexture;

	UPROPERTY(EditAnywhere, Category = "Azure Kinect")
	UTextureRenderTarget2D* ColorTexture;

	UPROPERTY(EditAnywhere, Category = "Azure Kinect")
	UTextureRenderTarget2D* InflaredTexture;

	UPROPERTY(EditAnywhere, Category = "Azure Kinect")
	EKinectDepthMode DepthMode;

	UPROPERTY(EditAnywhere, Category = "Azure Kinect")
	EKinectColorResolution ColorMode;

	UPROPERTY(EditAnywhere, Category = "Azure Kinect")
	EKinectFps Fps = EKinectFps::PER_SECOND_30;

	UPROPERTY(EditAnywhere, Category = "Azure Kinect")
	EKinectSensorOrientation SensorOrientation = EKinectSensorOrientation::DEFAULT;

	UPROPERTY(EditAnywhere, Category = "Azure Kinect")
	TArray<TSoftObjectPtr<ASkeletalMeshActor>> SkeltalMeshes;

	UPROPERTY(BlueprintReadWrite, Category = "Azure Kinect")
	int32 DeviceIndex = -1;
	
	TArray<TSharedPtr<FString>> DeviceList;

	UFUNCTION(BlueprintCallable, Category = "Azure Kinect")
	static int32 GetNumConnectedDevices();

	UFUNCTION(BlueprintCallable, Category = "Azure Kinect")
	void LoadDevice();

	UFUNCTION(BlueprintCallable, Category = "Azure Kinect")
	void StartDevice();
	
	UFUNCTION(BlueprintCallable, Category = "Azure Kinect")
	void StopDevice();
	
	UFUNCTION(BlueprintCallable, Category = "Azure Kinect")
	bool IsOpen() const { return bOpen; }

	UFUNCTION(BlueprintCallable, Category = "Azure Kinect")
	int32 GetNumTrackedBodies() const;

	UFUNCTION(BlueprintCallable, Category = "Azure Kinect")
	TArray<FTransform> GetSkeltonJoints(int32 BodyIndex) const;

	void Update();

private:
	bool bOpen;

	void CaptureColorImage();
	void CaptureDepthImage();
	void CaptureInflaredImage();

	static bool ValidateSkelton(TSoftObjectPtr<ASkeletalMeshActor> Skelton);
	static FTransform JointToTransform(const k4abt_joint_t& Joint, int32 Index);
	static FQuat AlignJointOrientation(const FQuat& Quat, int32 Index);
	void UpdateSkeltons();
	
	void CalcFrameCount();

	k4a::device NativeDevice;
	k4a::capture Capture;
	std::chrono::milliseconds FrameTime;
	k4a::image DepthRemapped;
	k4a::calibration KinectCalibration;
	k4a::transformation KinectTransformation;
	k4abt::tracker BodyTracker;

	FAzureKinectDeviceThread* Thread;
	
	int32 NumTrackedBodies;
	TArray<TArray<FTransform>> TrackedJoints;
};
