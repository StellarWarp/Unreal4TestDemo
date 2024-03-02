// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSessionSettings.h"
#include "OnlineSessionSettings.h"
#include "CaterGameSessionSettings.generated.h"

/**
 * 
 */
UCLASS()
class DEMOPROJECT_API UCaterGameSessionSettings : public UGameSessionSettings
{
	GENERATED_BODY()
};

class FCaterOnlineSessionSettings : public FOnlineSessionSettings
{
public:

	FCaterOnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4);
	virtual ~FCaterOnlineSessionSettings() {}
};


class FCaterOnlineSearchSettings : public FOnlineSessionSearch
{
public:
	FCaterOnlineSearchSettings(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FCaterOnlineSearchSettings() {}
};