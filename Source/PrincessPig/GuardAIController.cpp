// Fill out your copyright notice in the Description page of Project Settings.

#include "GuardAIController.h"
#include "PrincessPigCharacter.h"
#include "Guard.h"
#include "PatrolPoint.h"
#include "PatrolRoute.h"
#include "Objective.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
//#include "Perception/AISense.h"
//#include "Perception/AISense_Sight.h"
//#include "Perception/AISense_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GenericTeamAgentInterface.h"

#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

AGuardAIController::AGuardAIController()
{
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

	if (SightConfig)
	{
		// Sight configuration
		SightConfig->SetMaxAge(1.f);
		SightConfig->SightRadius = 1000;
		SightConfig->LoseSightRadius = 1500;
		SightConfig->PeripheralVisionAngleDegrees = 60.0f;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

		PerceptionComp->ConfigureSense(*SightConfig);
		PerceptionComp->SetSenseEnabled(SightConfig->Implementation, true);
	}

	if (HearingConfig)
	{
		// Hearing configuration
		HearingConfig->SetMaxAge(1.f);
		HearingConfig->HearingRange = 1000;
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

		PerceptionComp->ConfigureSense(*HearingConfig);
		PerceptionComp->SetSenseEnabled(HearingConfig->Implementation, true);
	}

	PatrolPointKey = "PatrolPoint";
	PatrolPointLookTargetKey = "PatrolPointLookTarget";
	PatrolIndexKey = "PatrolIndex";
	TimerKey = "Timer";
	TargetActorKey = "TargetActor";
	TargetLastKnownLocationKey = "TargetLastKnownLocation";
	TargetLastKnownVelocityKey = "TargetLastKnownVelocity";
	ObjectiveTypeKey = "ObjectiveType";
	ObjectiveLocationKey = "ObjectiveLocation";
}

void AGuardAIController::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);

	// Bind the delegates
	PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AGuardAIController::RespondToPerceptionUpdated);
	OnActorSeen.AddDynamic(this, &AGuardAIController::RespondToActorSeen);
	OnActorSightLost.AddDynamic(this, &AGuardAIController::RespondToActorSightLost);
	OnActorHeard.AddDynamic(this, &AGuardAIController::RespondToActorHeard);
	OnObjectiveChanged.AddDynamic(this, &AGuardAIController::RespondToObjectiveChanged);

	// Create objective uobject
	CurrentObjective = NewObject<UObjective>();
	
	AGuard* Guard = Cast<AGuard>(Pawn);
	if (Guard)
	{
		// Start up blackboard and behavior tree
		if (Guard->BehaviorTree->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*(Guard->BehaviorTree->BlackboardAsset));
		}
		BehaviorTreeComp->StartTree(*Guard->BehaviorTree);

		// Get this guard's team
		SetGenericTeamId(Guard->GetGenericTeamId());

		// Guards should start out walking
		Guard->SetWalking();

		// Use collision avoidance
		Guard->SetCollisionAvoidanceEnabled(true);
	}
}

void AGuardAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CurrentObjective && bObjectiveInSight)
	{
		CurrentObjective->Refresh();

		// Write new values to blackboard
		WriteObjectiveToBlackboard();
	}

	TArray<AActor*> Actors;
	GetPerceptionComponent()->GetKnownPerceivedActors(nullptr, Actors);
	for (auto & Actor : Actors)
	{
		DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), Actor->GetActorLocation(), FColor::Red, false, 0, 0, 3.f);
	}
	if (CurrentObjective && CurrentObjective->TargetActor)
	{
		DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), CurrentObjective->TargetActor->GetActorLocation(), FColor::Orange, false, 0, 0, 5.f);
	}
}

#pragma region Perception

void AGuardAIController::RespondToPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	for (auto & Actor : UpdatedActors)
	{
		FActorPerceptionBlueprintInfo PerceptionInfo;
		GetPerceptionComponent()->GetActorsPerception(Actor, PerceptionInfo);
		for (auto & Stimulus : PerceptionInfo.LastSensedStimuli)
		{
			if (!Stimulus.IsValid() || Stimulus.IsExpired())
				continue;

			switch (Stimulus.Type.Index)
			{
			case 0: // sight
				if (Stimulus.IsActive())
				{
					OnActorSeen.Broadcast(Actor);
				}
				else
				{
					OnActorSightLost.Broadcast(Actor);
				}
				break;

			case 1: // hearing
				if (Stimulus.IsActive())
				{
					OnActorHeard.Broadcast(Actor, Stimulus.Tag);
				}
				break;

			default: // invalid
				break;
			}
		}

	}
}


void AGuardAIController::RespondToActorSeen(AActor* Actor)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::White, FString::Printf(TEXT("I see a %s"), *Actor->GetName()));

	// Maybe create an objective based on this actor
	if (Actor->ActorHasTag("Character.Escapee"))
	{
		if (ShouldSetNewObjective(EObjectiveType::Chase, Actor))
		{
			SetNewObjective(EObjectiveType::Chase, Actor);
		}
	}

	else if (Actor->ActorHasTag("Distraction"))
	{
		if (ShouldSetNewObjective(EObjectiveType::Distraction, Actor))
		{
			SetNewObjective(EObjectiveType::Distraction, Actor);
		}

	}

	if (Actor == CurrentObjective->TargetActor)
	{
		bObjectiveInSight = true;
		SetFocus(Actor, EAIFocusPriority::Gameplay);
	}
}

