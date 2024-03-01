// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

#include "MyInGameMenuProxy.h"
#include "MyMainMenuProxy.h"
#include "Containers/Ticker.h"
#include "Misc/CoreDelegates.h"
#include "OnlineSubsystemUtils.h"


namespace MyGameInstanceState
{
	const FName None = FName(TEXT("None"));
	const FName PendingInvite = FName(TEXT("PendingInvite"));
	const FName MainMenu = FName(TEXT("MainMenu"));
	const FName MessageMenu = FName(TEXT("MessageMenu"));
	const FName Playing = FName(TEXT("Playing"));
}

UMyGameInstance::UMyGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, OnlineMode(EOnlineMode::Online) // Default to online
{
	CurrentState = MyGameInstanceState::None;
}

bool UMyGameInstance::Tick(float DeltaSeconds)
{


	
	return true;
}

AMyGameSession* UMyGameInstance::GetGameSession() const
{
	return nullptr;
}

void UMyGameInstance::Init()
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
			i, FOnLoginStatusChangedDelegate::CreateUObject(this, &UMyGameInstance::HandleUserLoginChanged));
	}

	IdentityInterface->AddOnControllerPairingChangedDelegate_Handle(
		FOnControllerPairingChangedDelegate::CreateUObject(
			this, &UMyGameInstance::HandleControllerPairingChanged));

	FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this, &UMyGameInstance::HandleAppWillDeactivate);

	FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &UMyGameInstance::HandleAppSuspend);
	FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(this, &UMyGameInstance::HandleAppResume);

	FCoreDelegates::OnSafeFrameChangedEvent.AddUObject(this, &UMyGameInstance::HandleSafeFrameChanged);
	FCoreDelegates::OnControllerConnectionChange.AddUObject(this, &UMyGameInstance::HandleControllerConnectionChange);

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UMyGameInstance::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMyGameInstance::OnPostLoadMap);

	FCoreUObjectDelegates::PostDemoPlay.AddUObject(this, &UMyGameInstance::OnPostDemoPlay);


	OnlineSub->AddOnConnectionStatusChangedDelegate_Handle(
		FOnConnectionStatusChangedDelegate::CreateUObject(
			this, &UMyGameInstance::HandleNetworkConnectionStatusChanged));

	if (SessionInterface.IsValid())
	{
		SessionInterface->AddOnSessionFailureDelegate_Handle(
			FOnSessionFailureDelegate::CreateUObject(this, &UMyGameInstance::HandleSessionFailure));
	}

	OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(
		this, &UMyGameInstance::OnEndSessionComplete);

	// Register delegate for ticker callback
	TickDelegate = FTickerDelegate::CreateUObject(this, &UMyGameInstance::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);


	// UI

	MainMenuUI = NewObject<UMyMainMenuProxy>(this);
	MainMenuUI->Construct(this, GetFirstGamePlayer());
	InGameMenuUI = NewObject<UMyInGameMenuProxy>(this);
	InGameMenuUI->Construct(this, GetFirstGamePlayer());
	
}

void UMyGameInstance::Shutdown()
{
	Super::Shutdown();
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

void UMyGameInstance::StartGameInstance()
{
	Super::StartGameInstance();

	GotoState(MyGameInstanceState::MainMenu);
}

const FName UMyGameInstance::GetCurrentState() const
{
	return CurrentState;
}

void UMyGameInstance::BeginNewState(FName NewState, FName PrevState)
{
	// per-state custom starting code here

	if (NewState == MyGameInstanceState::PendingInvite)
	{
		BeginPendingInviteState();
	}
	else if (NewState == MyGameInstanceState::MainMenu)
	{
		BeginMainMenuState();
	}
	else if (NewState == MyGameInstanceState::MessageMenu)
	{
		BeginMessageMenuState();
	}
	else if (NewState == MyGameInstanceState::Playing)
	{
		BeginPlayingState();
	}

	CurrentState = NewState;
}

void UMyGameInstance::EndCurrentState(FName NextState)
{
	// per-state custom ending code here
	if (CurrentState == MyGameInstanceState::PendingInvite)
	{
		EndPendingInviteState();
	}
	else if (CurrentState == MyGameInstanceState::MainMenu)
	{
		EndMainMenuState();
	}
	else if (CurrentState == MyGameInstanceState::MessageMenu)
	{
		EndMessageMenuState();
	}
	else if (CurrentState == MyGameInstanceState::Playing)
	{
		EndPlayingState();
	}

	CurrentState = MyGameInstanceState::None;
}

void UMyGameInstance::GotoState(FName NewState)
{
	UE_LOG( LogOnline, Log, TEXT( "GotoState: NewState: %s" ), *NewState.ToString() );

	PendingState = NewState;
}

void UMyGameInstance::MaybeChangeState()
{
	if ( (PendingState != CurrentState) && (PendingState != MyGameInstanceState::None) )
	{
		FName const OldState = CurrentState;

		// end current state
		EndCurrentState(PendingState);

		// begin new state
		BeginNewState(PendingState, OldState);

		// clear pending change
		PendingState = MyGameInstanceState::None;
	}
}

void UMyGameInstance::BeginPendingInviteState()
{
}

void UMyGameInstance::BeginMainMenuState()
{
}

void UMyGameInstance::BeginMessageMenuState()
{
}

void UMyGameInstance::BeginPlayingState()
{
}

void UMyGameInstance::EndPendingInviteState()
{
}

void UMyGameInstance::EndMainMenuState()
{
}

void UMyGameInstance::EndMessageMenuState()
{
}

void UMyGameInstance::EndPlayingState()
{
}

void UMyGameInstance::HandleUserLoginChanged(int I, ELoginStatus::Type Arg, ELoginStatus::Type Arg1,
	const FUniqueNetId& UniqueNetId)
{
}

void UMyGameInstance::HandleControllerPairingChanged(int I, const FUniqueNetId& UniqueNetId,
	const FUniqueNetId& UniqueNetId1)
{
}

void UMyGameInstance::HandleAppWillDeactivate()
{
}

void UMyGameInstance::HandleAppSuspend()
{
}

void UMyGameInstance::HandleAppResume()
{
}

void UMyGameInstance::HandleSafeFrameChanged()
{
}

void UMyGameInstance::HandleControllerConnectionChange(bool bArg, int I, int Arg)
{
}

void UMyGameInstance::OnPreLoadMap(const FString& String)
{
}

void UMyGameInstance::OnPostLoadMap(UWorld* World)
{
}

void UMyGameInstance::OnPostDemoPlay()
{
}

void UMyGameInstance::HandleNetworkConnectionStatusChanged(const FString& String,
	EOnlineServerConnectionStatus::Type Arg, EOnlineServerConnectionStatus::Type Arg1)
{
}

void UMyGameInstance::HandleSessionFailure(const FUniqueNetId& UniqueNetId, ESessionFailure::Type Arg)
{
}

void UMyGameInstance::OnEndSessionComplete(FName Name, bool bArg)
{
}
