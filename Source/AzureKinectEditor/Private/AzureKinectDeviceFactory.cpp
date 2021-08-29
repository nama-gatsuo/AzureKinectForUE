// Fill out your copyright notice in the Description page of Project Settings.


#include "AzureKinectDeviceFactory.h"
#include "AzureKinectDevice.h"
#include "AssetTypeCategories.h"

UAzureKinectDeviceFactory::UAzureKinectDeviceFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UAzureKinectDevice::StaticClass();
}

UObject* UAzureKinectDeviceFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UAzureKinectDevice>(InParent, InClass, InName, Flags);
}

bool UAzureKinectDeviceFactory::ShouldShowInNewMenu() const
{
	return true;
}

uint32 UAzureKinectDeviceFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}
