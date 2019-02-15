// Fill out your copyright notice in the Description page of Project Settings.

#include "BTDecorator_IsObjectiveInSight.h"
#include "GuardAIController.h"


bool UBTDecorator_IsObjectiveInSight::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AGuardAIController* GuardAI = Cast<AGuardAIController>(OwnerComp.GetAIOwner());
	if (GuardAI)
	{
		return GuardAI->bObjectiveInSight;
	}

	return false;
}

