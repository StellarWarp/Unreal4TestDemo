// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CaterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class DEMOPROJECT_API ACaterGameMode : public AGameModeBase
{
	GENERATED_BODY()

	ACaterGameMode(const FObjectInitializer& ObjectInitializer);

	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

public:


};
