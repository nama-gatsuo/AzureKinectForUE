// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AzureKinectEnum.h"

#include "AzureKinectSkeltonAnim.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Category = "Azure Kinect")
class AZUREKINECT_API UAzureKinectSkeltonAnim : public UAnimInstance
{
	GENERATED_BODY()
public:
	void SetJoints(const TArray<FTransform>& Transforms);
	void SetBodyID(int32 ID) { BodyID = ID; }

protected:
	UFUNCTION(BlueprintPure, Category = "Azure Kinect", meta = (BlueprintThreadSafe))
	FTransform GetJointTransfom(EKinectBodyJoint JoinId);
	
	UFUNCTION(BlueprintPure, Category = "Azure Kinect", meta = (BlueprintThreadSafe))
	bool HasJoints();

	UFUNCTION(BlueprintPure, Category = "Azure Kinect", meta = (BlueprintThreadSafe))
	int32 GetBodyID() { return BodyID; }

private:
	TArray<FTransform> Joints;
	int32 BodyID = -1;
};
