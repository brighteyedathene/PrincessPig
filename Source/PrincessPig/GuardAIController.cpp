// Fill out your copyright notice in the Description page of Project Settings.

#include "GuardAIController.h"
#include "Guard.h"
#include "PatrolPoint.h"
#include "PatrolRoute.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"

#include "DrawDebugHelpers.h"

AGuardAIController::AGuardAIController()
{
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));

	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

	PatrolPointKey = "PatrolPoint";
	PatrolIndexKey = "PatrolIndex";
}

void AGuardAIController::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);

	AGuard* Guard = Cast<AGuard>(Pawn);
	if (Guard)
	{
		if (Guard->BehaviorTree->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*(Guard->BehaviorTree->BlackboardAsset));
		}

		BehaviorTreeComp->StartTree(*Guard->BehaviorTree);
	}

}

void AGuardAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(),
		GetPawn()->GetActorLocation() + GetControlRotation().Quaternion().GetForwardVector() * 200.f, FColor::Cyan);
}

