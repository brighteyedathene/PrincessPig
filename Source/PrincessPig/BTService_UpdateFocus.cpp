// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_UpdateFocus.h"
#include "GuardAIController.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_UpdateFocus::UBTService_UpdateFocus()
{
	TargetKey.AddObjectFilter(this, TEXT("TargetKey"), AActor::StaticClass());
	TargetKey.AddVectorFilter(this, TEXT("TargetKey"));
}

void UBTService_UpdateFocus::TickNode(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)
{
	AAIController* OwnerAIController = OwnerComp.GetAIOwner();
	if (nullptr == OwnerAIController)
	{
		return;
	}
	APawn* OwnerPawn = OwnerAIController->GetPawn();
	if (nullptr == OwnerPawn)
	{
		return;
	}

	// If the target is an actor, check line of sight
	AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetKey.SelectedKeyName));
	if (TargetActor)
	{
		// Set focus if we have line of sight, or if we don't need it
		if (!bRequireLineOfSight || OwnerAIController->LineOfSightTo(TargetActor))
		{
			if (MaxFocusDistance == 0.0 || FVector::Distance(TargetActor->GetActorLocation(), OwnerPawn->GetActorLocation()) < MaxFocusDistance)
			{
				OwnerAIController->SetFocus(TargetActor, EAIFocusPriority::Gameplay);
				return;
			}
		}
	}
	else
	{
		const FVector TargetLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TargetKey.SelectedKeyName);
		OwnerAIController->SetFocalPoint(TargetLocation, EAIFocusPriority::Gameplay);
		return;
	}

	// if we haven't set focus and returned yet, then clear it now.
	OwnerAIController->ClearFocus(EAIFocusPriority::Gameplay);

}


