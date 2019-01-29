// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PrincessPigPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "PrincessPigCharacter.h"
#include "Engine/World.h"

#include "PrincessPigCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


#include "DrawDebugHelpers.h"

APrincessPigPlayerController::APrincessPigPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void APrincessPigPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// keep updating the destination every tick while desired
	if (bMoveToMouseCursor)
	{
		MoveToMouseCursor();
	}

	// interpolate control rotation towards velocity
	UpdateControlRotation(DeltaTime);
}

void APrincessPigPlayerController::UpdateControlRotation(float DeltaTime)
{
	if (!GetPawn()->GetVelocity().IsNearlyZero())
	{
		float RotationAlpha = fminf(1, fmaxf(0, 0.5 * DeltaTime));
		float NewYaw = GetControlRotation().Yaw * RotationAlpha + GetPawn()->GetVelocity().ToOrientationRotator().Yaw * (1 - RotationAlpha);
		FRotator NewControlRotation = GetControlRotation();
		NewControlRotation.Yaw = NewYaw;
		SetControlRotation(NewControlRotation);
	}
}

void APrincessPigPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &APrincessPigPlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &APrincessPigPlayerController::OnSetDestinationReleased);

	InputComponent->BindAction("UseItem", IE_Pressed, this, &APrincessPigPlayerController::OnUseItemPressed);
	InputComponent->BindAction("UseItem", IE_Released, this, &APrincessPigPlayerController::OnUseItemReleased);

	InputComponent->BindAxis("MoveForward", this, &APrincessPigPlayerController::OnMoveForward);
	InputComponent->BindAxis("MoveRight", this, &APrincessPigPlayerController::OnMoveRight);

}

void APrincessPigPlayerController::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);
}

void APrincessPigPlayerController::MoveToMouseCursor()
{
	{
		// Trace to see what is under the mouse cursor
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		if (Hit.bBlockingHit)
		{
			// We hit something, move there
			SetNewMoveDestination(Hit.ImpactPoint);
		}
	}
}

void APrincessPigPlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		float const Distance = FVector::Dist(DestLocation, MyPawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if ((Distance > 120.0f))
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, DestLocation);
		}
	}
}

void APrincessPigPlayerController::OnSetDestinationPressed()
{
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void APrincessPigPlayerController::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}

void APrincessPigPlayerController::OnMoveForward(float Value) 
{
	GetPawn()->AddMovementInput(FVector::ForwardVector, Value);
}

void APrincessPigPlayerController::OnMoveRight(float Value) 
{
	GetPawn()->AddMovementInput(FVector::RightVector, Value);
}

void APrincessPigPlayerController::OnUseItemPressed()
{

}

void APrincessPigPlayerController::OnUseItemReleased()
{

}
