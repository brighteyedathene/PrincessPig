// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GenericTeamAgentInterface.h"
#include "PrincessPigCharacter.generated.h"

class UTextRenderComponent;
class UBehaviorTree;
class APatrolRoute;
class AItem;

UENUM(BlueprintType)
enum class EPPMovementMode : uint8
{
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running")
};


UCLASS(Blueprintable)
class APrincessPigCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	APrincessPigCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

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

	/** Scene component for held items to attach to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
	class USceneComponent* ItemHandle;

#pragma region CollisionAvoidance

	UFUNCTION(BlueprintCallable, Category = "CollisionAvoidance")
	void SetCollisionAvoidanceEnabled(bool Enable);

#pragma endregion CollisionAvoidance



#pragma region AI

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	APatrolRoute* PatrolRoute;


#pragma endregion AI



#pragma region Teams
	// IGenericTeamAgentInterface
	UPROPERTY(EditAnywhere, BlueprintreadWrite, Category = "AI")
	FGenericTeamId TeamId;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID);
#pragma endregion Teams



#pragma region MovementModes
	
	UPROPERTY(Replicated, Transient, BlueprintReadWrite, Category = "Movement")
	EPPMovementMode MovementMode;

	/** Max speed while walking
	* This value is written to MaxWalkSpeed in Character Movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed;

	/** Max speed while running
	* This value is written to MaxWalkSpeed in Character Movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed;
	
	/** Normal max acceleration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float NormalAcceleration;

	/** Normal deceleration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float NormalDeceleration;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Movement")
		bool IsAcceptingPlayerInput();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetMovementMode(EPPMovementMode NewMovementMode);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Movement")
	void Server_SetMovementMode(EPPMovementMode NewMovementMode);

	/* Updates CharacterMovement variables to reflect movement mode and status effects */
	UFUNCTION(BlueprintCallable, Category = "Movement")
		virtual void UpdateMovementModifiers();

#pragma endregion MovementModes



#pragma region Actions

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Actions")
	void BPEvent_StartAction();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Actions")
	void BPEvent_StopAction();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Actions")
	void BPEvent_UseItem();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Actions")
	void BPEvent_Interact(AActor* InteractTarget);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Actions")
	void BPEvent_Agress();

#pragma endregion Actions



#pragma region Interaction

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Interaction")
		TArray<AActor*> AvailableInteractions;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Interaction")
		AActor* HighestPriorityInteraction;

	UFUNCTION()
		void RespondToInteractionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void RespondToInteractionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Actions")
	void Server_Interact(AActor* InteractTarget);

#pragma endregion Interaction



#pragma region Items

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Items")
	AItem* HeldItem;

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Items")
	void Server_PickUpItem(AActor* ItemActor);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Items")
	void Server_UseHeldItem();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Items")
	void Server_DropHeldItem();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Items")
		void BPEvent_OnBeginHoldItem(AActor* Item);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Items")
		void BPEvent_OnEndHoldItem(AActor* Item);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Items")
		bool IsHoldingItem() { return (HeldItem) ? true : false; };

#pragma endregion Items



#pragma region Subdue

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Subdue")
		bool IsSubdued() { return Replicated_IsSubdued;	 }

	UPROPERTY(ReplicatedUsing = OnRep_IsSubdued)
		bool Replicated_IsSubdued;

	FTimerHandle SubdueTimer;

	UFUNCTION(Category = "Subdue")
		virtual void OnRep_IsSubdued();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Subdue")
		void Server_SetSubduedDirectly(bool Subdued); 

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Subdue")
		void Server_SetSubduedFor(float Duration);

	UFUNCTION()
		void OnSubdueTimerExpired();

	UFUNCTION(BlueprintImplementableEvent, Category = "Subdue")
		void BPEvent_OnBeginSubdued();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Subdue")
		void BPEvent_OnEndSubdued();

#pragma endregion Subdue



#pragma region OffBalance
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OffBalance")
		bool IsOffBalance() { return Replicated_IsOffBalance; }

	UPROPERTY(ReplicatedUsing = OnRep_IsOffBalance)
		bool Replicated_IsOffBalance;

	FTimerHandle OffBalanceTimer;

	UFUNCTION(Category = "OffBalance")
		virtual void OnRep_IsOffBalance();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "OffBalance")
		void Server_SetOffBalanceDirectly(bool OffBalance);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "OffBalance")
		void Server_SetOffBalanceFor(float Duration);

	UFUNCTION()
		void OnOffBalanceTimerExpired();

	UFUNCTION(BlueprintImplementableEvent, Category = "OffBalance")
		void BPEvent_OnBeginOffBalance();

	UFUNCTION(BlueprintImplementableEvent, Category = "OffBalance")
		void BPEvent_OnEndOffBalance();


