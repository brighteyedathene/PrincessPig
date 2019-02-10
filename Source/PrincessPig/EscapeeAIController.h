// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Follow.h"
#include "EscapeeAIController.generated.h"


class UBehaviorTreeComponent;
class UBlackboardComponent;

/**
 * 
 */
UCLASS()
class PRINCESSPIG_API AEscapeeAIController : public AAIController, public IFollow
{
	GENERATED_BODY()

public:
	AEscapeeAIController();

	virtual void Possess(APawn* Pawn) override;

	UBehaviorTreeComponent* BehaviorTreeComp;
	UBlackboardComponent* BlackboardComp;
	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; };

#pragma region Follow

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Follow")
	void SetLeader(APrincessPigCharacter* NewLeader);
	virtual void SetLeader_Implementation(APrincessPigCharacter* NewLeader) override;

#pragma endregion Follow


#pragma region BlackboardKeys
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName FollowTargetKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName LocationToGoKey;
#pragma endregion BlackboardKeys
};
