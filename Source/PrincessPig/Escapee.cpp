// Fill out your copyright notice in the Description page of Project Settings.

#include "Escapee.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


AEscapee::AEscapee()
{
	// Configure avoidance group
	FNavAvoidanceMask DefaultAvoidanceGroup;
	DefaultAvoidanceGroup.ClearAll();
	DefaultAvoidanceGroup.SetGroup(1);
	GetCharacterMovement()->SetAvoidanceGroupMask(DefaultAvoidanceGroup);

	// Smaller radius for escapees
	GetCharacterMovement()->AvoidanceConsiderationRadius = 100.f;


	RunSpeed = 500;

	SetGenericTeamId(255);
	Tags.AddUnique(FName("Character.Escapee"));
}


