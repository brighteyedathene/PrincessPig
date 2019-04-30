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

	APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(Pawn);
	if (PPCharacter)
	{
		// Start up blackboard and behavior tree
		if (PPCharacter->BehaviorTree->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*(PPCharacter->BehaviorTree->BlackboardAsset));
		}
		BehaviorTreeComp->StartTree(*PPCharacter->BehaviorTree);

		// Get the team (this is mainly for perception, not that it matters hugely at this point)
		SetGenericTeamId(PPCharacter->GetGenericTeamId());

		// Enable collision avoidance
		PPCharacter->SetCollisionAvoidanceEnabled(true);

		// Configure avoidance group
		FNavAvoidanceMask DefaultAvoidanceGroup;
		DefaultAvoidanceGroup.ClearAll();
		DefaultAvoidanceGroup.SetGroup(1);
		PPCharacter->GetCharacterMovement()->SetAvoidanceGroupMask(DefaultAvoidanceGroup);

		// Allow AI escapees to be lead
		PPCharacter->Replicated_CanBecomeFollower = true;
	}
}


void AEscapeeAIController::SetLeader_Implementation(APrincessPigCharacter* NewLeader)
{
	if (NewLeader)
	{
		// Write follow target to blackboard
		BlackboardComp->SetValueAsObject(FollowTargetKey, NewLeader);
	}
	else
	{
		BlackboardComp->ClearValue(FollowTargetKey);
	}
}

