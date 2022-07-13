// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"

#include "BlasterBattle/GameMode/CaptureTheFlagBattleGameMode.h"
#include "BlasterBattle/Weapon/Flag.h"
#include "Components/SphereComponent.h"

AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;
	
	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Zone Sphere"));
	SetRootComponent(ZoneSphere);
	ZoneSphere->SetSphereRadius(90.f);
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();

	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);

	if (OverlappingFlag && OverlappingFlag->GetTeam() != Team)
	{
		ACaptureTheFlagBattleGameMode* GameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagBattleGameMode>();

		// will only succeed in the Server
		if (GameMode)
		{
			GameMode->FlagCaptured(OverlappingFlag, this);
		}
		
		OverlappingFlag->ResetFlag();
	}
}
