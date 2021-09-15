// Fill out your copyright notice in the Description page of Project Settings.


#include "AzureKinectSkeltonAnim.h"

void UAzureKinectSkeltonAnim::SetJoints(const TArray<FTransform>& Transforms)
{
	Joints = Transforms;
}

FTransform UAzureKinectSkeltonAnim::GetJointTransfom(EKinectBodyJoint JointId)
{
	if (HasJoints())
	{
		return Joints[uint8(JointId)];
	}
	else
	{
		return FTransform();
	}

}

bool UAzureKinectSkeltonAnim::HasJoints()
{
	return Joints.Num() != 0;
}
