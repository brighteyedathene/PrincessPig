// Fill out your copyright notice in the Description page of Project Settings.

#include "GuardAIController.h"
#include "PrincessPigCharacter.h"
#include "Guard.h"
#include "PatrolPoint.h"
#include "PatrolRoute.h"
#include "Objective.h"
#include "InteractionComponent.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GenericTeamAgentInterface.h"

#include "NavigationSystem.h"

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
		SightConfig->SightRadius = 1200;
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
		HearingConfig->SetMaxAge(0.01f);
		HearingConfig->HearingRange = 1200;
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
	TimestampKey = "Timestamp";
	TargetActorKey = "TargetActor";
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
	
	APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(Pawn);
	if (PPCharacter)
	{
		// Start up blackboard and behavior tree
		if (PPCharacter->BehaviorTree->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*(PPCharacter->BehaviorTree->BlackboardAsset));
		}
		BehaviorTreeComp->StartTree(*PPCharacter->BehaviorTree);

		// Get this guard's team
		SetGenericTeamId(PPCharacter->GetGenericTeamId());

		// Use collision avoidance
		PPCharacter->SetCollisionAvoidanceEnabled(true);

		// Configure avoidance group (0 for guards)
		FNavAvoidanceMask DefaultAvoidanceGroup;
		DefaultAvoidanceGroup.ClearAll();
		DefaultAvoidanceGroup.SetGroup(0);
		PPCharacter->GetCharacterMovement()->SetAvoidanceGroupMask(DefaultAvoidanceGroup);
		
		// Don't try to avoid players or escapees (groups 1 and 2)
		FNavAvoidanceMask DefaultGroupsToIgnore;
		DefaultGroupsToIgnore.SetGroup(1);
		DefaultGroupsToIgnore.SetGroup(2);
		PPCharacter->GetCharacterMovement()->SetGroupsToIgnoreMask(DefaultGroupsToIgnore);
	}
}

void AGuardAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// Some objectives (like Chase) require constant updates
	if (CurrentObjective)
	{
		// Refresh the pursuit location
		if (bObjectiveInSight)
		{
			CurrentObjective->Refresh();
			PursuitLocation = GetObjectivePursuitLocation();
		}

		// Check the status of the objective target (might be dead, disappeared etc)
		if(CurrentObjective->RequiresInteraction())
		{
			if (CurrentObjective->TargetActor->IsPendingKillPending())
			{
				// Actor not there anymore? Demote to search and check line of sight for something else to do
				DowngradeObjectiveToSearch();
			}

			// Is the objective a character?
			APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(CurrentObjective->TargetActor);
			if (PPCharacter)
			{
				// If the target is dead, there's nothing more to do (FOR NOW)
				if (PPCharacter->Replicated_IsDead)
				{
					DowngradeObjectiveToSearch();
				}

			}
		}
	}

	DebugShowObjective();

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
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::White, FString::Printf(TEXT("I see a %s"), *Actor->GetName()));

	// Maybe create an objective based on this actor
	if (Actor->ActorHasTag("Escapee"))
	{
		APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(Actor);
		if (PPCharacter && !PPCharacter->Replicated_IsDead)
		{
			if (ShouldSetNewObjective(EObjectiveType::Chase, Actor))
			{
				SetNewObjective(EObjectiveType::Chase, Actor);
			}
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
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::White, FString::Printf(TEXT("I can't see %s anymore..."), *Actor->GetName()));

	// if this actor is the current objective target, maybe downgrade to search
	if (CurrentObjective && Actor == CurrentObjective->TargetActor)
	{
		// lost sight of objective target!
		bObjectiveInSight = false;
		ClearFocus(EAIFocusPriority::Gameplay);

		// might want to check the other currently perceived actors for something better
		CheckCurrentLineOfSight();
	}

}

void AGuardAIController::RespondToActorHeard(AActor* Actor, FName Tag)
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::White, FString::Printf(TEXT("I hear a %s! Sounds like a %s!"), *Actor->GetName(), *Tag.ToString()));

	// Maybe create a search at this location
	if (Actor->ActorHasTag("Escapee") ||
		Actor->ActorHasTag("Guard"))
	{
		if (ShouldSetNewObjective(EObjectiveType::Search, Actor))
		{
			SetNewObjective(EObjectiveType::Search, Actor);
		}
	}

}


void AGuardAIController::CheckCurrentLineOfSight()
{
	TArray<AActor*> PerceivedActors;
	GetPerceptionComponent()->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);
	for (auto & Actor : PerceivedActors)
	{
		FActorPerceptionBlueprintInfo PerceptionInfo;
		GetPerceptionComponent()->GetActorsPerception(Actor, PerceptionInfo);
		for (auto & Stimulus : PerceptionInfo.LastSensedStimuli)
		{
			if (!Stimulus.IsValid() ||
				!Stimulus.IsActive() ||
				Stimulus.IsExpired() )
				continue;

			if (Stimulus.Type.Index == 0) // 0 refers to sight
			{ 
				OnActorSeen.Broadcast(Actor);
			}
		}
	}
}

#pragma endregion Perception

