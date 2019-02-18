// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PrincessPigPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "PrincessPigCharacter.h"
#include "InteractionComponent.h"
#include "Follow.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"


#include "DrawDebugHelpers.h"

APrincessPigPlayerController::APrincessPigPlayerController()
{
	bShowMouseCursor = false;

}

void APrincessPigPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// Apply movement input
	APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(GetPawn());
	if (PPCharacter)
	{
		if (PPCharacter->IsAcceptingPlayerInput())
		{
			PPCharacter->AddMovementInput(FVector::ForwardVector, ForwardInput);
			PPCharacter->AddMovementInput(FVector::RightVector, RightInput);

			// interpolate control rotation towards the input direction added above
			UpdateControlRotation(DeltaTime);
		}
	}

	// Print the current interaction
	if (IsLocalController())
	{
		if (GetHighestPriorityInteraction())
		{
			GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID(), 0.1f, FColor::Cyan, FString("(E) ") + GetHighestPriorityInteraction()->GetName());
			DrawDebugDirectionalArrow(GetWorld(), GetPawn()->GetActorLocation(), GetHighestPriorityInteraction()->GetActorLocation(), 1000, FColor::Cyan, false, 0, 0, 6.f);
		}
	}
}

void APrincessPigPlayerController::UpdateControlRotation(float DeltaTime)
{
	if (!GetPawn()->GetVelocity().IsNearlyZero())
	{	
		float t = fminf(1, 20 * DeltaTime);
		FVector ControlForward = GetControlRotation().Quaternion().GetForwardVector();
		FVector NewControlForward = GetPawn()->GetPendingMovementInputVector() * t + ControlForward * (1 - t);
		FRotator NewControlRotation = NewControlForward.ToOrientationRotator();

		SetControlRotation(NewControlRotation);
	}
}

void APrincessPigPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &APrincessPigPlayerController::OnMoveForward);
	InputComponent->BindAxis("MoveRight", this, &APrincessPigPlayerController::OnMoveRight);

	InputComponent->BindAction("PerformAction", IE_Pressed, this, &APrincessPigPlayerController::OnPerformActionPressed);
	InputComponent->BindAction("PerformAction", IE_Released, this, &APrincessPigPlayerController::OnPerformActionReleased);

	InputComponent->BindAction("UseItem", IE_Pressed, this, &APrincessPigPlayerController::OnUseItemPressed);
	InputComponent->BindAction("UseItem", IE_Released, this, &APrincessPigPlayerController::OnUseItemReleased);

	InputComponent->BindAction("Interact", IE_Pressed, this, &APrincessPigPlayerController::OnInteractPressed);
	InputComponent->BindAction("Interact", IE_Released, this, &APrincessPigPlayerController::OnInteractReleased);

	InputComponent->BindAction("DismissFollowers", IE_Pressed, this, &APrincessPigPlayerController::OnDismissPressed);

}

void APrincessPigPlayerController::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);
	
	Pawn->Tags.Add("Leader");

	APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(Pawn);
	if (PPCharacter)
	{
		// We don't want players to become followers just yet
		PPCharacter->bCanBecomeFollower = false;
	}
}

void APrincessPigPlayerController::OnMoveForward(float Value) 
{
	ForwardInput = Value;
}

void APrincessPigPlayerController::OnMoveRight(float Value) 
{
	RightInput = Value;
}

void APrincessPigPlayerController::OnPerformActionPressed()
{
	APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(GetPawn());
	if (PPCharacter)
	{
		PPCharacter->BPEvent_PerformAction();
	}
}
void APrincessPigPlayerController::OnPerformActionReleased()
{}


void APrincessPigPlayerController::OnUseItemPressed()
{
	APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(GetPawn());
	if (PPCharacter)
	{
		PPCharacter->BPEvent_UseItem();
	}
}
void APrincessPigPlayerController::OnUseItemReleased()
{}


void APrincessPigPlayerController::OnInteractPressed()
{
	APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(GetPawn());
	if (PPCharacter)
	{
		PPCharacter->BPEvent_Interact(GetHighestPriorityInteraction());
	}
}
void APrincessPigPlayerController::OnInteractReleased()
{}


void APrincessPigPlayerController::OnDismissPressed()
{
	APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(GetPawn());
	if (PPCharacter)
	{
		PPCharacter->DismissAllFollowers();
	}
}


#pragma region Interaction

AActor* APrincessPigPlayerController::GetHighestPriorityInteraction()
{
	/* Priority ranking */
	const float PickupPriority = 5.f;
	const float EscapeePriority = 2.f;
	const float StaticInteractivePriority = 1.5f;
	const float FollowerEscapeePriority = 1.f;

	APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(GetPawn());
	if (PPCharacter)
	{
		AActor * HighestPriorityActor = nullptr;
		float HighestPriority = 0;
		for (auto & Actor : PPCharacter->AvailableInteractions)
		{
			if (!Actor)
				continue;

			if (Actor->ActorHasTag("Pickup") && HighestPriority < PickupPriority)
			{
				HighestPriorityActor = Actor;
				HighestPriority = PickupPriority;
			}

			if (Actor->ActorHasTag("Character.Escapee"))
			{
				APrincessPigCharacter* Escapee = Cast<APrincessPigCharacter>(Actor);
				if (PPCharacter->Followers.Contains(Escapee))
				{
					if (HighestPriority < FollowerEscapeePriority)
					{
						HighestPriorityActor = Actor;
						HighestPriority = FollowerEscapeePriority;
					}
				}
				else
				{
					if (HighestPriority < EscapeePriority)
					{
						HighestPriorityActor = Actor;
						HighestPriority = EscapeePriority;
					}
				}
			}

			if (Actor->ActorHasTag("Door") && HighestPriority < StaticInteractivePriority)
			{
				HighestPriorityActor = Actor;
				HighestPriority = StaticInteractivePriority;
			}

		}
		return HighestPriorityActor;

	}
	return nullptr;
}

#pragma endregion Interaction


#pragma region Followers

void APrincessPigPlayerController::SetLeader_Implementation(APrincessPigCharacter* NewLeader)
{
	// TODO implement player following
}

#pragma endregion Followers

