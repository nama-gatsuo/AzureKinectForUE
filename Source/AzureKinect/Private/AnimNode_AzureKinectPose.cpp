// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNode_AzureKinectPose.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"
#include "k4abttypes.h"

DEFINE_LOG_CATEGORY(AzureKinectAnimNodeLog);

FAnimNode_AzureKinectPose::FAnimNode_AzureKinectPose()
{
	BonesToModify.Reserve(K4ABT_JOINT_COUNT);
	for (int i = 0; i < K4ABT_JOINT_COUNT; i++)
	{
		BonesToModify.Add(static_cast<EKinectBodyJoint>(i), FBoneReference());
	}
}

void FAnimNode_AzureKinectPose::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread);
	
	GetEvaluateGraphExposedInputs().Execute(Context);

	USkeletalMeshComponent* SkelMesh = Context.AnimInstanceProxy->GetSkelMeshComponent();
	
	BoneTransforms.Reset(K4ABT_JOINT_COUNT);

	for (int i = 0; i < Skeleton.Joints.Num(); i++)
	{
		EKinectBodyJoint JointIndex = static_cast<EKinectBodyJoint>(i);
		if (BonesToModify.Contains(JointIndex))
		{
			int32 BoneIndex = SkelMesh->GetBoneIndex(BonesToModify[JointIndex].BoneName);
			if (BoneIndex != INDEX_NONE)
			{
				FCompactPoseBoneIndex CompactBoneIndex(BoneIndex);
				BoneTransforms.Emplace(CompactBoneIndex, Skeleton.Joints[i]);
			}
			
		}
	}
	
}

void FAnimNode_AzureKinectPose::EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(EvaluateComponentSpace_AnyThread)

	Output.ResetToRefPose();

	for (const FBoneTransform& BoneTransform : BoneTransforms)
	{	
		FTransform Transform = Output.Pose.GetComponentSpaceTransform(BoneTransform.BoneIndex);
		Transform.SetRotation(BoneTransform.Transform.Rotator().Quaternion());
		Output.Pose.SetComponentSpaceTransform(BoneTransform.BoneIndex, Transform);
	}
	
	

}
