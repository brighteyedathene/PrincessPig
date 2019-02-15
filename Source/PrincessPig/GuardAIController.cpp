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
		HearingConfig->SetMaxAge(0.01f);
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
	TimestampKey = "Timestamp";
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

		// Bind response functions to this pawn's interaction component overlap events
		if (Guard->InteractionComponent)
		{
			Guard->InteractionComponent->OnComponentBeginOverlap.AddDynamic(this, &AGuardAIController::RespondToInteractionBeginOverlap);
			Guard->InteractionComponent->OnComponentEndOverlap.AddDynamic(this, &AGuardAIController::RespondToInteractionEndOverlap);
		}
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

		if(CurrentObjective->RequiresInteraction())
		{
			if (CurrentObjective->TargetActor->IsPendingKillPending())
			{
				// Actor not there anymore? Demote to search and check line of sight for something else to do
				CurrentObjective->SetObjectiveType(EObjectiveType::Search);
				CheckCurrentLineOfSight();
			}
		}
	}

	DebugShowObjective();

	for (auto & InteractionActor : AvailableInteractions)
	{
		GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID() + InteractionActor->GetUniqueID(), 0.2, FColor::Cyan, InteractionActor->GetName());
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

		// might want to check the other currently perceived actors for something better
		CheckCurrentLineOfSight();
	}

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


void AGuardAIController::CheckCurrentLineOfSight()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString("CHECKING MY PERCEPTS"));


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
	if (NewTargetActor)
	{
		bool NewObjectiveIsCloser = GetObjectiveDistance() > FVector::Distance(GetPawn()->GetActorLocation(), NewTargetActor->GetActorLocation());

		if (!CurrentObjective ||
			!CurrentObjective->TargetActor ||
			CurrentObjective->Type == EObjectiveType::None ||
			CurrentObjective->Type == EObjectiveType::Search ||
			(CurrentObjective->Type == EObjectiveType::Chase && NewObjectiveIsCloser) ||
			(CurrentObjective->Type == EObjectiveType::Distraction && NewObjectiveIsCloser) ||
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
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint + Hit.ImpactNormal * 20.f, 20, 3, FColor::Magenta, true, 0.1, 1, 4.f);
			// we have a hit! place the point a little bit back though
			float SafetyBufferDistance = 20.f;
			return Hit.ImpactPoint + Hit.ImpactNormal * SafetyBufferDistance;
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



#pragma region Interaction

bool AGuardAIController::IsObjectiveInteractionAvailable()
{
	if (CurrentObjective && CurrentObjective->TargetActor)
	{
		return AvailableInteractions.Contains(CurrentObjective->TargetActor);
	}
	else
	{
		return false;
	}
}


void AGuardAIController::RespondToInteractionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AvailableInteractions.Add(OtherActor);

	if (CurrentObjective && OtherActor == CurrentObjective->TargetActor)
	{
		//GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID() + OtherComp->GetUniqueID(), 15.f, FColor::Green, FString::Printf(
		//	TEXT("Begin %s 's %s overlapped with %s 's %s"),
		//	*GetName(),
		//	*OverlappedComp->GetName(),
		//	*OtherActor->GetName(),
		//	*OtherComp->GetName()));

		AGuard* Guard = Cast<AGuard>(GetPawn());
		if (Guard)
		{
			// Call the BPEvent
			Guard->BPEvent_ObjectiveReached();
		}

	}
}


void AGuardAIController::RespondToInteractionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AvailableInteractions.Remove(OtherActor);
	//GEngine->AddOnScreenDebugMessage((uint64)GetUniqueID() + OtherComp->GetUniqueID(), 15.f, FColor::Orange, FString::Printf(
	//	TEXT("End %s 's %s overlapped with %s 's %s"),
	//	*GetName(),
	//	*OverlappedComp->GetName(),
	//	*OtherActor->GetName(),
	//	*OtherComp->GetName()));


	if (CurrentObjective && OtherActor == CurrentObjective->TargetActor)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString("My objective got away!!!"));
		//CheckCurrentPercepts();
	}
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

