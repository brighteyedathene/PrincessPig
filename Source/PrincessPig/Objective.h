// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Objective.generated.h"


UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Search = 1 UMETA(DisplayName = "Search"),
	Distraction = 2 UMETA(DisplayName = "Distraction"),
	Chase = 3 UMETA(DisplayName = "Chase")
};

/**
 * 
 */
UCLASS()
class PRINCESSPIG_API UObjective : public UObject
{
	GENERATED_BODY()

public:
	UObjective();

	UPROPERTY(Transient, BlueprintReadWrite, Category = "Objective")
	EObjectiveType Type;

	UPROPERTY(Transient, BlueprintReadWrite, Category = "Objective")
	AActor* TargetActor;
	
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Objective")
	FVector LastKnownLocation;

	UPROPERTY(Transient, BlueprintReadWrite, Category = "Objective")
	FVector LastKnownVelocity;

	UFUNCTION(BlueprintCallable, Category = "Objective")
	void ChangeObjective(EObjectiveType NewType, AActor* NewTargetActor);

	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetObjectiveType(EObjectiveType NewType);


	UFUNCTION(BlueprintCallable, Category = "Objective")
	FVector GetLastKnownLocation();

	UFUNCTION(BlueprintCallable, Category = "Objective")
	FVector GetExtrapolatedLocation(float SecondsSinceLastSeen);

	// Update last known location and velocity (only if there is a target actor)
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void Refresh();

	// Reset objective to initial state
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void Clear();
};
