// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "BlasterBattle/Weapon/Weapon.h"
#include "BlasterBattle/Character/BlasterCharacter.h"
#include "BlasterBattle/PlayerController/BlasterPlayerController.h"
#include "BlasterBattle/Character/BlasterAnimInstance.h"
#include "BlasterBattle/Weapon/Projectile.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "BlasterBattle/Weapon/Shotgun.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 350.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	// The carriedAmmo only matters to the Owner of the ammo, so the variable will only be replicated to the owner that owns this component from the server, saving some bandwidth
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
	DOREPLIFETIME(UCombatComponent, bHoldingFlag);
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}

	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera()) {
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}

		if (Character->HasAuthority()) {
			InitializeCarriedAmmo();
		}
	}

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled()) {
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	
	if (bFireButtonPressed) {
		Fire();
	}
}

void UCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority()) {
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::Fire()
{
	if (CanFire()) {
		bCanFire = false;

		if (EquippedWeapon) {
			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile:
				FireProjectileWeapon();
				break;
			case EFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgun();
			}
			
			CrosshairShootingFactor = 0.75f;
		}

		StartFireTimer();
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon)
	{
		// [Lag Compensation]: We moved the TraceEndWithScatter to generate the HitTarget (when Scatter = true)
		// locally since the trace would be different from the client to the server. So we could get a
		// valid hit in the client but the server could be a invalid hit
		HitTarget = EquippedWeapon->GetUseScatter() ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		// [Lag Compensation]: We refactor this and call LocalFire so we can have all the animations before we send 
		// to the server since this will prevent a bad gameplay to the user when he is lagging
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon)
	{
		// [Lag Compensation]: We moved the TraceEndWithScatter to generate the HitTarget (when Scatter = true)
		// locally since the trace would be different from the client to the server. So we could get a
		// valid hit in the client but the server could be a invalid hit
		HitTarget = EquippedWeapon->GetUseScatter() ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		// [Lag Compensation]: We refactor this and call LocalFire so we can have all the animations before we send 
		// to the server since this will prevent a bad gameplay to the user when he is lagging
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireShotgun()
{
	if (EquippedWeapon)
	{
		AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
		if (Shotgun)
		{
			TArray<FVector_NetQuantize> HitTargets;
			Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
			if (!Character->HasAuthority()) LocalShotgunFire(HitTargets);
			ServerShotgunFire(HitTargets, EquippedWeapon->FireDelay);
		}
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;

	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) {
		return true;
	}
	
	if (bLocallyReloading) return false;
	
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	MulticastFire(TraceHitTarget);
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if (EquippedWeapon)
	{
		// Since we are comparing a float, we could have a discrepancy 
		const bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);

		return bNearlyEqual;
	}

	return true;
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	// [Lag Compensation]: If this checks return true, we already called LocalFire and we shouldn't call again
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	
	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	MulticastShotgunFire(TraceHitTargets);
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	if (EquippedWeapon)
	{
		// Since we are comparing a float, we could have a discrepancy 
		const bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);

		return bNearlyEqual;
	}

	return true;
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	// [Lag Compensation]: If this checks return true, we already called LocalFire and we shouldn't call again
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	
	LocalShotgunFire(TraceHitTargets);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	
	if (CombatState == ECombatState::ECS_Unoccupied) {
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun == nullptr || Character == nullptr) return;

	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::StartFireTimer()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &ThisClass::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;

	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic) {
		Fire();
	}
	ReloadEmptyWeapon();
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLaucher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Flag)
	{
		Character->Crouch();
		bHoldingFlag = true;
		AttachActorToSocket(WeaponToEquip, FName("FlagSocket"));
		WeaponToEquip->SetWeaponState(EWeaponState::EWS_Equipped);
		WeaponToEquip->SetOwner(Character);
	}
	else
	{
		if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
		{
			EquipSecondaryWeapon(WeaponToEquip);
		}
		else
		{
			EquipPrimaryWeapon(WeaponToEquip);
		}
		
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
	
}

void UCombatComponent::SwapWeapons()
{
	if (Character == nullptr || CombatState != ECombatState::ECS_Unoccupied || !Character->HasAuthority()) return;

	Character->PlaySwapMontage();
	CombatState = ECombatState::ECS_SwappingWeapons;
	Character->bFinishedSwapping = false;
	
	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(false);
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	DropEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	AttachActorToSocket(EquippedWeapon, FName("RightHandSocket"));

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(WeaponToEquip);

	ReloadEmptyWeapon();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	
	AttachActorToSocket(WeaponToEquip, FName("BackpackSocket"));

	SecondaryWeapon->SetOwner(Character);

	PlayEquipWeaponSound(WeaponToEquip);
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon) {
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::AttachActorToSocket(AActor* ActorToAttach, FName SocketName)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;

	const USkeletalMeshSocket* Socket = Character->GetMesh()->GetSocketByName(SocketName);

	if (Socket) {
		Socket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if (Character && WeaponToEquip && WeaponToEquip->EquipSound) {
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSound, Character->GetActorLocation());
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon && EquippedWeapon->IsEmpty()) {
		Reload();
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character) {
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToSocket(EquippedWeapon, FName("RightHandSocket"));

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;


		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->EnableCustomDepth(false);

		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character) {
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToSocket(SecondaryWeapon, FName("BackpackSocket"));
		
		PlayEquipWeaponSound(SecondaryWeapon);
	}
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 &&
		CombatState == ECombatState::ECS_Unoccupied &&
		EquippedWeapon &&
		!EquippedWeapon->IsFull() &&
		!bLocallyReloading
		)
	{
		// ServerReload will only execute on the server
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;
	if (!Character->IsLocallyControlled())
		HandleReload();
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;

	bLocallyReloading = false;

	if (Character->HasAuthority()) {
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}

	if (bFireButtonPressed) {
		Fire();
	}
}

