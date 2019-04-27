// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "Objective.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GuardAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActorSeenDelegate, AActor*, Actor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActorSightLostDelegate, AActor*, Actor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FActorHeardDelegate, AActor*, Actor, FName, Tag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FObjectiveChangedDelegate, EObjectiveType, OldType, EObjectiveType, NewType);


/**
 * 
 */
UCLASS()
class PRINCESSPIG_API AGuardAIController : public AAIController
{
	GENERATED_BODY()

public:
	AGuardAIController();

	virtual void Tick(float DeltaSeconds) override;

	virtual void Possess(APawn* Pawn) override;

	UBehaviorTreeComponent* BehaviorTreeComp;
	UBlackboardComponent* BlackboardComp;
	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; };


#pragma region Perception

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	UAIPerceptionComponent* PerceptionComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Perception")
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Perception")
	UAISenseConfig_Hearing* HearingConfig;

	uint8 GetSightSenseID() { return (SightConfig ? SightConfig->GetSenseID() : FAISenseID::InvalidID()); };
	uint8 GetHearingSenseID() { return (HearingConfig ? HearingConfig->GetSenseID() : FAISenseID::InvalidID()); };

	// Function delegate for OnPerceptionUpdated().
	UFUNCTION(BlueprintCallable, Category = "Perception")
	void RespondToPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	// Delegate functions for perception
	UPROPERTY(BlueprintAssignable, Category = "Perception")
	FActorSeenDelegate OnActorSeen;

	UPROPERTY(BlueprintAssignable, Category = "Perception")
	FActorSightLostDelegate OnActorSightLost;

	UPROPERTY(BlueprintAssignable, Category = "Perception")
	FActorHeardDelegate OnActorHeard;

	UFUNCTION(BlueprintCallable, Category = "Perception")
	virtual void RespondToActorSeen(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Perception")
	virtual void RespondToActorSightLost(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Perception")
	virtual void RespondToActorHeard(AActor* Actor, FName Tag);

	UFUNCTION(BlueprintCallable, Category = "Perception")
	virtual void RespondToActorTouched(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Perception")
	virtual void CheckCurrentLineOfSight();

#pragma endregion Perception

#pragma region Objective
	 
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Objective")
	UObjective* CurrentObjective;

	UPROPERTY(Transient, BlueprintReadWrite, Category = "Objective")
	bool bObjectiveInSight;

	UPROPERTY(Transient, BlueprintReadWrite, Category = "Objective")
	FVector PursuitLocation;

	/* Discard CurrentObjective in favour of a new objective, or vice-versa
	* Some AI controllers might want to use their own criteria for judging 
	* the importance of different objectives, so this is  made virtual
	*/
	UFUNCTION(BlueprintCallable, Category = "Objective")
	virtual bool ShouldSetNewObjective(EObjectiveType NewType, AActor* NewtargetActor);

	UFUNCTION(BlueprintCallable, Category = "Objective")
	virtual void SetNewObjective(EObjectiveType NewType, AActor* NewtargetActor);

	UFUNCTION(BlueprintCallable, Category = "Objective")
	void WriteObjectiveToBlackboard();

	UFUNCTION(BlueprintCallable, Category = "Objective")
	float GetObjectiveDistance();

	UFUNCTION(BlueprintCallable, Category = "Objective")
	float GetEstimatedTimeToReach(FVector Location, float MaxEstimate);

	UFUNCTION(BlueprintCallable, Category = "Objective")
	virtual FVector GetObjectivePursuitLocation();

	UFUNCTION(BlueprintCallable, Category = "Objective")
	void ClearObjective();

	UFUNCTION(BlueprintCallable, Category = "Objective")
	void DowngradeObjectiveToSearch();

	// Delegate functions for objectives
	UPROPERTY(BlueprintAssignable, Category = "Objective")
	FObjectiveChangedDelegate OnObjectiveChanged;

	UFUNCTION(BlueprintCallable, Category = "Objective")
	virtual void RespondToObjectiveChanged(EObjectiveType OldType, EObjectiveType NewType);

	UFUNCTION(BlueprintImplementableEvent, Category = "Objective")
	void BPEvent_ObjectiveChanged(EObjectiveType OldType, EObjectiveType NewType);

#pragma endregion Objective


#pragma region Interaction

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool IsObjectiveInteractionAvailable();

#pragma endregion Interaction



#pragma region BlackboardKeys
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName PatrolPointKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName PatrolPointLookTargetKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName PatrolIndexKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName TimerKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName TimestampKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName TargetActorKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName ObjectiveTypeKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName ObjectiveLocationKey;

#pragma endregion BlackboardKeys


	void DebugShowObjective();
};
