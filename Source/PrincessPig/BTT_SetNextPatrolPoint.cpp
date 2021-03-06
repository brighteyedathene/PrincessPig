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
	APrincessPigCharacter* PPCharacter = GuardAI ? Cast<APrincessPigCharacter>(GuardAI->GetPawn()) : nullptr;

	if (PPCharacter && GuardAI)
	{
		if (PPCharacter->PatrolRoute &&
			PPCharacter->PatrolRoute->PatrolPoints.Num() > 0)
		{
			UBlackboardComponent* BlackboardComp = GuardAI->GetBlackboardComp();
			int CurrentIndex = BlackboardComp->GetValueAsInt(GuardAI->PatrolIndexKey);

			int NextIndex = (CurrentIndex + 1) % PPCharacter->PatrolRoute->PatrolPoints.Num();
			APatrolPoint* NextPatrolPoint = PPCharacter->PatrolRoute->PatrolPoints[NextIndex];
			FVector NextPatrolPointLookTarget = NextPatrolPoint->GetActorLocation() + NextPatrolPoint->GetActorForwardVector() * 200.f;
			

			BlackboardComp->SetValueAsInt(GuardAI->PatrolIndexKey, NextIndex);
			BlackboardComp->SetValueAsObject(GuardAI->PatrolPointKey, NextPatrolPoint);
			BlackboardComp->SetValueAsVector(GuardAI->PatrolPointLookTargetKey, NextPatrolPointLookTarget);

			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;

}


