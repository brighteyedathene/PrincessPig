// Fill out your copyright notice in the Description page of Project Settings.

#include "BTDecorator_IsObjectiveInRange.h"
#include "GuardAIController.h"



bool UBTDecorator_IsObjectiveInRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AGuardAIController* GuardAI = Cast<AGuardAIController>(OwnerComp.GetAIOwner());
	if (GuardAI)
	{
		return GuardAI->IsObjectiveInteractionAvailable();
	}
	
	return false;
}