#pragma endregion OffBalance



#pragma region Blinded

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Blinded")
		bool IsBlinded() { return Replicated_IsBlinded; }

	UPROPERTY(ReplicatedUsing = OnRep_IsBlinded)
		bool Replicated_IsBlinded;

	FTimerHandle BlindedTimer;

	UFUNCTION(Category = "Blinded")
		virtual void OnRep_IsBlinded();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Blinded")
		void Server_SetBlindedDirectly(bool Blinded);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Blinded")
		void Server_SetBlindedFor(float Duration);

	UFUNCTION()
		void OnBlindedTimerExpired();

	UFUNCTION(BlueprintImplementableEvent, Category = "Blinded")
		void BPEvent_OnBeginBlinded();

	UFUNCTION(BlueprintImplementableEvent, Category = "Blinded")
		void BPEvent_OnEndBlinded();

#pragma endregion Blinded



#pragma region Health

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Health")
		float Replicated_MaxHealth;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Health")
		float Replicated_CurrentHealth;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Health")
		bool Replicated_IsDead;

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Health")
		void Server_TakeDamage(float Damage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Health")
		void BPEvent_OnDie();

#pragma endregion Health



#pragma region FollowAndLead
	
	UPROPERTY(Replicated)
	APrincessPigCharacter* Leader;
	
	UPROPERTY(Replicated)
	TArray<APrincessPigCharacter*> Followers;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Follow")
	bool Replicated_CanBecomeFollower;
	
	UFUNCTION(BlueprintCallable, Category = "Follow")
	virtual void BeginFollowing(APrincessPigCharacter* NewLeader);
	UFUNCTION(BlueprintCallable, Category = "Follow")
	virtual void StopFollowing(APrincessPigCharacter* ThisLeader = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Follow")
	virtual void RecruitFollower(APrincessPigCharacter* NewFollower);
	UFUNCTION(BlueprintCallable, Category = "Follow")
	virtual void DismissFollower(APrincessPigCharacter* Follower);
	
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void DismissAllFollowers();

	virtual void UpdateFollowerStatus(APrincessPigCharacter* Follower, bool bIsFollowing);

#pragma endregion FollowAndLead



#pragma region OverheadMessages

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OverheadMessages")
	UTextRenderComponent* OverheadText;

	FTimerHandle OverheadMessageTimerHandle;

	UFUNCTION(Server, Unreliable, WithValidation, BlueprintCallable, Category = "OverheadMessages")
	void Server_BroadcastOverheadMessage(float Duration, FColor Color, const FText& Message);

	UFUNCTION(NetMulticast, Unreliable, WithValidation, BlueprintCallable, Category = "OverheadMessages")
	void Multicast_ShowOverheadMessage(float Duration, FColor Color, const FText& Message);

	UFUNCTION()
	void OnMessageTimerExpired();

#pragma endregion OverheadMessages



#pragma region RPCExamples

	UFUNCTION(Server, Reliable, WithValidation, Category = "RPC")
	void Server_RPCExample();

	UFUNCTION(NetMulticast, Reliable, WithValidation, Category = "RPC")
	void Multicast_RPCExample();

#pragma endregion RPCExamples



#pragma region Replication

	UPROPERTY(ReplicatedUsing = OnRep_AllowOverlapPawns)
	bool Replicated_AllowOverlapPawns; // Use Server_SetAllowOverlapPawns to modify

	UFUNCTION(Category = "Replication")
	virtual void OnRep_AllowOverlapPawns();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Replication")
	void Server_SetAllowOverlapPawns(bool AllowOverlapPawns);

	UPROPERTY(ReplicatedUsing = OnRep_AllowOverlapDynamic)
	bool Replicated_AllowOverlapDynamic; // Use Server_SetAllowOverlapDynamic to modify

	UFUNCTION(Category = "Replication")
	virtual void OnRep_AllowOverlapDynamic();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Replication")
	void Server_SetAllowOverlapDynamic(bool AllowOverlapDynamic);

#pragma endregion Replication
};

