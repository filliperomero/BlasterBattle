// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	SetRootComponent(FlagMesh);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetAreaSphere()->SetupAttachment(FlagMesh);

	GetPickupWidget()->SetupAttachment(FlagMesh);
}

void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	if (FlagMesh) FlagMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	// Clean variables on the Server
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
	UE_LOG(LogTemp, Warning, TEXT("DROPPED FUNCTION"));
}

void AFlag::OnDropped()
{
	if (HasAuthority())
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (FlagMesh)
	{
		FlagMesh->SetSimulatePhysics(true);
		FlagMesh->SetEnableGravity(true);
		FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

		FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
		FlagMesh->MarkRenderStateDirty();
	}
	
	EnableCustomDepth(true);

	UE_LOG(LogTemp, Warning, TEXT("OnDropped FUNCTION"));
}

void AFlag::OnEquipped()
{
	ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (FlagMesh)
	{
	    FlagMesh->SetSimulatePhysics(false);
	    FlagMesh->SetEnableGravity(false);
	    FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
    EnableCustomDepth(false);

	UE_LOG(LogTemp, Warning, TEXT("OnEquipped FUNCTION"));
}