#pragma region Objective

bool AGuardAIController::ShouldSetNewObjective(EObjectiveType NewType, AActor* NewTargetActor)
{
	if (NewTargetActor && !NewTargetActor->IsPendingKillPending())
	{
		bool NewObjectiveIsCloser = GetObjectiveDistance() > FVector::Distance(GetPawn()->GetActorLocation(), NewTargetActor->GetActorLocation());

		if (!CurrentObjective ||
			!CurrentObjective->TargetActor ||
			CurrentObjective->Type == EObjectiveType::None ||
			(CurrentObjective->Type == EObjectiveType::Search && CurrentObjective->TargetActor->ActorHasTag("Guard")) ||
			(CurrentObjective->Type == EObjectiveType::Chase && NewObjectiveIsCloser) ||
			//(CurrentObjective->Type == EObjectiveType::Distraction && NewObjectiveIsCloser) ||
			(CurrentObjective->Type == EObjectiveType::Distraction) ||
			(!(bObjectiveInSight || IsObjectiveInteractionAvailable())))
		{
			return true;
		}
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
			// if chasing, use the pursuit location
			GetBlackboardComp()->SetValueAsVector(ObjectiveLocationKey, PursuitLocation);
		}
		else
		{
			// else just visit the immediate location
			GetBlackboardComp()->SetValueAsVector(ObjectiveLocationKey, CurrentObjective->GetLastKnownLocation());
		}

		// Write objective type
		GetBlackboardComp()->SetValueAsEnum(ObjectiveTypeKey, (uint8)CurrentObjective->Type);

		// Write objective target actor
		GetBlackboardComp()->SetValueAsObject(TargetActorKey, CurrentObjective->TargetActor);

	}
}

FVector AGuardAIController::GetObjectivePursuitLocation()
{
	if (!CurrentObjective)
	{
		return FVector(0, 0, 0);
	}

	float TimeToReachTarget = GetEstimatedTimeToReach(CurrentObjective->GetLastKnownLocation(), INFINITY);

	FVector PursuitLocation = CurrentObjective->GetExtrapolatedLocation(TimeToReachTarget);
		
	// Trace along the objective target's current trajectory 
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		CurrentObjective->GetLastKnownLocation(),
		PursuitLocation,
		ECollisionChannel::ECC_Visibility))
	{
		// we have a hit! place the point a little bit back though
		float SafetyBufferDistance = 20.f;
		PursuitLocation = Hit.ImpactPoint + Hit.ImpactNormal * SafetyBufferDistance;
	}

	// Project to navigation
	FNavLocation NavigableLocation;
	if (UNavigationSystemV1::GetNavigationSystem(this)->ProjectPointToNavigation(PursuitLocation, NavigableLocation))
	{
		PursuitLocation = NavigableLocation.Location;
	}
	else
	{
		PursuitLocation = CurrentObjective->GetLastKnownLocation();

	}

	return PursuitLocation;
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


void AGuardAIController::DowngradeObjectiveToSearch()
{
	if (CurrentObjective && CurrentObjective->Type != EObjectiveType::None)
	{
		CurrentObjective->SetObjectiveType(EObjectiveType::Search);
		
		CheckCurrentLineOfSight();
	}
}



void AGuardAIController::RespondToObjectiveChanged(EObjectiveType OldType, EObjectiveType NewType) {}


#pragma endregion Objective



#pragma region Interaction

bool AGuardAIController::IsObjectiveInteractionAvailable()
{
	APrincessPigCharacter* PPCharacter = Cast<APrincessPigCharacter>(GetPawn());
	if (PPCharacter)
	{
		if (CurrentObjective && CurrentObjective->TargetActor)
		{
			return PPCharacter->AvailableInteractions.Contains(CurrentObjective->TargetActor);
		}
	}
	return false;
	
}

#pragma endregion Interaction


void AGuardAIController::DebugShowObjective()
{
	if (CurrentObjective && CurrentObjective->TargetActor)
	{
		FString ObjectiveTypeString;
		FColor Color;
		switch (CurrentObjective->Type)
		{
		case EObjectiveType::Search:
			ObjectiveTypeString = FString("  Search  "); 
			Color = FColor::Yellow;
			break;
		case EObjectiveType::Chase:
			ObjectiveTypeString = FString("  Chase  "); 
			Color = FColor::Orange;
			break;
		case EObjectiveType::Distraction:
			ObjectiveTypeString = FString("  Distraction  "); 
			Color = FColor::Cyan;
			break;
		default:
			ObjectiveTypeString = FString("  None  "); 
			Color = FColor::White;
			break;
		}

		GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID(), 0.2, Color, GetName() + ObjectiveTypeString + CurrentObjective->TargetActor->GetName());
		FVector Destinaction = BlackboardComp->GetValueAsVector(ObjectiveLocationKey);
		DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), Destinaction, Color, false, 0, 0, 8.f);
		DrawDebugSphere(GetWorld(), CurrentObjective->TargetActor->GetActorLocation() + FVector(0, 0, 1) * 100, 25.f, 3, Color, false, 0, 0, 5.f);
	}
}

