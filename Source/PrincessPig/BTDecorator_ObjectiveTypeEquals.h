// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "GuardAIController.h"


#include "BTDecorator_ObjectiveTypeEquals.generated.h"


/**
 * 
 */
UCLASS()
class PRINCESSPIG_API UBTDecorator_ObjectiveTypeEquals : public UBTDecorator
{
	GENERATED_BODY()

public:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere, Category = "Objective")
	EObjectiveType ObjectiveType;
};
