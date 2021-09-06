#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"

#include "k4a/k4a.hpp"
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

	UPROPERTY(EditAnywhere, Category = "Azure Kinect", meta = (EditCondition = "!bOpened"))
	EKinectDepthMode DepthMode;

	UPROPERTY(EditAnywhere, Category = "Azure Kinect", meta = (EditCondition = "!bOpened"))
	EKinectColorResolution ColorMode;

	UPROPERTY(EditAnywhere, Category = "Azure Kinect", meta = (EditCondition = "!bOpened"))
	EKinectFps Fps = EKinectFps::PER_SECOND_30;

	UPROPERTY(BlueprintReadWrite, Category = "Azure Kinect")
	int32 DeviceIndex = -1;
	
	TArray<TSharedPtr<FString>> DeviceList;
	
	UPROPERTY(BlueprintReadOnly, Category = "Azure Kinect")
	bool bOpened;

	UFUNCTION(BlueprintCallable, Category = "Azure Kinect")
	static int32 GetNumConnectedDevices();

	UFUNCTION(BlueprintCallable, Category = "Azure Kinect")
	void LoadDevice();

	UFUNCTION(BlueprintCallable, Category = "Azure Kinect")
	void StartDevice();
	
	UFUNCTION(BlueprintCallable, Category = "Azure Kinect")
	void StopDevice();
	
	void Update();

private:

	void CaptureColorImage();
	void CaptureDepthImage();
	void CaptureInflaredImage();

	void CalcFrameCount();

	k4a::device NativeDevice;
	k4a::capture Capture;
	std::chrono::milliseconds FrameTime;
	k4a::image DepthRemapped;
	k4a::calibration KinectCalibration;
	k4a::transformation KinectTransformation;

	FAzureKinectDeviceThread* Thread;

};
