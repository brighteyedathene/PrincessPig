// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"

#include "PrincessPigCharacter.h"

#include "BTService_SetMovementMode.generated.h"

/**
 * 
 */
UCLASS()
class PRINCESSPIG_API UBTService_SetMovementMode : public UBTService
{
	GENERATED_BODY()
	
	virtual void TickNode(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds) override;

public:
	UPROPERTY(EditAnywhere, BlueprintreadWrite, Category = "MovementMode")
		EPPMovementMode MovementMode;
};
