// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PrincessPigCharacter.h"
#include "Escapee.generated.h"

class UBehaviorTree;

/**
 * 
 */
UCLASS()
class PRINCESSPIG_API AEscapee : public APrincessPigCharacter
{
	GENERATED_BODY()
	
public:
	AEscapee();

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTree;
	
	
};
