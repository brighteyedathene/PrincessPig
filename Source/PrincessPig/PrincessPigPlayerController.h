// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Follow.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PrincessPigPlayerController.generated.h"

UCLASS()
class APrincessPigPlayerController : public APlayerController, public IFollow
{
	GENERATED_BODY()

public:
	APrincessPigPlayerController();

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual void Possess(APawn* Pawn) override;
	// End PlayerController interface

	// Gradually moves control rotation in line with movement input
	void UpdateControlRotation(float DeltaTime);


#pragma region InputEvents

	float ForwardInput;
	float RightInput;

	/** Input handlers */
	void OnMoveForward(float Value);
	void OnMoveRight(float Value);
	
	void OnUseItemPressed();
	void OnUseItemReleased();

	void OnPerformActionPressed();
	void OnPerformActionReleased();

	void OnDismissPressed();

#pragma endregion InputEvents


#pragma region Interaction
	
	UFUNCTION()
	void RespondToInteractionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void RespondToInteractionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


#pragma endregion Interaction



#pragma region FollowInterface
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Follow")
	void SetLeader(APrincessPigCharacter* NewLeader);
	virtual void SetLeader_Implementation(APrincessPigCharacter* NewLeader) override;

#pragma endregion FollowInterface

};


