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

	// Customize 'Config' category
	IDetailCategoryBuilder& ConfigCategory = DetailBuilder.EditCategory("Config");
	
	TAttribute<bool> CheckDeviceOpen = TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateLambda(
		[this]() {
			return !AzureKinectDevice->IsOpen();
		}));

	{
		// Add Custom Row of Device selection
		CurrentOption = AzureKinectDevice->DeviceList[0];
		
		ConfigCategory.AddCustomRow(LOCTEXT("DeviceSelectionFilterString", "Device Selection"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DeviceSelectionLabel", "Device Selection"))
			]
			.ValueContent()
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.IsEnabled(CheckDeviceOpen)
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

	// Alternative of UProperty specifier: meta=(EditCondition="bOpen")
	// I don't wanna make "bOpen" editable UProperty.
	// Below is a workarround how to make UProperty conditional without condition (Uproperty boolean)
	auto DepthMode = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAzureKinectDevice, DepthMode));
	auto ColorMode = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAzureKinectDevice, ColorMode));
	auto Fps = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAzureKinectDevice, Fps));
	auto SensorOrientation = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAzureKinectDevice, SensorOrientation));
	auto RemapMode = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAzureKinectDevice, RemapMode));
	auto SkeletonTracking = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAzureKinectDevice, bSkeletonTracking));
		
	ConfigCategory.AddProperty(DepthMode).IsEnabled(CheckDeviceOpen);
	ConfigCategory.AddProperty(ColorMode).IsEnabled(CheckDeviceOpen);
	ConfigCategory.AddProperty(Fps).IsEnabled(CheckDeviceOpen);
	ConfigCategory.AddProperty(SensorOrientation).IsEnabled(CheckDeviceOpen);
	ConfigCategory.AddProperty(RemapMode).IsEnabled(CheckDeviceOpen);
	ConfigCategory.AddProperty(SkeletonTracking).IsEnabled(CheckDeviceOpen);
	

	// Customize 'IO' category
	IDetailCategoryBuilder& IOCategory = DetailBuilder.EditCategory("IO");

	// Add Custom Row of Execution buttons
	IOCategory.AddCustomRow(LOCTEXT("ButtonFilterString", "Function Buttons"))
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
				.Visibility_Lambda([this]() {
					return AzureKinectDevice->IsOpen() ? EVisibility::Collapsed : EVisibility::Visible;
				})
				.OnClicked_Lambda([this]() {
					AzureKinectDevice->LoadDevices();
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.Padding(FMargin(0.f, 2.f, 10.0f, 2.f))
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("StartButtonText", "StartDevice"))
				.Visibility_Lambda([this]() {
					return AzureKinectDevice->IsOpen() ? EVisibility::Collapsed : EVisibility::Visible;
				})
				.OnClicked_Lambda([this]() {
					AzureKinectDevice->StartDevice();
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.Padding(FMargin(0.f, 0, 10.f, 2.f))
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("StopButtonText", "StopDevice"))
				.Visibility_Lambda([this]() {
					return AzureKinectDevice->IsOpen() ? EVisibility::Visible : EVisibility::Collapsed;
				})
				.OnClicked_Lambda([this]() {
					AzureKinectDevice->StopDevice();
					return FReply::Handled();
				})
			]
		];

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

	return LOCTEXT("InvalidComboEntryText", "No Device");
}

#undef LOCTEXT_NAMESPACE