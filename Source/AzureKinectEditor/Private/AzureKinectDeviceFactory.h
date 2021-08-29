// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "AzureKinectDeviceFactory.generated.h"

/**
 * Factory class of UAzureKinectDevice.
 * UAzureKinectDevice can be Asset Type due to this factory class.
 */
UCLASS(hidecategories=Object)
class UAzureKinectDeviceFactory : public UFactory
{
	GENERATED_UCLASS_BODY()
public:

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
	virtual uint32 GetMenuCategories() const override;
};
