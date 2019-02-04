// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_WaitAtPatrolPoint.h"
#include "GuardAIController.h"
#include "Guard.h"
#include "PatrolPoint.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "DrawDebugHelpers.h"

EBTNodeResult::Type UBTT_WaitAtPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	bNotifyTick = true;

	AGuardAIController* GuardAI = Cast<AGuardAIController>(OwnerComp.GetAIOwner());
	AGuard* Guard = GuardAI ? Cast<AGuard>(GuardAI->GetPawn()) : nullptr;

	if (Guard && GuardAI)
	{
		UBlackboardComponent* BlackboardComp = GuardAI->GetBlackboardComp();

		// Look in the appropriate direction for the current patrol point
		GuardAI->SetFocalPoint(BlackboardComp->GetValueAsVector(GuardAI->PatrolPointLookTargetKey), EAIFocusPriority::Gameplay);

		// Initialise the patrol point wait timer
		BlackboardComp->SetValueAsFloat(GuardAI->TimerKey, 0.f);

		return EBTNodeResult::InProgress;
	}
	return EBTNodeResult::Failed;
}


void UBTT_WaitAtPatrolPoint::TickTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)
{

	AGuardAIController* GuardAI = Cast<AGuardAIController>(OwnerComp.GetAIOwner());
	if (GuardAI)
	{
		UBlackboardComponent* BlackboardComp = GuardAI->GetBlackboardComp();

		// Update the wait timer
		float Timer = BlackboardComp->GetValueAsFloat(GuardAI->TimerKey);
		Timer += DeltaSeconds;

		// TODO: Add a WaitTime variable to APatrolPoint and read from it here
		if (Timer > WaitTime)
		{
			// Stop looking at the patrol point target
			GuardAI->ClearFocus(EAIFocusPriority::Gameplay);
			;

			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		else
		{
			BlackboardComp->SetValueAsFloat(GuardAI->TimerKey, Timer);
		}
	}
	else
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}

}