void AGuardAIController::RespondToActorSightLost(AActor* Actor)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::White, FString::Printf(TEXT("I can't see %s anymore..."), *Actor->GetName()));

	// if this actor is the current objective target, maybe downgrade to search
	if (CurrentObjective && Actor == CurrentObjective->TargetActor)
	{
		// lost sight of objective target!
		bObjectiveInSight = false;
		ClearFocus(EAIFocusPriority::Gameplay);

		// we could explicitly end a chase here
		//CurrentObjective->SetObjectiveType(EObjectiveType::Search);
	}

	// might want to check the other currently perceived actors for something better
}

void AGuardAIController::RespondToActorHeard(AActor* Actor, FName Tag)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::White, FString::Printf(TEXT("I hear a %s! Sounds like a %s!"), *Actor->GetName(), *Tag.ToString()));

	// Maybe create a search at this location
	if (Actor->ActorHasTag("Character.Escapee"))
	{
		if (ShouldSetNewObjective(EObjectiveType::Search, Actor))
		{
			SetNewObjective(EObjectiveType::Search, Actor);
		}
	}

}

#pragma endregion Perception

#pragma region Objective

bool AGuardAIController::ShouldSetNewObjective(EObjectiveType NewType, AActor* NewTargetActor)
{
	bool NewObjectiveIsCloser = GetObjectiveDistance() > FVector::Distance(GetPawn()->GetActorLocation(), NewTargetActor->GetActorLocation());

	if (!CurrentObjective ||
		CurrentObjective->Type == EObjectiveType::None ||
		CurrentObjective->Type == EObjectiveType::Search ||
		(CurrentObjective->Type == EObjectiveType::Chase && !bObjectiveInSight) ||
		(CurrentObjective->Type == EObjectiveType::Chase && NewObjectiveIsCloser))
	{
		return true;
	}
	return false;
}

void AGuardAIController::SetNewObjective(EObjectiveType NewType, AActor* NewtargetActor)
{
	if (!CurrentObjective)
	{
		CurrentObjective = NewObject<UObjective>();
	}

	// Broadcast the objective changed event
	OnObjectiveChanged.Broadcast(CurrentObjective->Type, NewType);

	CurrentObjective->ChangeObjective(NewType, NewtargetActor);

	WriteObjectiveToBlackboard();
}



float AGuardAIController::GetObjectiveDistance()
{
	if (CurrentObjective &&
		CurrentObjective->Type != EObjectiveType::None && 
		GetPawn())
	{
		return FVector::Distance(GetPawn()->GetActorLocation(), CurrentObjective->GetLastKnownLocation());
	}
	else
	{
		return INFINITY;
	}
}

void AGuardAIController::WriteObjectiveToBlackboard()
{
	if (CurrentObjective)
	{
		// Write objective location
		if (CurrentObjective->Type == EObjectiveType::Chase)
		{
			// if chasing, pursue the target
			FVector PursuitLocation = GetObjectivePursuitLocation();
			GetBlackboardComp()->SetValueAsVector(ObjectiveLocationKey, PursuitLocation);
		}
		else
		{
			// else just visit the immediate location
			GetBlackboardComp()->SetValueAsVector(ObjectiveLocationKey, CurrentObjective->GetLastKnownLocation());
		}

		// Write objective type
		GetBlackboardComp()->SetValueAsEnum(ObjectiveTypeKey, (uint8)CurrentObjective->Type);
	}
}

FVector AGuardAIController::GetObjectivePursuitLocation()
{
	if (CurrentObjective)
	{
		float TimeToReachTarget = GetEstimatedTimeToReach(CurrentObjective->GetLastKnownLocation(), INFINITY);

		FVector NaivePredictedLocation = CurrentObjective->GetExtrapolatedLocation(TimeToReachTarget);
		
		// Trace along the objective target's current trajectory 
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(
			Hit,
			CurrentObjective->GetLastKnownLocation(),
			NaivePredictedLocation,
			ECollisionChannel::ECC_Visibility))
		{
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 20, 3, FColor::Magenta, true, 0.1, 1, 4.f);
			return Hit.ImpactPoint;
		}
		else
		{
			DrawDebugSphere(GetWorld(), NaivePredictedLocation, 20, 3, FColor::Orange, true, 0.1, 1, 4.f);
			return NaivePredictedLocation;
		}
	}
	else
	{
		return FVector(0, 0, 0);
	}
}

float AGuardAIController::GetEstimatedTimeToReach(FVector Location, float MaxEstimate)
{
	if (GetPawn() && 
		GetPawn()->GetMovementComponent() &&
		GetPawn()->GetMovementComponent()->GetMaxSpeed() > 0)
	{
		float Distance = FVector::Distance(GetPawn()->GetActorLocation(), Location);
		float MaxSpeed = GetPawn()->GetMovementComponent()->GetMaxSpeed();

		return fminf(MaxEstimate, Distance / MaxSpeed);
	}
	else
	{
		return 0;
	}
}

void AGuardAIController::ClearObjective()
{ 
	SetNewObjective(EObjectiveType::None, nullptr);
}

void AGuardAIController::RespondToObjectiveChanged(EObjectiveType OldType, EObjectiveType NewType)
{
	UE_LOG(LogTemp, Warning, TEXT("New objective: was %d, now %d"), (uint8)OldType, (uint8)NewType);

	APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(GetPawn());
	if (PPCharacter)
	{
		if (NewType == EObjectiveType::None)
		{
			PPCharacter->SetWalking();
		}
		else
		{
			PPCharacter->SetRunning();
		}
	}

}


#pragma endregion Objective
