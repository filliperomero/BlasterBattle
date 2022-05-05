// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "BlasterBattle/Character/BlasterCharacter.h"
#include "BlasterBattle/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
}


// This function will be called only by the Server
// And we created this one because OnRep_Score will only update the score in the clients
void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	float newScore = GetScore() + ScoreAmount;
	SetScore(newScore);

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character) {
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

		if (Controller) {
			Controller->SetHUDScore(newScore);
		}
	}
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character) {
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

		if (Controller) {
			Controller->SetHUDScore(GetScore());
		}
	}
}

// This function will be called only by the Server
void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character) {
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

		if (Controller) {
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

// This function will be called by all clients
// We don't need the Defeats value because this is already replicated
void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character) {
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

		if (Controller) {
			Controller->SetHUDDefeats(Defeats);
		}
	}
}
