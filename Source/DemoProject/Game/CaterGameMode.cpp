// Fill out your copyright notice in the Description page of Project Settings.


#include "CaterGameMode.h"

#include "CaterGameInstance.h"
#include "CaterGameState.h"
#include "DemoProject/Player/CaterController.h"
#include "DemoProject/Player/CaterPlayerState.h"
#include "DemoProject/Player/CaterSpectatorPawn.h"
#include "DemoProject/UI/CaterHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "UObject/ConstructorHelpers.h"


ACaterGameMode::ACaterGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
		DefaultPawnClass = PlayerPawnBPClass.Class;
	
	// static ConstructorHelpers::FClassFinder<APawn> BotPawnOb(TEXT("/Game/Blueprints/Pawns/BotPawn"));
	// BotPawnClass = BotPawnOb.Class;
	
	HUDClass = ACaterHUD::StaticClass();
	PlayerControllerClass = ACaterController::StaticClass();
	PlayerStateClass = ACaterPlayerState::StaticClass();
	SpectatorClass = ACaterSpectatorPawn::StaticClass();
	GameStateClass = ACaterGameState::StaticClass();
	// ReplaySpectatorPlayerControllerClass = AShooterDemoSpectator::StaticClass();
	
	// MinRespawnDelay = 5.0f;
	//
	// bAllowBots = true;	
	// bNeedsBotCreation = true;
	bUseSeamlessTravel = FParse::Param(FCommandLine::Get(), TEXT("NoSeamlessTravel")) ? false : true;
}

void ACaterGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	// const int32 BotsCountOptionValue = UGameplayStatics::GetIntOption(Options, GetBotsCountOptionName(), 0);
	// SetAllowBots(BotsCountOptionValue > 0 ? true : false, BotsCountOptionValue);	
	// Super::InitGame(MapName, Options, ErrorMessage);
	//
	// const UGameInstance* GameInstance = GetGameInstance();
	// if (GameInstance && Cast<UCaterGameInstance>(GameInstance)->GetOnlineMode() != EOnlineMode::Offline)
	// {
	// 	bPauseable = false;
	// }

	
}
