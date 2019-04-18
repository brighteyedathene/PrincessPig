// Fill out your copyright notice in the Description page of Project Settings.

#include "PPMovementComponent.h"


void UPPMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	GEngine->AddOnScreenDebugMessage(123, 25.f, FColor::Silver, FString("Using PPMovementComponent!!!!"));
}

