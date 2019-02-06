// Fill out your copyright notice in the Description page of Project Settings.

#include "Escapee.h"
#include "GameFramework/CharacterMovementComponent.h"


AEscapee::AEscapee()
{
	// Configure avoidance group
	FNavAvoidanceMask DefaultAvoidanceGroup;
	DefaultAvoidanceGroup.ClearAll();
	DefaultAvoidanceGroup.SetGroup(1);
	GetCharacterMovement()->SetAvoidanceGroupMask(DefaultAvoidanceGroup);

	RunSpeed = 500;

	SetGenericTeamId(255);
	Tags.AddUnique(FName("Character.Escapee"));
}


