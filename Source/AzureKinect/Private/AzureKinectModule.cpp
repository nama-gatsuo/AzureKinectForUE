// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FAzureKinectModule"

class FAzureKinectModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override {};
	virtual void ShutdownModule() override {};
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAzureKinectModule, AzureKinect)