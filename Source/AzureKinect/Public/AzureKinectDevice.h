#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "k4a/k4a.hpp"
#include "AzureKinectEnum.h"

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
	UTextureRenderTarget2D* IrTexture;

	UFUNCTION(CallInEditor, Category = "Azure Kinect")
	void LoadDevice();
	
	UFUNCTION(Category = "Azure Kinect")
	static int32 GetNumConnectedDevices();

	UPROPERTY(EditAnywhere, Category = "Azure Kinect")
	EKinectDepthMode DepthMode;

	UPROPERTY(EditAnywhere, Category = "Azure Kinect")
	EKinectColorResolution ColorMode;

	UPROPERTY(EditAnywhere, Category = "Azure Kinect")
	EKinectFps Fps = EKinectFps::PER_SECOND_30;

	int32 DeviceIndex = -1;
	
	TArray<TSharedPtr<FString>> DeviceList;

	void StartDevice();
	void StopDevice();
	void Update();
	void CaptureColorImage();
	void CaptureDepthImage();

private:

	void CalcFrameCount();

	k4a::device NativeDevice;
	k4a::capture Capture;
	std::chrono::milliseconds FrameTime;

};
