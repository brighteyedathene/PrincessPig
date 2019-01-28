// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PrincessPigPlayerController.generated.h"

UCLASS()
class APrincessPigPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APrincessPigPlayerController();

//protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual void Possess(APawn* Pawn) override;
	// End PlayerController interface


#pragma region InputEvents
	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Navigate player to the current mouse cursor location. */
	void MoveToMouseCursor();

	/** Navigate player to the current touch location. */
	void MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location);
	
	/** Navigate player to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);

	/** Input handlers */
	void OnSetDestinationPressed();
	void OnSetDestinationReleased();

	void OnMoveForward(float Value);
	void OnMoveRight(float Value);
	
	void OnUseItemPressed();
	void OnUseItemReleased();

#pragma endregion InputEvents


	void UpdateControlRotation(float DeltaTime);
};


