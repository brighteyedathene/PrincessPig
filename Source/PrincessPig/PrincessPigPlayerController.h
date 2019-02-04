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

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual void Possess(APawn* Pawn) override;
	// End PlayerController interface


#pragma region InputEvents

	/** Input handlers */
	void OnMoveForward(float Value);
	void OnMoveRight(float Value);
	
	void OnUseItemPressed();
	void OnUseItemReleased();

#pragma endregion InputEvents


	void UpdateControlRotation(float DeltaTime);
};


