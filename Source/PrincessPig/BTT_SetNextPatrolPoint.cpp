// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_SetNextPatrolPoint.h"
#include "GuardAIController.h"
#include "Guard.h"
#include "PatrolPoint.h"
#include "PatrolRoute.h"
#include "BehaviorTree/BlackboardComponent.h"


EBTNodeResult::Type UBTT_SetNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AGuardAIController* GuardAI = Cast<AGuardAIController>(OwnerComp.GetAIOwner());
	AGuard* Guard = GuardAI ? Cast<AGuard>(GuardAI->GetPawn()) : nullptr;

	if (Guard && GuardAI)
	{
		if (Guard->PatrolRoute->PatrolPoints.Num() > 0)
		{
			UBlackboardComponent* BlackboardComp = GuardAI->GetBlackboardComp();
			int CurrentIndex = BlackboardComp->GetValueAsInt(GuardAI->PatrolIndexKey);

			int NextIndex = (CurrentIndex + 1) % Guard->PatrolRoute->PatrolPoints.Num();
			APatrolPoint* NextPatrolPoint = Guard->PatrolRoute->PatrolPoints[NextIndex];

			BlackboardComp->SetValueAsInt(GuardAI->PatrolIndexKey, NextIndex);
			BlackboardComp->SetValueAsObject(GuardAI->PatrolPointKey, NextPatrolPoint);

			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;

}


