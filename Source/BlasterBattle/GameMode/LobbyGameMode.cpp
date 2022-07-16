// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance)
	{
		const UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);

		if (NumberOfPlayers == Subsystem->DesiredNumPublicConnections) {
			UWorld* World = GetWorld();

			if (World) {
				bUseSeamlessTravel = true;

				const FString MatchType = Subsystem->DesiredMatchType;
				FString MapToTravel {TEXT("")};
				
				if (MatchType == "FreeForAll") MapToTravel = TEXT("BlasterMap");
				else if (MatchType == "Teams") MapToTravel = TEXT("TeamsMap");
				else if (MatchType == "CaptureTheFlag") MapToTravel = TEXT("CaptureTheFlagMap");
				
				if (MapToTravel != "")	
					World->ServerTravel(FString::Printf(TEXT("/Game/Maps/%s?listen"), *MapToTravel));
			}
		}
	}
}
