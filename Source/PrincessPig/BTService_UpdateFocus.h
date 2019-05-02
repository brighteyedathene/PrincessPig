// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateFocus.generated.h"

/**
 * 
 */
UCLASS()
class PRINCESSPIG_API UBTService_UpdateFocus : public UBTService
{
	GENERATED_BODY()

	UBTService_UpdateFocus();

	virtual void TickNode(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds) override;

public:
	UPROPERTY(EditAnywhere, BlueprintreadWrite, Category = "Focus")
	FBlackboardKeySelector TargetKey;
	
	UPROPERTY(EditAnywhere, BlueprintreadWrite, Category = "Focus")
	bool bRequireLineOfSight;

	/** Do not focus on actors beyond this range. */
	UPROPERTY(EditAnywhere, BlueprintreadWrite, Category = "Focus")
	float MaxFocusDistance;
};
