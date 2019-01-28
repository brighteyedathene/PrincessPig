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

		APatrolPoint* PatrolPoint = Cast<APatrolPoint>(BlackboardComp->GetValueAsObject(GuardAI->PatrolPointKey));


		// Turn towards the point 
		GuardAI->SetControlRotation(PatrolPoint->GetActorRotation());

		DrawDebugLine(GetWorld(), Guard->GetActorLocation(), Guard->GetActorLocation() + Guard->GetControlRotation().Quaternion().GetForwardVector() * 300, FColor::Green, true, 4, 0, 3.f);

		Timer = 0;

		return EBTNodeResult::InProgress;
	}
	return EBTNodeResult::Failed;
}


void UBTT_WaitAtPatrolPoint::TickTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)
{
	Timer += DeltaSeconds;
	if (Timer > WaitTime)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}

}
