// Fill out your copyright notice in the Description page of Project Settings.


#include "CaterGameInstance.h"

#include "CaterGameSession.h"
#include "CaterInGameMenuProxy.h"
#include "CaterMainMenuProxy.h"
#include "Containers/Ticker.h"
#include "Misc/CoreDelegates.h"
#include "OnlineSubsystemUtils.h"
#include "GameFramework/GameModeBase.h"


namespace CaterGameInstanceState
{
	const FName None = FName(TEXT("None"));
	const FName PendingInvite = FName(TEXT("PendingInvite"));
	const FName MainMenu = FName(TEXT("MainMenu"));
	const FName MessageMenu = FName(TEXT("MessageMenu"));
	const FName Playing = FName(TEXT("Playing"));
}

UCaterGameInstance::UCaterGameInstance(): OnlineMode(EOnlineMode::Online) // Default to online
{
	CurrentState = CaterGameInstanceState::None;
}

bool UCaterGameInstance::Tick(float DeltaSeconds)
{
	return true;
}

ACaterGameSession* UCaterGameInstance::GetGameSession() const
{
	UWorld* const World = GetWorld();
	if (World)
	{
		AGameModeBase* const Game = World->GetAuthGameMode();
		if (Game)
		{
			return Cast<ACaterGameSession>(Game->GameSession);
		}
	}

	return nullptr;
}

void UCaterGameInstance::Init()
{
	Super::Init();

	IgnorePairingChangeForControllerId = -1;
	CurrentConnectionStatus = EOnlineServerConnectionStatus::Connected;

	LocalPlayerOnlineStatus.InsertDefaulted(0, MAX_LOCAL_PLAYERS);

	// game requires the ability to ID users.
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);
	const IOnlineIdentityPtr IdentityInterface = OnlineSub->GetIdentityInterface();
	check(IdentityInterface.IsValid());

	const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	check(SessionInterface.IsValid());

	// bind any OSS delegates we needs to handle
	for (int i = 0; i < MAX_LOCAL_PLAYERS; ++i)
	{
		IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(
			i, FOnLoginStatusChangedDelegate::CreateUObject(this, &UCaterGameInstance::HandleUserLoginChanged));
	}

	IdentityInterface->AddOnControllerPairingChangedDelegate_Handle(
		FOnControllerPairingChangedDelegate::CreateUObject(
			this, &UCaterGameInstance::HandleControllerPairingChanged));

	FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this, &UCaterGameInstance::HandleAppWillDeactivate);

	FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &UCaterGameInstance::HandleAppSuspend);
	FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(this, &UCaterGameInstance::HandleAppResume);

	FCoreDelegates::OnSafeFrameChangedEvent.AddUObject(this, &UCaterGameInstance::HandleSafeFrameChanged);
	FCoreDelegates::OnControllerConnectionChange.
		AddUObject(this, &UCaterGameInstance::HandleControllerConnectionChange);

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UCaterGameInstance::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UCaterGameInstance::OnPostLoadMap);

	FCoreUObjectDelegates::PostDemoPlay.AddUObject(this, &UCaterGameInstance::OnPostDemoPlay);


	OnlineSub->AddOnConnectionStatusChangedDelegate_Handle(
		FOnConnectionStatusChangedDelegate::CreateUObject(
			this, &UCaterGameInstance::HandleNetworkConnectionStatusChanged));

	if (SessionInterface.IsValid())
	{
		SessionInterface->AddOnSessionFailureDelegate_Handle(
			FOnSessionFailureDelegate::CreateUObject(this, &UCaterGameInstance::HandleSessionFailure));
	}

	OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(
		this, &UCaterGameInstance::OnEndSessionComplete);

	// Register delegate for ticker callback
	TickDelegate = FTickerDelegate::CreateUObject(this, &UCaterGameInstance::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);


	// UI

	MainMenuUI = NewObject<UCaterMainMenuProxy>(this);
	MainMenuUI->Construct(this, GetFirstGamePlayer());
	InGameMenuUI = NewObject<UCaterInGameMenuProxy>(this);
	InGameMenuUI->Construct(this, GetFirstGamePlayer());
}

