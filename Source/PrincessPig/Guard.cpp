// Fill out your copyright notice in the Description page of Project Settings.

#include "Guard.h"
#include "GameFramework/CharacterMovementComponent.h"


AGuard::AGuard()
{
	// Configure avoidance group
	FNavAvoidanceMask DefaultAvoidanceGroup;
	DefaultAvoidanceGroup.ClearAll();
	DefaultAvoidanceGroup.SetGroup(0);
	GetCharacterMovement()->SetAvoidanceGroupMask(DefaultAvoidanceGroup);

	// Don't try to avoid players or escapees
	FNavAvoidanceMask DefaultGroupsToIgnore;
	DefaultGroupsToIgnore.SetGroup(1);
	DefaultGroupsToIgnore.SetGroup(2);
	GetCharacterMovement()->SetGroupsToIgnoreMask(DefaultGroupsToIgnore);

	// Make the guards less agile by reducing acceleratin and friction
	GetCharacterMovement()->bRequestedMoveUseAcceleration = true;
	//GetCharacterMovement()->bUseAccelerationForPaths = true; must set this in blueprint!
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxAcceleration = 400.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 800.f;
	RunSpeed = 680.f;

	SetGenericTeamId(FGenericTeamId(1));
	Tags.AddUnique(FName("Character.Guard"));
}


