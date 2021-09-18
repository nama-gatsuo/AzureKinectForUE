// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNodes/AnimGraphNode_AzureKinectPose.h"
#include "AnimationGraphSchema.h"

#define LOCTEXT_NAMESPACE "AzureKinectPose"

FText UAnimGraphNode_AzureKinectPose::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return LOCTEXT("AzureKinectPose", "Azure Kinect Pose");
}

FString UAnimGraphNode_AzureKinectPose::GetNodeCategory() const
{

    return FString("Azure Kinect");
}

FText UAnimGraphNode_AzureKinectPose::GetTooltipText() const
{
    return LOCTEXT(
        "AnimGraphNode_AzureKinectPose_Tooltip",
        "Process AzureKinect skeleton input into pose"
    );
}

FLinearColor UAnimGraphNode_AzureKinectPose::GetNodeTitleColor() const
{
    return FLinearColor(0.f, 0.f, 0.f);
}

void UAnimGraphNode_AzureKinectPose::CreateOutputPins()
{
    CreatePin(EGPD_Output, UAnimationGraphSchema::PC_Struct, FComponentSpacePoseLink::StaticStruct(), TEXT("ComponentPose"));
}

#undef LOCTEXT_NAMESPACE