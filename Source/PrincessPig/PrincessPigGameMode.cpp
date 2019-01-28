// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PrincessPigGameMode.h"
#include "PrincessPigPlayerController.h"
#include "PrincessPigCharacter.h"
#include "UObject/ConstructorHelpers.h"

APrincessPigGameMode::APrincessPigGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = APrincessPigPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Guards/BP_Guard"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}