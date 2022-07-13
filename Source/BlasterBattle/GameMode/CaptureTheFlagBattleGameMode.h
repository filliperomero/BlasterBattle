// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsBattleGameMode.h"
#include "CaptureTheFlagBattleGameMode.generated.h"

class AFlag;
class AFlagZone;

/**
 * 
 */
UCLASS()
class BLASTERBATTLE_API ACaptureTheFlagBattleGameMode : public ATeamsBattleGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;
	void FlagCaptured(const AFlag* Flag, const AFlagZone* FlagZone);
};
