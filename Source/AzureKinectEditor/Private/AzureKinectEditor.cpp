// Copyright Epic Games, Inc. All Rights Reserved.

#include "AzureKinectEditor.h"
#include "Modules/ModuleInterface.h"
#include "AzureKinectDeviceActions.h"

#define LOCTEXT_NAMESPACE "FAzureKinectEditorModule"

void FAzureKinectEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FAzureKinectDeviceActions()));
}

void FAzureKinectEditorModule::ShutdownModule()
{}

bool FAzureKinectEditorModule::SupportsDynamicReloading()
{
	return true;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAzureKinectEditorModule, AzureKinectEditor)