// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterBattleGameMode.h"
#include "BlasterBattle/Character/BlasterCharacter.h"
#include "BlasterBattle/PlayerController/BlasterPlayerController.h"
#include "BlasterBattle/PlayerState/BlasterPlayerState.h"
#include "BlasterBattle/GameState/BlasterBattleGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

namespace MatchState {
	const FName Cooldown = FName("Cooldown");
}

ABlasterBattleGameMode::ABlasterBattleGameMode()
{
	bDelayedStart = true;
}

void ABlasterBattleGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart) {
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if (CountdownTime <= 0.f) {
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress) {
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if (CountdownTime <= 0.f) {
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}

}

void ABlasterBattleGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlasterBattleGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	
	// Set the MatchState for every player in the Server
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) {
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);

		if (BlasterPlayer) {
			BlasterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterBattleGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;

	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterBattleGameState* BlasterGameState = GetGameState<ABlasterBattleGameState>();


	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState) {
		AttackerPlayerState->AddToScore(1.f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);
	}

	if (VictimPlayerState) {
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimmedCharacter) {
		ElimmedCharacter->Elim();
	}
}

void ABlasterBattleGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter) {
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController) {
		TArray<AActor*> PlayersStart;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayersStart);
		int32 Selection = FMath::RandRange(0, PlayersStart.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayersStart[Selection]);
	}
}
