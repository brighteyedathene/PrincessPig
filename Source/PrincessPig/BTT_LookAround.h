// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_LookAround.generated.h"

/**
 * 
 */
UCLASS()
class PRINCESSPIG_API UBTT_LookAround : public UBTTaskNode
{
	GENERATED_BODY()
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, Category = "LookAround")
	float WaitTime;

	UPROPERTY(EditAnywhere, Category = "LookAround")
	float YawDifference;

};
