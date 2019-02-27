// Fill out your copyright notice in the Description page of Project Settings.

#include "Item.h"
#include "PrincessPigCharacter.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	SetReplicates(true);
}

void AItem::Use(APrincessPigCharacter* PPUser)
{
	BPEvent_OnUsed(PPUser);
}
