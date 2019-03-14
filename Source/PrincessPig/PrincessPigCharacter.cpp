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
#include "Item.h"

#include "DrawDebugHelpers.h"

APrincessPigCharacter::APrincessPigCharacter()
{
	// Probable already set as default in Super, but...
	bReplicates = true;

	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetIsReplicated(true);

	// Create interaction sphere
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>("InteractionComponent");
	InteractionComponent->SetupAttachment(RootComponent);
	InteractionComponent->SetRelativeLocation(FVector(70, 0, 0));
	InteractionComponent->IgnoreActorWhenMoving(this, true);
	InteractionComponent->OnComponentBeginOverlap.AddDynamic(this, &APrincessPigCharacter::RespondToInteractionBeginOverlap);
	InteractionComponent->OnComponentEndOverlap.AddDynamic(this, &APrincessPigCharacter::RespondToInteractionEndOverlap);

	// Create item handle for non-stowable items
	ItemHandle = CreateDefaultSubobject<USceneComponent>("ItemHandle");
	if (GetMesh())
	{
		ItemHandle->SetupAttachment(GetMesh(), FName("ItemSocket"));
		ItemHandle->SetRelativeLocation(FVector(0, 0, 0));
		ItemHandle->SetRelativeRotation(FRotator(0, 0, 0));
	}
	else
	{
		ItemHandle->SetupAttachment(RootComponent);
		ItemHandle->SetRelativeLocation(FVector(50, 30, 0));
		ItemHandle->SetRelativeRotation(FRotator(0, 0, -25));
	}

	// Don't rotate character to control rotation (this doesn't make use of RotationRate!)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	WalkSpeed = 300;
	RunSpeed = 600;
	NormalAcceleration = 2048;
	NormalDeceleration = 2048;

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
	CameraBoom->TargetArmLength = 2200.f;
	CameraBoom->RelativeRotation = FRotator(280.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 0.5;
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

	// Configure Health
	Replicated_MaxHealth = 1.f;
	Replicated_CurrentHealth = Replicated_MaxHealth;
	Replicated_IsDead = false;
}

void APrincessPigCharacter::BeginPlay()
{
	Super::BeginPlay();

	Replicated_CurrentHealth = Replicated_MaxHealth;
	Replicated_IsDead = false;

	UpdateMovementModifiers();
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

bool APrincessPigCharacter::IsAcceptingPlayerInput()
{
	return !Replicated_IsSubdued;
}

void APrincessPigCharacter::SetMovementMode(EPPMovementMode NewMovementMode)
{
	MovementMode = NewMovementMode;
	UpdateMovementModifiers();
}

bool APrincessPigCharacter::Server_SetMovementMode_Validate(EPPMovementMode NewMovementMode) { return true; }
void APrincessPigCharacter::Server_SetMovementMode_Implementation(EPPMovementMode NewMovementMode)
{
	SetMovementMode(NewMovementMode);
}


void APrincessPigCharacter::UpdateMovementModifiers()
{
	// Acceleration
	if (Replicated_IsOffBalance)
	{
		GetCharacterMovement()->MaxAcceleration = 0;
		GetCharacterMovement()->BrakingDecelerationWalking = 0;
		GetCharacterMovement()->BrakingFrictionFactor = 0.01;
	}
	else
	{
		GetCharacterMovement()->MaxAcceleration = NormalAcceleration;
		GetCharacterMovement()->BrakingDecelerationWalking = NormalDeceleration;
		GetCharacterMovement()->BrakingFrictionFactor = 1;
	}

	// Max speed
	if (Replicated_IsSubdued)
	{
		GetCharacterMovement()->MaxWalkSpeed = 0;
	}
	else
	{
		switch (MovementMode)
		{
		case EPPMovementMode::Walking:
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			break;
		case EPPMovementMode::Running:
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
			break;
		}
	}

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
	// In case blueprints might want to know about being subued (eg, saying 'ouch' or displaying some particle)
	if (Replicated_IsSubdued)
	{
		BPEvent_OnBeginSubdued();
	}
	else
	{
		BPEvent_OnEndSubdued();
	}

	// Being subdued can affect movement capabilities
	UpdateMovementModifiers();
}

void APrincessPigCharacter::OnSubdueTimerExpired()
{
	Replicated_IsSubdued = false;
	OnRep_IsSubdued();
}

#pragma endregion Subdue



#pragma region OffBalance

bool APrincessPigCharacter::Server_SetOffBalanceDirectly_Validate(bool OffBalance) { return true; }
void APrincessPigCharacter::Server_SetOffBalanceDirectly_Implementation(bool OffBalance)
{
	GetWorld()->GetTimerManager().ClearTimer(OffBalanceTimer);
	Replicated_IsOffBalance = OffBalance;
	OnRep_IsOffBalance();
}

bool APrincessPigCharacter::Server_SetOffBalanceFor_Validate(float Duration) { return Duration > 0; }
void APrincessPigCharacter::Server_SetOffBalanceFor_Implementation(float Duration)
{
	GetWorld()->GetTimerManager().SetTimer(OffBalanceTimer, this, &APrincessPigCharacter::OnOffBalanceTimerExpired, Duration);
	Replicated_IsOffBalance = true;
	OnRep_IsOffBalance();
}

void APrincessPigCharacter::OnRep_IsOffBalance()
{
	if (Replicated_IsOffBalance)
	{
		BPEvent_OnBeginOffBalance();
	}
	else
	{
		BPEvent_OnEndOffBalance();
	}

	// OffBalance affects movement capabilities
	UpdateMovementModifiers();
}

void APrincessPigCharacter::OnOffBalanceTimerExpired()
{
	Replicated_IsOffBalance = false;
	OnRep_IsOffBalance();
}

#pragma endregion OffBalance



#pragma region Health

bool APrincessPigCharacter::Server_TakeDamage_Validate(float Damage) { return true; }
void APrincessPigCharacter::Server_TakeDamage_Implementation(float Damage)
{
	Replicated_CurrentHealth = Replicated_CurrentHealth - Damage;
	if (Replicated_CurrentHealth <= 0)
	{
		Replicated_IsDead = true;

		// Drop any held item
		Server_DropHeldItem();

		// Disable collision with doors and other pawns on death
		Server_SetAllowOverlapPawns(true);
		Server_SetAllowOverlapDynamic(true);

		// Disable movement and collision avoidance
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetAvoidanceEnabled(false);

		// Call the blueprint event for blueprint effects
		BPEvent_OnDie();
	}
}

#pragma endregion Health




#pragma region Interaction

void APrincessPigCharacter::RespondToInteractionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(HasAuthority())
	{
		if (OtherActor != this)
		{
			AvailableInteractions.Add(OtherActor);
		}

	}
}

void APrincessPigCharacter::RespondToInteractionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasAuthority())
	{
		if (OtherActor != this)
		{
			AvailableInteractions.Remove(OtherActor);
		}

	}
}

