// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_LookAround.h"

#include "GuardAIController.h"
#include "BehaviorTree/BlackboardComponent.h"


EBTNodeResult::Type UBTT_LookAround::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	bNotifyTick = true;

	AGuardAIController* GuardAI = Cast<AGuardAIController>(OwnerComp.GetAIOwner());

	if (GuardAI)
	{
		UBlackboardComponent* BlackboardComp = GuardAI->GetBlackboardComp();

		// Calculate new focal point based on the angle
		FRotator NewRotation = GuardAI->GetControlRotation();
		NewRotation.Yaw += YawDifference;
		FVector NewFocalPoint = GuardAI->GetPawn()->GetActorLocation() + NewRotation.Vector() * 300;

		// Set the focal point
		GuardAI->SetFocalPoint(NewFocalPoint, EAIFocusPriority::Gameplay);

		// Initialise the timer
		BlackboardComp->SetValueAsFloat(GuardAI->TimerKey, 0.f);

		return EBTNodeResult::InProgress;
	}
	return EBTNodeResult::Failed;
}


void UBTT_LookAround::TickTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)
{

	AGuardAIController* GuardAI = Cast<AGuardAIController>(OwnerComp.GetAIOwner());
	if (GuardAI)
	{
		UBlackboardComponent* BlackboardComp = GuardAI->GetBlackboardComp();

		// Update the wait timer
		float Timer = BlackboardComp->GetValueAsFloat(GuardAI->TimerKey);
		Timer += DeltaSeconds;

		if (Timer > WaitTime)
		{
			// Stop looking at the patrol point target
			GuardAI->ClearFocus(EAIFocusPriority::Gameplay);

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


EBTNodeResult::Type UBTT_LookAround::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	OwnerComp.GetAIOwner()->ClearFocus(EAIFocusPriority::Gameplay);

	return EBTNodeResult::Aborted;
}
