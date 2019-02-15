// Fill out your copyright notice in the Description page of Project Settings.

#include "BTDecorator_ObjectiveTypeEquals.h"



bool UBTDecorator_ObjectiveTypeEquals::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AGuardAIController* GuardAI = Cast<AGuardAIController>(OwnerComp.GetAIOwner());
	if (GuardAI && GuardAI->CurrentObjective)
	{
		return GuardAI->CurrentObjective->Type == ObjectiveType;
	}

	return false;
}