bool APrincessPigCharacter::Server_Interact_Validate(AActor* InteractTarget) { return true; }
void APrincessPigCharacter::Server_Interact_Implementation(AActor* InteractTarget)
{
	if (InteractTarget && AvailableInteractions.Contains(InteractTarget))
	{
		GEngine->AddOnScreenDebugMessage(123445, 6.f, FColor::White, FString("Interacting with ") + InteractTarget->GetName());
		if (InteractTarget->ActorHasTag("Item"))
		{
			Server_PickUpItem(InteractTarget);
		}

		else if (InteractTarget->ActorHasTag("Character.Escapee"))
		{
			APrincessPigCharacter* Escapee = Cast<APrincessPigCharacter>(InteractTarget);
			if (Escapee)
			{
				GEngine->AddOnScreenDebugMessage(123445, 6.f, FColor::White, FString("Recruiting ") + InteractTarget->GetName());
				RecruitFollower(Escapee);
			}
		}
	}
}

#pragma endregion Interaction



#pragma region Items

bool APrincessPigCharacter::Server_PickUpItem_Validate(AActor* ItemActor) { return true; }
void APrincessPigCharacter::Server_PickUpItem_Implementation(AActor* ItemActor)
{
	if (ItemActor && AvailableInteractions.Contains(ItemActor))
	{
		AItem* Item = Cast<AItem>(ItemActor);
		if (Item)
		{
			if (!HeldItem)
			{
				HeldItem = Item;
				HeldItem->BPEvent_OnPickedUp(this);
			}
			else
			{
				// must drop held item to pick this up!
			}
			
		}
	}
}

bool APrincessPigCharacter::Server_UseHeldItem_Validate() { return true; }
void APrincessPigCharacter::Server_UseHeldItem_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString("Trying to use held item..."));
	if (HeldItem)
	{
		HeldItem->Use(this);
	}
}

bool APrincessPigCharacter::Server_DropHeldItem_Validate() { return true; }
void APrincessPigCharacter::Server_DropHeldItem_Implementation()
{
	if (HeldItem)
	{
		HeldItem->BPEvent_OnDropped();
		HeldItem = nullptr;
	}
}


#pragma endregion Items



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
	SetMovementMode(EPPMovementMode::Running);

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
	}
	else
	{
		// Correct the false leader's follower list
		ThisLeader->UpdateFollowerStatus(this, false);
	}
}

void APrincessPigCharacter::RecruitFollower(APrincessPigCharacter* NewFollower)
{
	if(NewFollower->Replicated_CanBecomeFollower && !Followers.Contains(NewFollower))
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

	DOREPLIFETIME(APrincessPigCharacter, HeldItem);
	DOREPLIFETIME(APrincessPigCharacter, MovementMode);
	DOREPLIFETIME(APrincessPigCharacter, Replicated_MaxHealth);
	DOREPLIFETIME(APrincessPigCharacter, Replicated_CurrentHealth);
	DOREPLIFETIME(APrincessPigCharacter, Replicated_IsDead);
	DOREPLIFETIME(APrincessPigCharacter, Replicated_IsSubdued);
	DOREPLIFETIME(APrincessPigCharacter, Replicated_IsOffBalance);
	DOREPLIFETIME(APrincessPigCharacter, Replicated_AllowOverlapPawns);
	DOREPLIFETIME(APrincessPigCharacter, Replicated_AllowOverlapDynamic);
	DOREPLIFETIME(APrincessPigCharacter, AvailableInteractions);
	DOREPLIFETIME(APrincessPigCharacter, Replicated_CanBecomeFollower);
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
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap);
	}
	else
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Block);
	}
}
/* AllowOverlap Dynamic
This is for doors! when characters die, we don't really want them stopping doors
*/
bool APrincessPigCharacter::Server_SetAllowOverlapDynamic_Validate(bool AllowOverlapDynamic) { return true; }
void APrincessPigCharacter::Server_SetAllowOverlapDynamic_Implementation(bool AllowOverlapDynamic)
{
	Replicated_AllowOverlapDynamic = AllowOverlapDynamic;
	OnRep_AllowOverlapDynamic();
}

void APrincessPigCharacter::OnRep_AllowOverlapDynamic()
{
	if (Replicated_AllowOverlapDynamic)
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECR_Overlap);
	}
	else
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECR_Block);
	}
}


#pragma endregion Replication

