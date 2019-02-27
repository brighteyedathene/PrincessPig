// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"


class APrincessPigCharacter;

UCLASS()
class PRINCESSPIG_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void Use(APrincessPigCharacter* PPUser);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Item")
	void BPEvent_OnUsed(APrincessPigCharacter* PPUser);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Item")
	void BPEvent_OnPickedUp(APrincessPigCharacter* PPUser);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Item")
	void BPEvent_OnDropped();
};
