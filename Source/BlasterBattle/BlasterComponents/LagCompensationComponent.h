// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class ABlasterPlayerController;
class ABlasterCharacter;
class AWeapon;

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	// Giving UPROPERTY so it will participate in the garbage collector
	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	TMap<FName, FBoxInformation> HitBoxInfo;
	
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bIsHeadShot;
	
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTERBATTLE_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SaveFramePackages();

	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	FServerSideRewindResult ServerSideRewind(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		const float HitTime
	);

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize TraceStart,
		const FVector_NetQuantize HitLocation,
		const float HitTime,
		AWeapon* DamageCauser
	);
	
protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, const float HitTime);

	void CacheBoxPositions(const ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(const ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage);
	void ResetHitBoxes(const ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage);
	void ToggleCharacterMeshCollision(const ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		const ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation
	);

private:
	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY()
	ABlasterPlayerController* Controller;
	
	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime { 4.f };

public:	
			
};
