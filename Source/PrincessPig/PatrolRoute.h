// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolRoute.generated.h"

class APatrolPoint;

UCLASS()
class PRINCESSPIG_API APatrolRoute : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APatrolRoute();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	TArray<APatrolPoint*> PatrolPoints;
	
};
