// Fill out your copyright notice in the Description page of Project Settings.


#include "AzureKinectDeviceActions.h"
#include "AzureKinectDevice.h"

FText FAzureKinectDeviceActions::GetName() const
{
	return NSLOCTEXT("AzureKinectDeviceActions", "AssetTypeActions_AzureKinectDevice", "Azure Kinect Device");
}

FColor FAzureKinectDeviceActions::GetTypeColor() const
{
	return FColor::White;
}

UClass* FAzureKinectDeviceActions::GetSupportedClass() const
{
	return UAzureKinectDevice::StaticClass();
}

uint32 FAzureKinectDeviceActions::GetCategories()
{
	return EAssetTypeCategories::Media;
}