void UCombatComponent::FinishSwap()
{
	if (Character && Character->HasAuthority())
		CombatState = ECombatState::ECS_Unoccupied;

	if (Character) Character->bFinishedSwapping = true;

	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(true);
}

void UCombatComponent::FinishSwapAttachWeapons()
{
	PlayEquipWeaponSound(EquippedWeapon);

	if (Character == nullptr || !Character->HasAuthority()) return;
	
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
	
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToSocket(EquippedWeapon, FName("RightHandSocket"));
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	
	ReloadEmptyWeapon();

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToSocket(SecondaryWeapon, FName("BackpackSocket"));
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(1);

	bCanFire = true;

	if (EquippedWeapon->IsFull() || CarriedAmmo == 0) {
		JumpToShotgunEnd();
	}
}

void UCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage()) {
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState) {
	case ECombatState::ECS_Reloading:
		if (Character && !Character->IsLocallyControlled())
			HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed) {
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled()) {
			Character->PlayThrowGrenadeMontage();
			FName SocketName =
				EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
				EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun ?
				FName("PistolSocket") :
				FName("LeftHandSocket");
			AttachActorToSocket(EquippedWeapon, SocketName);
			ShowAttachedGrenade(true);
		}
		break;
	case ECombatState::ECS_SwappingWeapons:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapMontage();
		}
		break;
	}
}

void UCombatComponent::HandleReload()
{
	if (Character == nullptr) return;
	
	Character->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;

	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}

	return 0;
}

void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) return;
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_ThrowingGrenade;

	if (Character && EquippedWeapon) {
		Character->PlayThrowGrenadeMontage();
		FName SocketName = 
			EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || 
			EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun ? 
			FName("PistolSocket") : 
			FName("LeftHandSocket");
		AttachActorToSocket(EquippedWeapon, SocketName);
		ShowAttachedGrenade(true);
	}


	if (Character && !Character->HasAuthority()) {
		ServerThrowGrenade();
	}

	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	// Prevents cheating by doing the check also in the Server
	if (Grenades == 0) return;
	
	CombatState = ECombatState::ECS_ThrowingGrenade;

	if (Character) {
		Character->PlayThrowGrenadeMontage();
		FName SocketName =
			EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
			EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun ?
			FName("PistolSocket") :
			FName("LeftHandSocket");
		AttachActorToSocket(EquippedWeapon, SocketName);
		ShowAttachedGrenade(true);
	}

	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}

bool UCombatComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToSocket(EquippedWeapon, FName("RightHandSocket"));
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();

		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
				);
		}
	}
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarriedAmmo == 0;

	if (bJumpToShotgunEnd) {
		JumpToShotgunEnd();
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;

	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld) {
		FVector Start = CrosshairWorldPosition;

		if (Character) {
			// This is a fix so we draw the lineTrace start point after the character and not from the camaera start point
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		// TODO: Can't we make this differently? Like using an enum and check like a "type" in the actor?
		// Research: Gameplay Tags.
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>()) {
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else {
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}

		// Avoid breaking the animation when aiming to the sky where we don't have a blocking hit
		if (!TraceHitResult.bBlockingHit) TraceHitResult.ImpactPoint = End;
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller) {
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;

		if (HUD) {			
			if (EquippedWeapon) {
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
			}
			else {
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
			}

			// Calculate crosshair spread
			// [0, 600] -> [0, 1] = if our maxspeed is 600
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling()) {
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else {
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming) {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bAiming) {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
	}
	else {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (Character && Character->GetFollowCamera()) {
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	// This is not needed but since we want the animation right away, we are setting here also.
	// This is because in ServerSetAiming we are also setting and the information will be sent to the server and after back to the client
	// so eventually, the aiming will be set to true but with a "lag". For aiming, it is good to have a fast feedback.
	bAiming = bIsAiming;
	// We don't need to check if we are client here to call a server function because it will not matter. We are going to run the function in the server anyway
	ServerSetAiming(bIsAiming);
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed  : BaseWalkSpeed;
	}

	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle) {
		Character->ShowSniperScopeWidget(bIsAiming);
	}

	// [Lag Compensation/Client-Side Prediction]: Variable for reconciliation 
	if (Character->IsLocallyControlled())
		bAimingButtonPressed = bIsAiming;
}

// RPC for SetAiming
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

// [Lag Compensation/Client-Side Prediction]
void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bAiming = bAimingButtonPressed;
	}
}

void UCombatComponent::OnRep_HoldingFlag()
{
	if (bHoldingFlag && Character && Character->IsLocallyControlled())
		Character->Crouch();
}
