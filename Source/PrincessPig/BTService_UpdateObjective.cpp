// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_UpdateObjective.h"
#include "GuardAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void UBTService_UpdateObjective::TickNode(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)
{
	AGuardAIController* GuardAI = Cast<AGuardAIController>(OwnerComp.GetAIOwner());
	if (GuardAI)
	{
		GuardAI->WriteObjectiveToBlackboard();
		GuardAI->CheckCurrentLineOfSight();
	}
}

