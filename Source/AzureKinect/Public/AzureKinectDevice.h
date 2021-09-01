#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "k4a/k4a.hpp"

#include "AzureKinectDevice.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AzureKinectDeviceLog, Log, All);

UCLASS(BlueprintType, hidecategories=(Object))
class AZUREKINECT_API UAzureKinectDevice : public UObject
{
	GENERATED_BODY()
public:
	UAzureKinectDevice();
	UAzureKinectDevice(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "AzureKinect")
	UTextureRenderTarget2D* DepthTexture;

	UPROPERTY(EditAnywhere, Category = "AzureKinect")
	UTextureRenderTarget2D* ColorTexture;

	UPROPERTY(EditAnywhere, Category = "AzureKinect")
	UTextureRenderTarget2D* IrTexture;

	UFUNCTION(CallInEditor, Category = "AzureKinect")
	void LoadDevice();
	
	int32 DeviceIndex = -1;
	TArray<TSharedPtr<FString>> DeviceList;

private:
	

};