void UCaterGameInstance::Shutdown()
{
	Super::Shutdown();
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

void UCaterGameInstance::StartGameInstance()
{
	Super::StartGameInstance();

	GotoState(CaterGameInstanceState::MainMenu);
}

bool UCaterGameInstance::HostGame(ULocalPlayer* LocalPlayer, const FString& GameType, const FString& InTravelURL)
{
	if (GetOnlineMode() == EOnlineMode::Offline)
	{
		//
		// Offline game, just go straight to map
		//

		ShowLoadingScreen();
		GotoState(CaterGameInstanceState::Playing);

		// Travel to the specified match URL
		TravelURL = InTravelURL;
		GetWorld()->ServerTravel(TravelURL);
		return true;
	}

	ACaterGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		// add callback delegate for completion
		OnCreatePresenceSessionCompleteDelegateHandle = GameSession->OnCreatePresenceSessionComplete().AddUObject(
			this, &UCaterGameInstance::OnCreatePresenceSessionComplete);

		TravelURL = InTravelURL;
		bool const bIsLanMatch = InTravelURL.Contains(TEXT("?bIsLanMatch"));

		//determine the map name from the travelURL
		const FString& MapNameSubStr = "/Game/Maps/";
		const FString& ChoppedMapName = TravelURL.RightChop(MapNameSubStr.Len());
		const FString& MapName = ChoppedMapName.LeftChop(ChoppedMapName.Len() - ChoppedMapName.Find("?game"));

		if (GameSession->HostSession(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession,
		                             GameType, MapName, bIsLanMatch, true, AShooterGameSession::DEFAULT_NUM_PLAYERS))
		{
			// If any error occurred in the above, pending state would be set
			if ((PendingState == CurrentState) || (PendingState == ShooterGameInstanceState::None))
			{
				// Go ahead and go into loading state now
				// If we fail, the delegate will handle showing the proper messaging and move to the correct state
				ShowLoadingScreen();
				GotoState(ShooterGameInstanceState::Playing);
				return true;
			}
		}
	}

	return false;
}

const FName UCaterGameInstance::GetCurrentState() const
{
	return CurrentState;
}

void UCaterGameInstance::BeginNewState(FName NewState, FName PrevState)
{
	// per-state custom starting code here

	if (NewState == CaterGameInstanceState::PendingInvite)
	{
		BeginPendingInviteState();
	}
	else if (NewState == CaterGameInstanceState::MainMenu)
	{
		BeginMainMenuState();
	}
	else if (NewState == CaterGameInstanceState::MessageMenu)
	{
		BeginMessageMenuState();
	}
	else if (NewState == CaterGameInstanceState::Playing)
	{
		BeginPlayingState();
	}

	CurrentState = NewState;
}

void UCaterGameInstance::EndCurrentState(FName NextState)
{
	// per-state custom ending code here
	if (CurrentState == CaterGameInstanceState::PendingInvite)
	{
		EndPendingInviteState();
	}
	else if (CurrentState == CaterGameInstanceState::MainMenu)
	{
		EndMainMenuState();
	}
	else if (CurrentState == CaterGameInstanceState::MessageMenu)
	{
		EndMessageMenuState();
	}
	else if (CurrentState == CaterGameInstanceState::Playing)
	{
		EndPlayingState();
	}

	CurrentState = CaterGameInstanceState::None;
}

void UCaterGameInstance::GotoState(FName NewState)
{
	UE_LOG(LogOnline, Log, TEXT( "GotoState: NewState: %s" ), *NewState.ToString());

	PendingState = NewState;
}

void UCaterGameInstance::MaybeChangeState()
{
	if ((PendingState != CurrentState) && (PendingState != CaterGameInstanceState::None))
	{
		FName const OldState = CurrentState;

		// end current state
		EndCurrentState(PendingState);

		// begin new state
		BeginNewState(PendingState, OldState);

		// clear pending change
		PendingState = CaterGameInstanceState::None;
	}
}

void UCaterGameInstance::BeginPendingInviteState()
{
}

void UCaterGameInstance::BeginMainMenuState()
{
}

void UCaterGameInstance::BeginMessageMenuState()
{
}

void UCaterGameInstance::BeginPlayingState()
{
}

void UCaterGameInstance::EndPendingInviteState()
{
}

void UCaterGameInstance::EndMainMenuState()
{
}

void UCaterGameInstance::EndMessageMenuState()
{
}

void UCaterGameInstance::EndPlayingState()
{
}

void UCaterGameInstance::HandleUserLoginChanged(int I, ELoginStatus::Type Arg, ELoginStatus::Type Arg1,
                                                const FUniqueNetId& UniqueNetId)
{
}

void UCaterGameInstance::HandleControllerPairingChanged(int I, const FUniqueNetId& UniqueNetId,
                                                        const FUniqueNetId& UniqueNetId1)
{
}

void UCaterGameInstance::HandleAppWillDeactivate()
{
}

void UCaterGameInstance::HandleAppSuspend()
{
}

void UCaterGameInstance::HandleAppResume()
{
}

void UCaterGameInstance::HandleSafeFrameChanged()
{
}

void UCaterGameInstance::HandleControllerConnectionChange(bool bArg, int I, int Arg)
{
}

void UCaterGameInstance::OnPreLoadMap(const FString& String)
{
}

void UCaterGameInstance::OnPostLoadMap(UWorld* World)
{
}

void UCaterGameInstance::OnPostDemoPlay()
{
}

void UCaterGameInstance::HandleNetworkConnectionStatusChanged(const FString& String,
                                                              EOnlineServerConnectionStatus::Type Arg,
                                                              EOnlineServerConnectionStatus::Type Arg1)
{
}

void UCaterGameInstance::HandleSessionFailure(const FUniqueNetId& UniqueNetId, ESessionFailure::Type Arg)
{
}

void UCaterGameInstance::OnEndSessionComplete(FName Name, bool bArg)
{
}
