// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagBattleGameMode.h"

#include "BlasterBattle/CaptureTheFlag/FlagZone.h"
#include "BlasterBattle/GameState/BlasterBattleGameState.h"
#include "BlasterBattle/Weapon/Flag.h"
#include "Kismet/GameplayStatics.h"

void ACaptureTheFlagBattleGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter,
                                                     ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterBattleGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagBattleGameMode::FlagCaptured(const AFlag* Flag, const AFlagZone* FlagZone)
{
	if (Flag == nullptr || FlagZone == nullptr) return;

	ABlasterBattleGameState* BGameState = Cast<ABlasterBattleGameState>(GameState);
	
	if (BGameState)
	{
		if (FlagZone->Team == ETeam::ET_BlueTeam || FlagZone->Team == ETeam::ET_RedTeam)
			BGameState->TeamScores(FlagZone->Team);
	}
}
