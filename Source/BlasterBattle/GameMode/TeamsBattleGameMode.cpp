// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsBattleGameMode.h"

#include "BlasterBattle/GameState/BlasterBattleGameState.h"
#include "BlasterBattle/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"

ATeamsBattleGameMode::ATeamsBattleGameMode()
{
	bTeamsMatch = true;
}

// Function to add a new player in Mid Game
void ATeamsBattleGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABlasterBattleGameState* BGameState = Cast<ABlasterBattleGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* NewPlayerState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
	
	if (BGameState && NewPlayerState && NewPlayerState->GetTeam() == ETeam::ET_NoTeam)
	{
		if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
		{
			BGameState->RedTeam.AddUnique(NewPlayerState);
			NewPlayerState->SetTeam(ETeam::ET_RedTeam);
		}
		else
		{
			BGameState->BlueTeam.AddUnique(NewPlayerState);
			NewPlayerState->SetTeam(ETeam::ET_BlueTeam);
		}
	}
}

// Function to remove a player from a team when he exit the game
void ATeamsBattleGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ABlasterBattleGameState* BGameState = Cast<ABlasterBattleGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* ExitingPlayerState = Exiting->GetPlayerState<ABlasterPlayerState>();
	
	if (BGameState && ExitingPlayerState)
	{
		if (BGameState->RedTeam.Contains(ExitingPlayerState)) BGameState->RedTeam.Remove(ExitingPlayerState);
		else if (BGameState->BlueTeam.Contains(ExitingPlayerState)) BGameState->BlueTeam.Remove(ExitingPlayerState);
	}
}

void ATeamsBattleGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlasterBattleGameState* BGameState = Cast<ABlasterBattleGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		for (auto PState : BGameState->PlayerArray)
		{
			ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());
			if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

float ATeamsBattleGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	if (Attacker == nullptr || Victim == nullptr) return BaseDamage;

	ABlasterPlayerState* AttackerPState = Attacker->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPState = Victim->GetPlayerState<ABlasterPlayerState>();

	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;

	if (AttackerPState == VictimPState) return BaseDamage;

	if (AttackerPState->GetTeam() == VictimPState->GetTeam()) return 0.f;

	return BaseDamage;
}
