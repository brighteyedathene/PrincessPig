// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_SetMovementMode.h"
#include "AIController.h"

void UBTService_SetMovementMode::TickNode(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)
{
	APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (PPCharacter)
	{
		PPCharacter->SetMovementMode(MovementMode);
	}
}

