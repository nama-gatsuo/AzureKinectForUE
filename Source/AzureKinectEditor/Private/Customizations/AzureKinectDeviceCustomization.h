// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "AzureKinectDevice.h"
/**
 * 
 */
class FAzureKinectDeviceCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	
	TSharedRef<SWidget> MakeWidgetForOption(TSharedPtr<FString> InOption);
	void OnSelectionChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type);
	FText GetCurrentItemLabel() const;
	EVisibility OnGetPropVisibility() const;
	EVisibility OnGetPropVisibilityNegative() const;
	FReply OnStart();
	FReply OnLoad();
	FReply OnStop();
	bool OnGetOpened() const;

private:

	TWeakObjectPtr<UAzureKinectDevice> AzureKinectDevice;
	TSharedPtr<FString> CurrentOption;
	
};
