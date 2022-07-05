// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterBattleGameMode.h"
#include "TeamsBattleGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERBATTLE_API ATeamsBattleGameMode : public ABlasterBattleGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	virtual void HandleMatchHasStarted() override;
	
private:

public:
	
};
