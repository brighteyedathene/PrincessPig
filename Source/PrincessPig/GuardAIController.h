// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GuardAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
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

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName PatrolPointKey;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName PatrolPointTargetKey;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName PatrolIndexKey;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TimerKey;

};
