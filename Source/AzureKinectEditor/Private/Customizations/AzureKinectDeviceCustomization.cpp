// Fill out your copyright notice in the Description page of Project Settings.

#include "AzureKinectDeviceCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SComboBox.h"


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
	IDetailCategoryBuilder& OverridesCategory = DetailBuilder.EditCategory("Azure Kinect");
	
	{
		// Add Custom Row
		OverridesCategory.AddCustomRow(LOCTEXT("FilterString", "Search Filter Keywords"))
			.WholeRowContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TestString", "This is a very temporal comment."))
			];
	}

	{
		// Add Custom Row
		AzureKinectDevice->DeviceList;
		CurrentOption = AzureKinectDevice->DeviceList[0];

		OverridesCategory.AddCustomRow(LOCTEXT("DeviceSelectionFilterString", "Device Selection"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DeviceSelectionLabel", "Device Selection"))
			]
			.ValueContent()
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&(AzureKinectDevice->DeviceList))
				.OnSelectionChanged(this, &FAzureKinectDeviceCustomization::OnSelectionChanged)
				.OnGenerateWidget(this, &FAzureKinectDeviceCustomization::MakeWidgetForOption)
				.InitiallySelectedItem(CurrentOption)
				[
					SNew(STextBlock)
					.Text(this, &FAzureKinectDeviceCustomization::GetCurrentItemLabel)
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

#undef LOCTEXT_NAMESPACE