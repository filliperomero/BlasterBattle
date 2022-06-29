// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "BlasterBattle/BlasterTypes/TurningInPlace.h"
#include "BlasterBattle/Interfaces/InteractWithCrosshairsInterface.h"
#include "BlasterBattle/BlasterTypes/CombatState.h"
#include "BlasterCharacter.generated.h"

class UBoxComponent;
class UAnimMontage;

UCLASS()
class BLASTERBATTLE_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	/**
	 * Play Montages
	 */
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapMontage();
	
	virtual void OnRep_ReplicatedMovement() override;

	void Elim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	// The implementation of this function does not exist in the code. The implementation is in the blueprint
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();

	void SpawnDefaultWeapon();

	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;
	
	bool bFinishedSwapping = false;
	
protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	void GrenadeButtonPressed();
	void DropOrDestroyWeapons();
	void DropOrDestroyWeapon(class AWeapon* Weapon);

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	
	// poll for any relevant classes and initialize our HUD
	void PollInit();

	virtual void Destroyed() override;
	void RotateInPlace(float DeltaTime);

	/**
	 * Hit Boxes used for server-side Rewind
	 */
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* HeadBox;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* PelvisBox;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Spine_02_Box;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Spine_03_Box;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Upperarm_L_Box;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Upperarm_R_Box;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Lowerarm_L_Box;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Lowerarm_R_Box;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Hand_L_Box;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Hand_R_Box;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* BackpackBox;
	
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* BlanketBox;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Thigh_L_Box;
	
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Thigh_R_Box;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Calf_L_Box;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Calf_R_Box;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Foot_L_Box;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
	UBoxComponent* Foot_R_Box;
	
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	/**
	 * Blaster Components
	 */
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensation;

	// RPC that will be called by the client in the server
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	/**
	* Animation montages
	*/

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapMontage;
	
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	void HideCameraIfCharacterClose();
	void HideWeapons(bool bShouldHide);

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/**
	* Player health
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/*
	 * Player Shield
	 */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 10.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	void ElimTimerFinished();

	/**
	* Disolve Effect
	*/

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	// Dynamic Instance that we can change at runtme
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolevMaterialInstance;

	// Material Instanec set on the blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	/**
	* Elim bot
	*/

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

	/**
	* Grenade
	*/

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/*
	 * Default Weapon
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool isWeaponEquipped();
	bool isAiming();

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone;	}
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; } 
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; } 
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	bool IsLocallyReloading();
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
};
