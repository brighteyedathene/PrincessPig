// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PrincessPigCharacter.h"
#include "Guard.generated.h"

class UBehaviorTree;
class APatrolRoute;

/**
 * 
 */
UCLASS()
class PRINCESSPIG_API AGuard : public APrincessPigCharacter
{
	GENERATED_BODY()

public:

	AGuard();

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	APatrolRoute* PatrolRoute;


	UFUNCTION(BlueprintImplementableEvent, Category = "Actions")
		void BPEvent_ObjectiveReached();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Actions")
		void BPEvent_Subdue();
};
