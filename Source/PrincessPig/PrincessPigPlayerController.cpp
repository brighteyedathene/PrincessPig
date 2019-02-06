// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PrincessPigPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "PrincessPigCharacter.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"


#include "DrawDebugHelpers.h"

APrincessPigPlayerController::APrincessPigPlayerController()
{
	bShowMouseCursor = false;

	AvoidanceGroup.ClearAll();
	AvoidanceGroup.SetGroup(2);
}

void APrincessPigPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// Apply movement input
	if (GetPawn())
	{
		GetPawn()->AddMovementInput(FVector::ForwardVector, ForwardInput);
		GetPawn()->AddMovementInput(FVector::RightVector, RightInput);

		// interpolate control rotation towards the input direction added above
		UpdateControlRotation(DeltaTime);
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

	InputComponent->BindAction("UseItem", IE_Pressed, this, &APrincessPigPlayerController::OnUseItemPressed);
	InputComponent->BindAction("UseItem", IE_Released, this, &APrincessPigPlayerController::OnUseItemReleased);

	InputComponent->BindAxis("MoveForward", this, &APrincessPigPlayerController::OnMoveForward);
	InputComponent->BindAxis("MoveRight", this, &APrincessPigPlayerController::OnMoveRight);

}

void APrincessPigPlayerController::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);
	
	Pawn->Tags.Add("Leader");
}

void APrincessPigPlayerController::OnMoveForward(float Value) 
{
	ForwardInput = Value;
}

void APrincessPigPlayerController::OnMoveRight(float Value) 
{
	RightInput = Value;
}

void APrincessPigPlayerController::OnUseItemPressed()
{

}

void APrincessPigPlayerController::OnUseItemReleased()
{

}
