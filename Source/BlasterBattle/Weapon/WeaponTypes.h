#pragma once

#define TRACE_LENGTH 80000.f

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

UENUM(BlueprintType)
enum class EWeaponType : uint8 {
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLaucher UMETA(DIsplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DIsplayName = "Pistol"),
	EWT_SubmachineGun UMETA(DIsplayName = "SubmachineGun"),
	EWT_Shotgun UMETA(DIsplayName = "Shotgun"),
	EWT_SniperRifle UMETA(DIsplayName = "Sniper Rifle"),
	EWT_GrenadeLauncher UMETA(DIsplayName = "Grenade Launcher"),

	EWT_MAX UMETA(DisplayName = "DefaultMax")
};