// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EscapeeAIController.generated.h"


class UBehaviorTreeComponent;
class UBlackboardComponent;

/**
 * 
 */
UCLASS()
class PRINCESSPIG_API AEscapeeAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEscapeeAIController();

	virtual void Possess(APawn* Pawn) override;

	UBehaviorTreeComponent* BehaviorTreeComp;
	UBlackboardComponent* BlackboardComp;
	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName FollowTargetKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName LocationToGoKey;

};
