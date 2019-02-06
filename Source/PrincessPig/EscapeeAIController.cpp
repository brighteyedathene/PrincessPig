// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeeAIController.h"
#include "PrincessPigCharacter.h"
#include "Escapee.h"


#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"


AEscapeeAIController::AEscapeeAIController()
{
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

	FollowTargetKey = "FollowTarget";
	LocationToGoKey = "LocationToGo";
}


void AEscapeeAIController::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);

	AEscapee* Escapee = Cast<AEscapee>(Pawn);
	if (Escapee)
	{
		// Start up blackboard and behavior tree
		if (Escapee->BehaviorTree->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*(Escapee->BehaviorTree->BlackboardAsset));
		}
		BehaviorTreeComp->StartTree(*Escapee->BehaviorTree);

		// Get the team (this is mainly for perception, not that it matters hugely at this point)
		SetGenericTeamId(Escapee->GetGenericTeamId());

		// Escapee should start out walking
		Escapee->SetWalking();

		// Enable collision avoidance
		Escapee->SetCollisionAvoidanceEnabled(true);

		// Allow overlapping with pawns - we don't want these guys getting in the way of players
		Escapee->SetCollisionResponseToPawn(ECollisionResponse::ECR_Overlap);

	}
}




