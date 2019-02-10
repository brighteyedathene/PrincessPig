// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PrincessPigCharacter.h"
#include "UObject/Interface.h"
#include "Follow.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFollow : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PRINCESSPIG_API IFollow
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Follow")
	void SetLeader(APrincessPigCharacter* NewLeader);

};
