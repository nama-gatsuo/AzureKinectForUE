// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BonePose.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AzureKinectDevice.h"

#include "AnimNode_AzureKinectPose.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AzureKinectAnimNodeLog, Log, All);

/**
 * 
 */
USTRUCT(BlueprintInternalUseOnly)
struct AZUREKINECT_API FAnimNode_AzureKinectPose : public FAnimNode_Base
{
	GENERATED_BODY()

public:
	FAnimNode_AzureKinectPose();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transform", meta = (PinShownByDefault))
	FAzureKinectSkeleton Skeleton;

	UPROPERTY(EditAnywhere, Category="Bone Mapping")
	TMap<EKinectBodyJoint, FBoneReference> BonesToModify;

	// FAnimNode_Base interface
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext& Output) override;

private:
	TArray<FBoneTransform> BoneTransforms;
};
