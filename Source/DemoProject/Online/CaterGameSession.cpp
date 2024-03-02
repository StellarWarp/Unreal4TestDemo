﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "CaterGameSession.h"

#include "CaterGameSessionSettings.h"
#include "CaterPlayerController.h"
#include "OnlineSubsystemSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"


namespace
{
	const FString CustomMatchKeyword("Custom");
}


ACaterGameSession::ACaterGameSession()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(
			this, &ACaterGameSession::OnCreateSessionComplete);
		OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(
			this, &ACaterGameSession::OnDestroySessionComplete);

		OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(
			this, &ACaterGameSession::OnFindSessionsComplete);
		OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(
			this, &ACaterGameSession::OnJoinSessionComplete);

		OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(
			this, &ACaterGameSession::OnStartOnlineGameComplete);
	}
}


/**
 * Delegate fired when a session start request has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
void ACaterGameSession::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		}
	}

	if (bWasSuccessful)
	{
		// tell non-local players to start online game
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ACaterPlayerController* PC = Cast<ACaterPlayerController>(*It);
			if (PC && !PC->IsLocalPlayerController())
			{
				PC->ClientStartOnlineGame();
			}
		}
	}
}

/** Handle starting the match */
void ACaterGameSession::HandleMatchHasStarted()
{
	// start online game locally and wait for completion
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && (Sessions->GetNamedSession(NAME_GameSession) != nullptr))
		{
			UE_LOG(LogOnlineGame, Log, TEXT("Starting session %s on server"), *FName(NAME_GameSession).ToString());
			OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(
				OnStartSessionCompleteDelegate);
			Sessions->StartSession(NAME_GameSession);
		}
	}
}

/** 
 * Ends a game session 
 *
 */
void ACaterGameSession::HandleMatchHasEnded()
{
	// end online game locally 
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && (Sessions->GetNamedSession(NAME_GameSession) != nullptr))
		{
			// tell the clients to end
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				ACaterPlayerController* PC = Cast<ACaterPlayerController>(*It);
				if (PC && !PC->IsLocalPlayerController())
				{
					PC->ClientEndOnlineGame();
				}
			}

			// server is handled here
			UE_LOG(LogOnlineGame, Log, TEXT("Ending session %s on server"), *FName(NAME_GameSession).ToString());
			Sessions->EndSession(NAME_GameSession);
		}
	}
}

bool ACaterGameSession::IsBusy() const
{
	if (HostSettings.IsValid() || SearchSettings.IsValid())
	{
		return true;
	}

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			EOnlineSessionState::Type GameSessionState = Sessions->GetSessionState(NAME_GameSession);
			EOnlineSessionState::Type PartySessionState = Sessions->GetSessionState(NAME_PartySession);
			if (GameSessionState != EOnlineSessionState::NoSession || PartySessionState !=
				EOnlineSessionState::NoSession)
			{
				return true;
			}
		}
	}

	return false;
}

EOnlineAsyncTaskState::Type ACaterGameSession::GetSearchResultStatus(int32& SearchResultIdx, int32& NumSearchResults)
{
	SearchResultIdx = 0;
	NumSearchResults = 0;

	if (SearchSettings.IsValid())
	{
		if (SearchSettings->SearchState == EOnlineAsyncTaskState::Done)
		{
			SearchResultIdx = CurrentSessionParams.BestSessionIdx;
			NumSearchResults = SearchSettings->SearchResults.Num();
		}
		return SearchSettings->SearchState;
	}

	return EOnlineAsyncTaskState::NotStarted;
}

/**
 * Get the search results.
 *
 * @return Search results
 */
const TArray<FOnlineSessionSearchResult>& ACaterGameSession::GetSearchResults() const
{
	return SearchSettings->SearchResults;
};


