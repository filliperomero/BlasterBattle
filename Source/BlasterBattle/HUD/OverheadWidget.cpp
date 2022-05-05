// Fill out your copyright notice in the Description pags.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText) {
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::SetDisplayName(FString NameToDisplay)
{
	if (DisplayName) {
		DisplayName->SetText(FText::FromString(NameToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole RemoteRole = InPawn->GetRemoteRole();
	
	FString Role;

	switch (RemoteRole)
	{
	case ROLE_None:
		Role = FString("None");
		break;
	case ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy");
		break;
	case ROLE_AutonomousProxy:
		Role = FString("Autonomous Proxy");
		break;
	case ROLE_Authority:
		Role = FString("Authority");
		break;
	case ROLE_MAX:
		break;
	default:
		break;
	}

	FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);

	SetDisplayText(RemoteRoleString);
}

void UOverheadWidget::ShowPlayerName(APawn* InPawn)
{
	APlayerState* PlayerState = InPawn->GetPlayerState();

	if (PlayerState) {
		FString PlayerName = PlayerState->GetPlayerName();
		SetDisplayName(PlayerName);
	}
}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();

	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}
