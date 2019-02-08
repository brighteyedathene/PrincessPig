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
#include "Net/UnrealNetwork.h"


#include "DrawDebugHelpers.h"

APrincessPigCharacter::APrincessPigCharacter()
{
	// Probable already set as default in Super, but...
	bReplicates = true;

	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetIsReplicated(true);

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

// Just enables RVOA avoidance on the character movement component
void APrincessPigCharacter::SetCollisionAvoidanceEnabled(bool Enable)
{
	GetCharacterMovement()->SetAvoidanceEnabled(Enable);
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



#pragma region Teams
// IGenericTeamAgentInterface
FGenericTeamId APrincessPigCharacter::GetGenericTeamId() const
{
	return TeamId;
}
void APrincessPigCharacter::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	TeamId = TeamID;
}
#pragma endregion Teams



#pragma region RPCExamples

bool APrincessPigCharacter::Server_RPCExample_Validate() { return true; }
void APrincessPigCharacter::Server_RPCExample_Implementation()
{
	// As the server, call some function on all the connected clients
	Multicast_RPCExample();
}

bool APrincessPigCharacter::Multicast_RPCExample_Validate() { return true; }
void APrincessPigCharacter::Multicast_RPCExample_Implementation()
{
	// Do something on all connected clients (if server)
}

#pragma endregion RPCExamples



#pragma region Replication

// All the replicated variable need to be added in this function
void APrincessPigCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APrincessPigCharacter, Replicated_AllowOverlapPawns);
}

/* AllowOverlapPawns
We want to be able to set collision response for both client 
and server whenever an allied pawn is possessed by AI so that 
it doesn't block player movement. */
bool APrincessPigCharacter::Server_SetAllowOverlapPawns_Validate(bool AllowOverlapPawns) { return true; }
void APrincessPigCharacter::Server_SetAllowOverlapPawns_Implementation(bool AllowOverlapPawns)
{
	Replicated_AllowOverlapPawns = AllowOverlapPawns;
	OnRep_AllowOverlapPawns();
}

void APrincessPigCharacter::OnRep_AllowOverlapPawns()
{
	if (Replicated_AllowOverlapPawns)
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap);
	else
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Block);
}


#pragma endregion Replication