/**
 * Delegate fired when a session create request has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
void ACaterGameSession::OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnlineGame, Verbose, TEXT("OnCreateSessionComplete %s bSuccess: %d"), *InSessionName.ToString(),
	       bWasSuccessful);

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	}

	OnCreatePresenceSessionComplete().Broadcast(InSessionName, bWasSuccessful);
}

/**
 * Delegate fired when a destroying an online session has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
void ACaterGameSession::OnDestroySessionComplete(FName InSessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnlineGame, Verbose, TEXT("OnDestroySessionComplete %s bSuccess: %d"), *InSessionName.ToString(),
	       bWasSuccessful);

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		HostSettings = NULL;
	}
}

bool ACaterGameSession::HostSession(TSharedPtr<const FUniqueNetId> UserId, FName InSessionName, const FString& GameType,
                                    const FString& MapName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers)
{
	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		CurrentSessionParams.SessionName = InSessionName;
		CurrentSessionParams.bIsLAN = bIsLAN;
		CurrentSessionParams.bIsPresence = bIsPresence;
		CurrentSessionParams.UserId = UserId;
		MaxPlayers = MaxNumPlayers;

		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && CurrentSessionParams.UserId.IsValid())
		{
			HostSettings = MakeShareable(new FCaterOnlineSessionSettings(bIsLAN, bIsPresence, MaxPlayers));
			HostSettings->Set(SETTING_GAMEMODE, GameType, EOnlineDataAdvertisementType::ViaOnlineService);
			HostSettings->Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineService);
			HostSettings->Set(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"),
			                  EOnlineDataAdvertisementType::DontAdvertise);
			HostSettings->Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
			HostSettings->Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"),
			                  EOnlineDataAdvertisementType::DontAdvertise);

#if !PLATFORM_SWITCH
			// On Switch, we don't have room for this in the session data (and it's not used anyway when searching), so there's no need to add it.
			// Can be readded if the buffer size increases.
			HostSettings->Set(SEARCH_KEYWORDS, CustomMatchKeyword, EOnlineDataAdvertisementType::ViaOnlineService);
#endif

			OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
				OnCreateSessionCompleteDelegate);
			return Sessions->CreateSession(*CurrentSessionParams.UserId, CurrentSessionParams.SessionName,
			                               *HostSettings);
		}
		else
		{
			OnCreateSessionComplete(InSessionName, false);
		}
	}
#if !UE_BUILD_SHIPPING
	else
	{
		// Hack workflow in development
		OnCreatePresenceSessionComplete().Broadcast(NAME_GameSession, true);
		return true;
	}
#endif

	return false;
}

bool ACaterGameSession::HostSession(const TSharedPtr<const FUniqueNetId> UserId, const FName InSessionName,
                                    const FOnlineSessionSettings& SessionSettings)
{
	bool bResult = false;

	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		CurrentSessionParams.SessionName = InSessionName;
		CurrentSessionParams.bIsLAN = SessionSettings.bIsLANMatch;
		CurrentSessionParams.bIsPresence = SessionSettings.bUsesPresence;
		CurrentSessionParams.UserId = UserId;
		MaxPlayers = SessionSettings.NumPrivateConnections + SessionSettings.NumPublicConnections;

		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && CurrentSessionParams.UserId.IsValid())
		{
			OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
				OnCreateSessionCompleteDelegate);
			bResult = Sessions->CreateSession(*UserId, InSessionName, SessionSettings);
		}
		else
		{
			OnCreateSessionComplete(InSessionName, false);
		}
	}

	return bResult;
}

void ACaterGameSession::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogOnlineGame, Verbose, TEXT("OnFindSessionsComplete bSuccess: %d"), bWasSuccessful);

	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

			UE_LOG(LogOnlineGame, Verbose, TEXT("Num Search Results: %d"), SearchSettings->SearchResults.Num());
			for (int32 SearchIdx = 0; SearchIdx < SearchSettings->SearchResults.Num(); SearchIdx++)
			{
				const FOnlineSessionSearchResult& SearchResult = SearchSettings->SearchResults[SearchIdx];
				DumpSession(&SearchResult.Session);
			}

			OnFindSessionsComplete().Broadcast(bWasSuccessful);
		}
	}
}

void ACaterGameSession::ResetBestSessionVars()
{
	CurrentSessionParams.BestSessionIdx = -1;
}

void ACaterGameSession::ChooseBestSession()
{
	// Start searching from where we left off
	for (int32 SessionIndex = CurrentSessionParams.BestSessionIdx + 1; SessionIndex < SearchSettings->SearchResults.
	     Num(); SessionIndex++)
	{
		// Found the match that we want
		CurrentSessionParams.BestSessionIdx = SessionIndex;
		return;
	}

	CurrentSessionParams.BestSessionIdx = -1;
}

void ACaterGameSession::StartMatchmaking()
{
	ResetBestSessionVars();
	ContinueMatchmaking();
}

void ACaterGameSession::ContinueMatchmaking()
{
	ChooseBestSession();
	if (CurrentSessionParams.BestSessionIdx >= 0 && CurrentSessionParams.BestSessionIdx < SearchSettings->SearchResults.
		Num())
	{
		IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid() && CurrentSessionParams.UserId.IsValid())
			{
				OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
					OnJoinSessionCompleteDelegate);
				Sessions->JoinSession(*CurrentSessionParams.UserId, CurrentSessionParams.SessionName,
				                      SearchSettings->SearchResults[CurrentSessionParams.BestSessionIdx]);
			}
		}
	}
	else
	{
		OnNoMatchesAvailable();
	}
}

void ACaterGameSession::OnNoMatchesAvailable()
{
	UE_LOG(LogOnlineGame, Verbose, TEXT("Matchmaking complete, no sessions available."));
	SearchSettings = NULL;
}

void ACaterGameSession::FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName InSessionName, bool bIsLAN,
                                     bool bIsPresence)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		CurrentSessionParams.SessionName = InSessionName;
		CurrentSessionParams.bIsLAN = bIsLAN;
		CurrentSessionParams.bIsPresence = bIsPresence;
		CurrentSessionParams.UserId = UserId;

		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && CurrentSessionParams.UserId.IsValid())
		{
			SearchSettings = MakeShareable(new FCaterOnlineSearchSettings(bIsLAN, bIsPresence));
			SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, CustomMatchKeyword, EOnlineComparisonOp::Equals);

			TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SearchSettings.ToSharedRef();

			OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(
				OnFindSessionsCompleteDelegate);
			Sessions->FindSessions(*CurrentSessionParams.UserId, SearchSettingsRef);
		}
	}
	else
	{
		OnFindSessionsComplete(false);
	}
}

bool ACaterGameSession::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName InSessionName,
                                    int32 SessionIndexInSearchResults)
{
	bool bResult = false;

	if (SessionIndexInSearchResults >= 0 && SessionIndexInSearchResults < SearchSettings->SearchResults.Num())
	{
		bResult = JoinSession(UserId, InSessionName, SearchSettings->SearchResults[SessionIndexInSearchResults]);
	}

	return bResult;
}

bool ACaterGameSession::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName InSessionName,
                                    const FOnlineSessionSearchResult& SearchResult)
{
	bool bResult = false;

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && UserId.IsValid())
		{
			OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
				OnJoinSessionCompleteDelegate);
			bResult = Sessions->JoinSession(*UserId, InSessionName, SearchResult);
		}
	}

	return bResult;
}

/**
 * Delegate fired when the joining process for an online session has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
void ACaterGameSession::OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
	bool bWillTravel = false;

	UE_LOG(LogOnlineGame, Verbose, TEXT("OnJoinSessionComplete %s bSuccess: %d"), *InSessionName.ToString(),
	       static_cast<int32>(Result));

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
		}
	}

	OnJoinSessionComplete().Broadcast(Result);
}

bool ACaterGameSession::TravelToSession(int32 ControllerId, FName InSessionName)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		FString URL;
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && Sessions->GetResolvedConnectString(InSessionName, URL))
		{
			APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), ControllerId);
			if (PC)
			{
				PC->ClientTravel(URL, TRAVEL_Absolute);
				return true;
			}
		}
		else
		{
			UE_LOG(LogOnlineGame, Warning, TEXT("Failed to join session %s"), *SessionName.ToString());
		}
	}
#if !UE_BUILD_SHIPPING
	else
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), ControllerId);
		if (PC)
		{
			FString LocalURL(TEXT("127.0.0.1"));
			PC->ClientTravel(LocalURL, TRAVEL_Absolute);
			return true;
		}
	}
#endif //!UE_BUILD_SHIPPING

	return false;
}

void ACaterGameSession::RegisterServer()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		IOnlineSessionPtr SessionInt = OnlineSub->GetSessionInterface();
		if (SessionInt.IsValid())
		{
			TSharedPtr<class FCaterOnlineSessionSettings> CaterHostSettings = MakeShareable(
				new FCaterOnlineSessionSettings(false, false, 16));
			CaterHostSettings->Set(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"),
			                       EOnlineDataAdvertisementType::DontAdvertise);
			CaterHostSettings->Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
			CaterHostSettings->Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"),
			                       EOnlineDataAdvertisementType::DontAdvertise);
			CaterHostSettings->Set(SETTING_GAMEMODE, FString("TeamDeathmatch"),
			                       EOnlineDataAdvertisementType::ViaOnlineService);
			CaterHostSettings->Set(SETTING_MAPNAME, GetWorld()->GetMapName(),
			                       EOnlineDataAdvertisementType::ViaOnlineService);
			CaterHostSettings->bAllowInvites = true;
			CaterHostSettings->bIsDedicated = true;
			if (FParse::Param(FCommandLine::Get(), TEXT("forcelan")))
			{
				UE_LOG(LogOnlineGame, Log, TEXT("Registering server as a LAN server"));
				CaterHostSettings->bIsLANMatch = true;
			}
			HostSettings = CaterHostSettings;
			OnCreateSessionCompleteDelegateHandle = SessionInt->AddOnCreateSessionCompleteDelegate_Handle(
				OnCreateSessionCompleteDelegate);
			SessionInt->CreateSession(0, NAME_GameSession, *HostSettings);
		}
	}
}
