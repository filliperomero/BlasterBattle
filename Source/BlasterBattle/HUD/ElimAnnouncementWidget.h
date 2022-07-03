// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ElimAnnouncementWidget.generated.h"

class UHorizontalBox;
class UTextBlock;

/**
 * 
 */
UCLASS()
class BLASTERBATTLE_API UElimAnnouncementWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* AnnouncementBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementText;

	void SetElimAnnouncementText(const FString AttackerName, const FString VictimName);
};
