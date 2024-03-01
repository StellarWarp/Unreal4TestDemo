// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameMode.h"

#include "MyGameInstance.h"
#include "MyGameState.h"
#include "DemoProject/Player/MyController.h"
#include "DemoProject/Player/MyPlayerState.h"
#include "DemoProject/Player/MySpectatorPawn.h"
#include "DemoProject/UI/MyHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "UObject/ConstructorHelpers.h"


AMyGameMode::AMyGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
		DefaultPawnClass = PlayerPawnBPClass.Class;
	
	// static ConstructorHelpers::FClassFinder<APawn> BotPawnOb(TEXT("/Game/Blueprints/Pawns/BotPawn"));
	// BotPawnClass = BotPawnOb.Class;
	
	HUDClass = AMyHUD::StaticClass();
	PlayerControllerClass = AMyController::StaticClass();
	PlayerStateClass = AMyPlayerState::StaticClass();
	SpectatorClass = AMySpectatorPawn::StaticClass();
	GameStateClass = AMyGameState::StaticClass();
	// ReplaySpectatorPlayerControllerClass = AShooterDemoSpectator::StaticClass();
	
	// MinRespawnDelay = 5.0f;
	//
	// bAllowBots = true;	
	// bNeedsBotCreation = true;
	bUseSeamlessTravel = FParse::Param(FCommandLine::Get(), TEXT("NoSeamlessTravel")) ? false : true;
}

void AMyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	// const int32 BotsCountOptionValue = UGameplayStatics::GetIntOption(Options, GetBotsCountOptionName(), 0);
	// SetAllowBots(BotsCountOptionValue > 0 ? true : false, BotsCountOptionValue);	
	// Super::InitGame(MapName, Options, ErrorMessage);
	//
	// const UGameInstance* GameInstance = GetGameInstance();
	// if (GameInstance && Cast<UMyGameInstance>(GameInstance)->GetOnlineMode() != EOnlineMode::Offline)
	// {
	// 	bPauseable = false;
	// }

	
}
