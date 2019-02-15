// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PrincessPigCharacter.h"
#include "InteractionComponent.h"
#include "Follow.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Net/UnrealNetwork.h"


#include "DrawDebugHelpers.h"

APrincessPigCharacter::APrincessPigCharacter()
{
	// Probable already set as default in Super, but...
	bReplicates = true;

	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetIsReplicated(true);

	// create interaction sphere
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>("InteractionComponent");
	InteractionComponent->SetupAttachment(RootComponent);
	InteractionComponent->SetRelativeLocation(FVector(70, 0, 0));
	InteractionComponent->IgnoreActorWhenMoving(this, true);


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

	FString AuthString = HasAuthority() ? FString("Auth  ") : FString("Remote   ");
	if (IsAcceptingPlayerInput())
	{
		FColor Color = HasAuthority() ? FColor::Cyan : FColor::Yellow;

		//GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID() + 991, 0.3, Color, AuthString + GetName() + FString("  is accepting input"));
	}
	else
	{
		FColor Color = HasAuthority() ? FColor::Blue : FColor::Orange;

		//GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID() + 991, 0.3, Color, AuthString + GetName() + FString("  is NOT accepting input"));

	}
}



#pragma region CollisionAvoidance

// Just enables RVOA avoidance on the character movement component
void APrincessPigCharacter::SetCollisionAvoidanceEnabled(bool Enable)
{
	GetCharacterMovement()->SetAvoidanceEnabled(Enable);
}

#pragma endregion CollisionAvoidance



#pragma region MovementModes

bool APrincessPigCharacter::IsAcceptingPlayerInput()
{
	return !Replicated_IsSubdued;
}

void APrincessPigCharacter::SetWalking()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APrincessPigCharacter::SetRunning()
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}
#pragma endregion MovementModes



#pragma region Subdue

bool APrincessPigCharacter::Server_SetSubduedDirectly_Validate(bool Subdued) { return true; }
void APrincessPigCharacter::Server_SetSubduedDirectly_Implementation(bool Subdued)
{
	GetWorld()->GetTimerManager().ClearTimer(SubdueTimer);
	Replicated_IsSubdued = Subdued;
	OnRep_IsSubdued();
}

bool APrincessPigCharacter::Server_SetSubduedFor_Validate(float Duration) { return Duration > 0; }
void APrincessPigCharacter::Server_SetSubduedFor_Implementation(float Duration)
{
	GetWorld()->GetTimerManager().SetTimer(SubdueTimer, this, &APrincessPigCharacter::OnSubdueTimerExpired, Duration);
	Replicated_IsSubdued = true;
	OnRep_IsSubdued();
}

void APrincessPigCharacter::OnRep_IsSubdued()
{
	if (Replicated_IsSubdued)
	{
		BPEvent_OnBeginSubdued();
	}
	else
	{
		BPEvent_OnEndSubdued();
	}
}

void APrincessPigCharacter::OnSubdueTimerExpired()
{
	Replicated_IsSubdued = false;
}

#pragma endregion Subdue



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



#pragma region FollowAndLead

void APrincessPigCharacter::BeginFollowing(APrincessPigCharacter* NewLeader)
{
	// Notify old leader
	if (Leader)
	{
		Leader->UpdateFollowerStatus(this, false);
	}

	Leader = NewLeader;
	
	// Notify new leader
	Leader->UpdateFollowerStatus(this, true);

	// Set new leader in controller
	IFollow* FollowInterface = Cast<IFollow>(GetController());
	if (FollowInterface)
	{
		FollowInterface->SetLeader_Implementation(Leader);
	}

	// Begin running
	SetRunning();

}

void APrincessPigCharacter::StopFollowing(APrincessPigCharacter* ThisLeader)
{
	// Compare current leader with supplied leader, only unfollow if they are the same
	// OR
	// when given nullptr, always unfollow
	if (ThisLeader == Leader || ThisLeader == nullptr)
	{
		if (Leader)
		{
			// Notify soon-to-be ex-leader
			Leader->UpdateFollowerStatus(this, false);
		}

		Leader = nullptr;

		// Set new leader (nullptr) in controller
		IFollow* FollowInterface = Cast<IFollow>(GetController());
		if (FollowInterface)
		{
			FollowInterface->SetLeader_Implementation(nullptr);
		}

		// Set walking again
		SetWalking();
	}
	else
	{
		// Correct the false leader's follower list
		ThisLeader->UpdateFollowerStatus(this, false);
	}
}

void APrincessPigCharacter::RecruitFollower(APrincessPigCharacter* NewFollower)
{
	if(NewFollower->bCanBecomeFollower && !Followers.Contains(NewFollower))
		NewFollower->BeginFollowing(this);
}

void APrincessPigCharacter::DismissFollower(APrincessPigCharacter* Follower)
{
	Follower->StopFollowing(this);
}

bool APrincessPigCharacter::DismissAllFollowers_Validate() { return true; }
void APrincessPigCharacter::DismissAllFollowers_Implementation()
{
	for (int i = Followers.Num() - 1; i >= 0; i--)
	{
		DismissFollower(Followers[i]);
	}

}

void APrincessPigCharacter::UpdateFollowerStatus(APrincessPigCharacter* Follower, bool bIsFollowing)
{
	if (bIsFollowing)
	{
		Followers.AddUnique(Follower);
	}
	else
	{
		Followers.Remove(Follower);
	}
}

#pragma endregion FollowAndLead



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

	DOREPLIFETIME(APrincessPigCharacter, Replicated_IsSubdued);
	DOREPLIFETIME(APrincessPigCharacter, Replicated_AllowOverlapPawns);
	DOREPLIFETIME(APrincessPigCharacter, Followers);
	DOREPLIFETIME(APrincessPigCharacter, Leader);
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

