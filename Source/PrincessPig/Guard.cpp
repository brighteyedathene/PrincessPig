// Fill out your copyright notice in the Description page of Project Settings.

#include "Guard.h"
#include "GameFramework/CharacterMovementComponent.h"


AGuard::AGuard()
{
	GetCharacterMovement()->bUseRVOAvoidance = true; 
	GetCharacterMovement()->AvoidanceConsiderationRadius = 200.f;
	
	GetCharacterMovement()->bRequestedMoveUseAcceleration = true;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxAcceleration = 500.f;
	GetCharacterMovement()->MaxWalkSpeed = 700.f;

	SetGenericTeamId(FGenericTeamId(1));
	Tags.AddUnique(FName("Character.Guard"));
}


