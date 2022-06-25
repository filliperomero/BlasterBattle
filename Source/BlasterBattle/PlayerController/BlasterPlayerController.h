// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERBATTLE_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);

	// Synced with server world clock
	virtual float GetServerTime();
	// Sync with server clock as soon as possible
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleCooldown();

	float SingleTripTime = 0.f;

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();

	/**
	* Sync Time between Client and Server
	*/

	// Request the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	// Difference between Client and server time
	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	void HighPingWarning(int32 PingAmount);
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);
	
private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	UPROPERTY()
	class ABlasterBattleGameMode* BlasterBattleGameMode;

	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float LevelStartingTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bInitializeHealth = false;
	bool bInitializeShield = false;
	bool bInitializeScore = false;
	bool bInitializeDefeats = false;
	bool bInitializeGrenades = false;
	bool bInitializeCarriedAmmo = false;
	bool bInitializeWeaponAmmo = false;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	float HUDScore;
	int32 HUDDefeats;
	int32 HUDGrenades;
	float HUDCarriedAmmo;
	float HUDWeaponAmmo;

	/*
	 * Ping
	 */
	float HighPingRunningTime = 0.f;
	
	UPROPERTY(EditAnywhere, Category = Ping)
	float HighPintDuration = 5.f;
	
	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere, Category = Ping)
	float CheckPingFrequency = 20.f;

	UPROPERTY(EditAnywhere, Category = Ping)
	float HighPingThreshold = 50.f;
		
};
