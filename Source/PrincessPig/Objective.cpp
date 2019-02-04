// Fill out your copyright notice in the Description page of Project Settings.

#include "Objective.h"
#include "Engine/World.h"

#include "DrawDebugHelpers.h"

UObjective::UObjective()
{
	Type = EObjectiveType::None;
	TargetActor = nullptr;
	LastKnownLocation = FVector::ZeroVector;
	LastKnownVelocity = FVector::ZeroVector;
}

void UObjective::ChangeObjective(EObjectiveType NewType, AActor* NewTargetActor)
{
	Type = NewType;
	TargetActor = NewTargetActor;
	if (NewTargetActor)
	{
		LastKnownLocation = NewTargetActor->GetActorLocation();
		LastKnownVelocity = NewTargetActor->GetVelocity();
	}
	else
	{
		LastKnownLocation = FVector::ZeroVector;
		LastKnownVelocity = FVector::ZeroVector;
	}
}

void UObjective::SetObjectiveType(EObjectiveType NewType)
{
	Type = NewType;
}


FVector UObjective::GetLastKnownLocation()
{
	return LastKnownLocation;
}

FVector UObjective::GetExtrapolatedLocation(float SecondsSinceLastSeen)
{
	return LastKnownLocation + LastKnownVelocity * SecondsSinceLastSeen;
}

void UObjective::Refresh()
{
	if (TargetActor)
	{
		LastKnownLocation = TargetActor->GetActorLocation();
		LastKnownVelocity = TargetActor->GetVelocity();
	}
}

void UObjective::Clear()
{
	Type = EObjectiveType::None;
	TargetActor = nullptr;
	LastKnownLocation = FVector::ZeroVector;
	LastKnownVelocity = FVector::ZeroVector;
}
