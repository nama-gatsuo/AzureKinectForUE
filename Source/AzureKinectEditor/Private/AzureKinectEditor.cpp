

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

#include "AzureKinectDevice.h"
#include "AzureKinectDeviceActions.h"
#include "AzureKinectDeviceCustomization.h"


#define LOCTEXT_NAMESPACE "FAzureKinectEditorModule"

class FAzureKinectEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */

	virtual void StartupModule() override
	{
		AzureKinectDeviceName = UAzureKinectDevice::StaticClass()->GetFName();
		RegisterAssetTools();
		RegisterCustomizations();
	}
	
	virtual void ShutdownModule() override
	{
		UnregisterAssetTools();
		UnregisterCustomizations();
	}
	
protected:

	/** Registers asset tool actions. */
	void RegisterAssetTools()
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FAzureKinectDeviceActions()));
	}

	/** Unregisters asset tool actions. */
	void UnregisterAssetTools()
	{
		FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");

		if (AssetToolsModule)
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();

			for (auto Action : RegisteredAssetTypeActions)
			{
				AssetTools.UnregisterAssetTypeActions(Action);
			}
		}
	}

	/**
	 * Registers a single asset type action.
	 *
	 * @param AssetTools The asset tools object to register with.
	 * @param Action The asset type action to register.
	 */
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		RegisteredAssetTypeActions.Add(Action);
	}


	/** Register details view customizations. */
	void RegisterCustomizations()
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(AzureKinectDeviceName, FOnGetDetailCustomizationInstance::CreateStatic(&FAzureKinectDeviceCustomization::MakeInstance));
	}

	void UnregisterCustomizations()
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(AzureKinectDeviceName);
	}

private:
	/** The collection of registered asset type actions. */
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;

	FName AzureKinectDeviceName;

};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAzureKinectEditorModule, AzureKinectEditor)