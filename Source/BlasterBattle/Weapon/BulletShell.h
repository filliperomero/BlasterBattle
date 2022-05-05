// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletShell.generated.h"

UCLASS()
class BLASTERBATTLE_API ABulletShell : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletShell();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BulletShellMesh;

	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;

public:	
	

};
