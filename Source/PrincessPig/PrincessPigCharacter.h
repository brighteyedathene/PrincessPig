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

	virtual void Tick(float DeltaSeconds) override;

	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UAIPerceptionStimuliSourceComponent* GetPerceptionStimuliSource() { return PerceptionStimuliSource; }
	FORCEINLINE class UInteractionComponent* GetInteractionComponent() { return InteractionComponent; }

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
	/** Sphere collision for detecting nearby things */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	class UInteractionComponent* InteractionComponent;


#pragma region CollisionAvoidance

	UFUNCTION(BlueprintCallable, Category = "CollisionAvoidance")
	void SetCollisionAvoidanceEnabled(bool Enable);

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



#pragma region Actions

	UFUNCTION(BlueprintImplementableEvent, Category = "Actions")
	void BPEvent_PerformAction();

	UFUNCTION(BlueprintImplementableEvent, Category = "Actions")
	void BPEvent_UseItem();

#pragma endregion Actions



#pragma region Teams
	// IGenericTeamAgentInterface
	FGenericTeamId TeamId;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID);
#pragma endregion Teams



#pragma region FollowAndLead
	
	UPROPERTY(Replicated)
	APrincessPigCharacter* Leader;
	
	UPROPERTY(Replicated)
	TArray<APrincessPigCharacter*> Followers;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	bool bCanBecomeFollower;

	virtual void BeginFollowing(APrincessPigCharacter* NewLeader);
	virtual void StopFollowing(APrincessPigCharacter* ThisLeader = nullptr);

	virtual void RecruitFollower(APrincessPigCharacter* NewFollower);
	virtual void DismissFollower(APrincessPigCharacter* Follower);
	
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void DismissAllFollowers();

	virtual void UpdateFollowerStatus(APrincessPigCharacter* Follower, bool bIsFollowing);

#pragma endregion FollowAndLead



#pragma region RPCExamples

	UFUNCTION(Server, Reliable, WithValidation, Category = "RPC")
	void Server_RPCExample();

	UFUNCTION(NetMulticast, Reliable, WithValidation, Category = "RPC")
	void Multicast_RPCExample();

#pragma endregion RPCExamples



#pragma region Replication

	UPROPERTY(ReplicatedUsing = OnRep_AllowOverlapPawns)
	bool Replicated_AllowOverlapPawns; // Use Server_SetAllowedOverlapPawns to modify

	UFUNCTION(Category = "Replication")
	virtual void OnRep_AllowOverlapPawns();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Replication")
	void Server_SetAllowOverlapPawns(bool AllowOverlapPawns);

#pragma endregion Replication
};

