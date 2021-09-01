// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AssetTypeActions_Base.h"

class FAzureKinectDeviceActions : public FAssetTypeActions_Base
{
public:

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;

	//virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	//virtual void GetActions(const TArray<UObject*>&InObjects, FMenuBuilder & MenuBuilder) override;
	//virtual bool HasActions(const TArray<UObject*>& InObjects) const override;

private:
};
