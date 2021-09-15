// Fill out your copyright notice in the Description page of Project Settings.

#include "AzureKinectDeviceCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SComboBox.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "AzureKinectDeviceCustomization"

TSharedRef<IDetailCustomization> FAzureKinectDeviceCustomization::MakeInstance()
{
	return MakeShareable(new FAzureKinectDeviceCustomization());
}

void FAzureKinectDeviceCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{

	// Retrieve target object
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() == 1)
	{
		AzureKinectDevice = Cast<UAzureKinectDevice>(Objects[0].Get());
	}
	else
	{
		return;
	}

	// Customize 'AzureKinect' category
	IDetailCategoryBuilder& KinectCategory = DetailBuilder.EditCategory("Azure Kinect");
	
	{
		// Add Custom Row of Device selection
		AzureKinectDevice->DeviceList;
		CurrentOption = AzureKinectDevice->DeviceList[0];
		
		KinectCategory.AddCustomRow(LOCTEXT("DeviceSelectionFilterString", "Device Selection"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DeviceSelectionLabel", "Device Selection"))
			]
			.ValueContent()
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.IsEnabled_Raw(this, &FAzureKinectDeviceCustomization::OnGetOpened)
				.OptionsSource(&(AzureKinectDevice->DeviceList))
				.OnSelectionChanged_Raw(this, &FAzureKinectDeviceCustomization::OnSelectionChanged)
				.OnGenerateWidget_Raw(this, &FAzureKinectDeviceCustomization::MakeWidgetForOption)
				.InitiallySelectedItem(CurrentOption)
				[
					SNew(STextBlock)
					.Text(this, &FAzureKinectDeviceCustomization::GetCurrentItemLabel)
				]
			];
	}

	{

		// Alternative of UProperty specifier: meta=(EditCondition="bOpen")
		// I don't wanna make "bOpen" editable UProperty.
		// Below is a workarround how to make UProperty conditional without condition (Uproperty boolean)

		auto DepthMode = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAzureKinectDevice, DepthMode));
		auto ColorMode = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAzureKinectDevice, ColorMode));
		auto Fps = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAzureKinectDevice, Fps));
		auto SensorOrientation = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAzureKinectDevice, SensorOrientation));

		TAttribute<bool> CheckDeviceOpen(this, &FAzureKinectDeviceCustomization::OnGetOpened);

		KinectCategory.AddProperty(DepthMode).IsEnabled(CheckDeviceOpen);
		KinectCategory.AddProperty(ColorMode).IsEnabled(CheckDeviceOpen);
		KinectCategory.AddProperty(Fps).IsEnabled(CheckDeviceOpen);
		KinectCategory.AddProperty(SensorOrientation).IsEnabled(CheckDeviceOpen);
	}

	{
		// Add Custom Row of Execution buttons
		KinectCategory.AddCustomRow(LOCTEXT("ButtonFilterString", "Function Buttons"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ExecutionLabel", "Execution"))
			]
			.ValueContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(FMargin(0.f, 2.f, 10.f, 2.f))
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("LoadButtonText", "LoadDevice"))
					.Visibility_Raw(this, &FAzureKinectDeviceCustomization::OnGetPropVisibility)
					.OnClicked_Raw(this, &FAzureKinectDeviceCustomization::OnLoad)
				]
				+ SHorizontalBox::Slot()
				.Padding(FMargin(0.f, 2.f, 10.0f, 2.f))
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("StartButtonText", "StartDevice"))
					.Visibility_Raw(this, &FAzureKinectDeviceCustomization::OnGetPropVisibility)
					.OnClicked_Raw(this, &FAzureKinectDeviceCustomization::OnStart)
				]
				+ SHorizontalBox::Slot()
				.Padding(FMargin(0.f, 0, 10.f, 2.f))
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("StopButtonText", "StopDevice"))
					.Visibility_Raw(this, &FAzureKinectDeviceCustomization::OnGetPropVisibilityNegative)
					.OnClicked_Raw(this, &FAzureKinectDeviceCustomization::OnStop)
				]
			];

	}

}

TSharedRef<SWidget> FAzureKinectDeviceCustomization::MakeWidgetForOption(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock).Text(FText::FromString(*InOption));
}

void FAzureKinectDeviceCustomization::OnSelectionChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type)
{
	CurrentOption = NewValue;
	// Also update UAzureKinectDevice's current index
	int32 IndexOfFound = AzureKinectDevice->DeviceList.Find(NewValue);
	if (IndexOfFound == INDEX_NONE)
	{
		AzureKinectDevice->DeviceIndex = - 1;
	}
	else
	{
		AzureKinectDevice->DeviceIndex = IndexOfFound - 1;
	}
}

FText FAzureKinectDeviceCustomization::GetCurrentItemLabel() const
{
	if (CurrentOption.IsValid())
	{
		return FText::FromString(*CurrentOption);
	}

	return LOCTEXT("InvalidComboEntryText", "<<Invalid option>>");	
}

EVisibility FAzureKinectDeviceCustomization::OnGetPropVisibility() const
{
	return AzureKinectDevice->IsOpen() ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility FAzureKinectDeviceCustomization::OnGetPropVisibilityNegative() const
{
	return AzureKinectDevice->IsOpen() ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply FAzureKinectDeviceCustomization::OnStart()
{
	AzureKinectDevice->StartDevice();
	return FReply::Handled();
}

FReply FAzureKinectDeviceCustomization::OnLoad()
{
	AzureKinectDevice->LoadDevice();
	return FReply::Handled();
}

FReply FAzureKinectDeviceCustomization::OnStop()
{
	AzureKinectDevice->StopDevice();
	return FReply::Handled();
}

bool FAzureKinectDeviceCustomization::OnGetOpened() const
{
	return !AzureKinectDevice->IsOpen();
}

#undef LOCTEXT_NAMESPACE