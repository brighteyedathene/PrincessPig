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
}

void APrincessPigPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);


	// interpolate control rotation towards velocity
	UpdateControlRotation(DeltaTime);
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
	
	APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(Pawn);
	if (PPCharacter)
	{
		// Players should defalt to running
		PPCharacter->SetRunning();
	}
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
