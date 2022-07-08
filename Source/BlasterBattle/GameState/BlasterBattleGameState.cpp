// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterBattleGameState.h"

#include "BlasterBattle/PlayerController/BlasterPlayerController.h"
#include "BlasterBattle/PlayerState/BlasterPlayerState.h"
#include "Net/UnrealNetwork.h"

void ABlasterBattleGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterBattleGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterBattleGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterBattleGameState, BlueTeamScore);
}

void ABlasterBattleGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0) {
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore) {
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore) {
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void ABlasterBattleGameState::TeamScores(const ETeam Team)
{
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());

	if (BPlayer == nullptr) return;
	
	if (Team == ETeam::ET_BlueTeam)
	{
		++BlueTeamScore;
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
	else if (Team == ETeam::ET_RedTeam)
	{
		++RedTeamScore;
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterBattleGameState::OnRep_RedTeamScore()
{
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());

	if (BPlayer) BPlayer->SetHUDRedTeamScore(RedTeamScore);
}

void ABlasterBattleGameState::OnRep_BlueTeamScore()
{
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());

	if (BPlayer) BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
}
