// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class DEMOPROJECT_API AMyGameMode : public AGameModeBase
{
	GENERATED_BODY()

	AMyGameMode(const FObjectInitializer& ObjectInitializer);
	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage);

public:


};
