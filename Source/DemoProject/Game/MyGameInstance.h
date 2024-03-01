// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Containers/Ticker.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MyGameInstance.generated.h"


class UMyInGameMenuProxy;
class UMyMainMenuProxy;
class AMyGameSession;

namespace MyGameInstanceState
{
	extern const FName None;
	extern const FName PendingInvite;
	extern const FName WelcomeScreen;
	extern const FName MainMenu;
	extern const FName MessageMenu;
	extern const FName Playing;
}

UENUM()
enum class EOnlineMode : uint8
{
	Offline,
	LAN,
	Online
};

UCLASS()
class DEMOPROJECT_API UMyGameInstance : public UGameInstance
{
public:
	GENERATED_BODY()

	UMyGameInstance(const FObjectInitializer& ObjectInitializer);

public:
	bool Tick(float DeltaSeconds); 
	
	AMyGameSession* GetGameSession() const;
	
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;


	//state control api
private:
	UPROPERTY(config)
	FString WelcomeScreenMap;

	UPROPERTY(config)
	FString MainMenuMap;
	
	FName CurrentState;
	FName PendingState;

	/** URL to travel to after pending network operations */
	FString TravelURL;

	/** Current online mode of the game (offline, LAN, or online) */
	EOnlineMode OnlineMode;

private:
	const FName GetCurrentState() const;

	void BeginNewState(FName Name, FName OldState);
	void EndCurrentState(FName Name);
	
	void GotoState(FName NewState);
	void MaybeChangeState();

	/** Returns true if the game is in online mode */
	EOnlineMode GetOnlineMode() const { return OnlineMode; }

	void BeginPendingInviteState();
	void BeginMainMenuState();
	void BeginMessageMenuState();
	void BeginPlayingState();

	void EndPendingInviteState();
	void EndMainMenuState();
	void EndMessageMenuState();
	void EndPlayingState();


private:
	/** Controller to ignore for pairing changes. -1 to skip ignore. */
	int32 IgnorePairingChangeForControllerId;

	/** Last connection status that was passed into the HandleNetworkConnectionStatusChanged hander */
	EOnlineServerConnectionStatus::Type	CurrentConnectionStatus;
	
	/** Delegate for callbacks to Tick */
	FTickerDelegate TickDelegate;

	/** Handle to various registered delegates */
	FDelegateHandle TickDelegateHandle;
	FDelegateHandle TravelLocalSessionFailureDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnSearchSessionsCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnEndSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
	FDelegateHandle OnCreatePresenceSessionCompleteDelegateHandle;

	/** Local player login status when the system is suspended */
	TArray<ELoginStatus::Type> LocalPlayerOnlineStatus;

private:
	void HandleUserLoginChanged(int I, ELoginStatus::Type Arg, ELoginStatus::Type Arg1, const FUniqueNetId& UniqueNetId);
	void HandleControllerPairingChanged(int I, const FUniqueNetId& UniqueNetId, const FUniqueNetId& UniqueNetId1);
	void HandleAppWillDeactivate();
	void HandleAppSuspend();
	void HandleAppResume();
	void HandleSafeFrameChanged();
	void HandleControllerConnectionChange(bool bArg, int I, int Arg);
	void OnPreLoadMap(const FString& String);
	void OnPostLoadMap(UWorld* World);
	void OnPostDemoPlay();
	void HandleNetworkConnectionStatusChanged(const FString& String, EOnlineServerConnectionStatus::Type Arg, EOnlineServerConnectionStatus::Type Arg1);
	void HandleSessionFailure(const FUniqueNetId& UniqueNetId, ESessionFailure::Type Arg);

	/** Delegate for ending a session */
	FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;
	void OnEndSessionComplete(FName Name, bool bArg);


// UI
	
private:
	UPROPERTY()
	UMyMainMenuProxy* MainMenuUI;
	
	UPROPERTY()
	UMyInGameMenuProxy* InGameMenuUI;

public:
	//callable from blueprint to get MainMenuUI
	UFUNCTION(BlueprintCallable, Category = "UMyGameInstance")
	UMyMainMenuProxy* GetMainMenuUIProxy() const { return MainMenuUI; }

	UFUNCTION(BlueprintCallable, Category = "UMyGameInstance")
	UMyInGameMenuProxy* GetInGameMenuUIProxy() const { return InGameMenuUI; }

};
