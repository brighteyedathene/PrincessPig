// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GenericTeamAgentInterface.h"
#include "PrincessPigCharacter.generated.h"

UCLASS(Blueprintable)
class APrincessPigCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	APrincessPigCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UAIPerceptionStimuliSourceComponent* GetPerceptionStimuliSource() { return PerceptionStimuliSource; }

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	
	/** Perception stimuli source that allows this actor to be perceived by AI agents */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Perception, meta = (AllowPrivateAccess = "true"))
	class UAIPerceptionStimuliSourceComponent* PerceptionStimuliSource;


public:
#pragma region CollisionAvoidance
	UFUNCTION(BlueprintCallable, Category = "CollisionAvoidance")
	void SetCollisionAvoidanceEnabled(bool Enable);

	UFUNCTION(BlueprintCallable, Category = "CollisionAvoidance")
	void SetCollisionResponseToPawn(ECollisionResponse CollisionResponse);
#pragma endregion CollisionAvoidance



#pragma region MovementModes
	/** Writes values to CharacterMovement */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual void SetWalking();

	/** Writes values to CharacterMovement */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual void SetRunning();

	/** Max speed while walking
	* This value is written to MaxWalkSpeed in Character Movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed;

	/** Max speed while running
	* This value is written to MaxWalkSpeed in Character Movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed;
#pragma endregion MovementModes



	// IGenericTeamAgentInterface
	FGenericTeamId TeamId;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID);

};

