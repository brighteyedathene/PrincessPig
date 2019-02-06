// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PrincessPigCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"


#include "DrawDebugHelpers.h"

APrincessPigCharacter::APrincessPigCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to control rotation (this doesn't make use of RotationRate!)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	WalkSpeed = 300;
	RunSpeed = 600;
	SetRunning();

	// Collision avoidance
	// Set to false here so that no RPC is needed to disable it upon Player Possession
	// (AI controllers have no problem since they run on the server)
	GetCharacterMovement()->bUseRVOAvoidance = false;
	GetCharacterMovement()->AvoidanceConsiderationRadius = 200.f;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 1800.f;
	CameraBoom->RelativeRotation = FRotator(-65.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level
	CameraBoom->SetupAttachment(RootComponent);

	// Create top-down camera
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Create perception stimuli source
	PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
}

void APrincessPigCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}



#pragma region CollisionAvoidance
void APrincessPigCharacter::SetCollisionAvoidanceEnabled(bool Enable)
{
	GetCharacterMovement()->SetAvoidanceEnabled(Enable);
}

void APrincessPigCharacter::SetCollisionResponseToPawn(ECollisionResponse CollisionResponse)
{
	Super::GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, CollisionResponse);
}
#pragma endregion CollisionAvoidance



#pragma region MovementModes
// Movement controls (handy functions for AI)
void APrincessPigCharacter::SetWalking()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APrincessPigCharacter::SetRunning()
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}
#pragma endregion MovementModes



// IGenericTeamAgentInterface
FGenericTeamId APrincessPigCharacter::GetGenericTeamId() const
{
	return TeamId;
}
void APrincessPigCharacter::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	TeamId = TeamID;
}

