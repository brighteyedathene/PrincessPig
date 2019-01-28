// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PrincessPigPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "PrincessPigCharacter.h"
#include "Engine/World.h"
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

	DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(),
		GetPawn()->GetActorLocation() + GetControlRotation().Quaternion().GetForwardVector() * 200.f, FColor::Cyan);
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

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &APrincessPigPlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &APrincessPigPlayerController::MoveToTouchLocation);

	InputComponent->BindAction("ResetVR", IE_Pressed, this, &APrincessPigPlayerController::OnResetVR);
}

void APrincessPigPlayerController::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void APrincessPigPlayerController::MoveToMouseCursor()
{
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		if (APrincessPigCharacter* MyPawn = Cast<APrincessPigCharacter>(GetPawn()))
		{
			if (MyPawn->GetCursorToWorld())
			{
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, MyPawn->GetCursorToWorld()->GetComponentLocation());
			}
		}
	}
	else
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

void APrincessPigPlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;
	GetHitResultAtScreenPosition(ScreenSpaceLocation, CurrentClickTraceChannel, true, HitResult);
	if (HitResult.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(HitResult.ImpactPoint);
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
