#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "k4a/k4a.hpp"

#include "AzureKinectDevice.generated.h"

UCLASS(BlueprintType, hidecategories=(Object))
class AZUREKINECT_API UAzureKinectDevice : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* DepthTexture;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* ColorTexture;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* IrTexture;
};
