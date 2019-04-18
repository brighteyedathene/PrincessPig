// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PrincessPigCharacter.h"
#include "Guard.generated.h"

class UBehaviorTree;

/**
 * 
 */
UCLASS()
class PRINCESSPIG_API AGuard : public APrincessPigCharacter
{
	GENERATED_BODY()

public:
	AGuard(const class FObjectInitializer& ObjectInitializer);
};
