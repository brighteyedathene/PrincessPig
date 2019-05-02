// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkGameInstance.h"
#include "PrincessPig.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

UNetworkGameInstance::UNetworkGameInstance()
{
	/** Bind function for CREATING a Session */
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UNetworkGameInstance::OnCreateSessionComplete);
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UNetworkGameInstance::OnStartOnlineGameComplete);
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UNetworkGameInstance::OnFindSessionsComplete);
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UNetworkGameInstance::OnJoinSessionComplete);

}


bool UNetworkGameInstance::HostSession(TSharedPtr<const FUniqueNetId> UserId, FString UserSessionName, bool bIsLan, bool bIsPresence, int32 MaxNumPlayers)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogNetwork, Warning, TEXT("No OnlineSubsystem found!"));
		return false;
	}

	// Get the Session Interface, so we can call the "CreateSession" function on it
	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogNetwork, Warning, TEXT("SessionInterface was invalid!"));
		return false;
	}

	if (!UserId.IsValid())
	{
		UE_LOG(LogNetwork, Warning, TEXT(" UserID was invalid!"));
		return false;
	}
	

	/*
		Fill in all  the Session Settings that we want to use.

		There are more with SessionSettings.Set(...);
		For example the Map or the GameMode/Type.
	*/
	SessionSettings = MakeShareable(new FOnlineSessionSettings());

	SessionSettings->bIsLANMatch = bIsLan;
	SessionSettings->bUsesPresence = bIsPresence;
	SessionSettings->NumPublicConnections = MaxNumPlayers;
	SessionSettings->NumPrivateConnections = 0;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

	SessionSettings->Set(SETTING_MAPNAME, FString("/Game/Maps/LobbyMap.LobbyMap"), EOnlineDataAdvertisementType::ViaOnlineService);
	
	// Add the server name (given by user upon creating game) to search keywords 
	SessionSettings->Set(SEARCH_KEYWORDS, UserSessionName, EOnlineDataAdvertisementType::ViaOnlineService);


	// Set the delegate to the Handle of the SessionInterface
	OnCreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

	// Our delegate should get called when this is complete (doesn't need to be successful!)
	return SessionInterface->CreateSession(*UserId, GameSessionName, *SessionSettings);
	
}


void UNetworkGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnCreateSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));


	// Get the OnlineSubsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogNetwork, Warning, TEXT("No OnlineSubsystem found!"));
		return;
	}

	// Get the Session Interface to call the StartSession function
	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogNetwork, Warning, TEXT("SessionInterface was invalid!"));
		return;
	}

	// Clear the SessionComplete delegate handle, since we finished this call
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	if (bWasSuccessful)
	{
		// Set the StartSession delegate handle
		OnStartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

		// Our StartSessionComplete delegate should get called after this
		SessionInterface->StartSession(SessionName);
	}
	else 
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Green, FString::Printf(TEXT("Please make sure you are signed in to Steam!")));
		GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("Failed to create session!")));

		UGameplayStatics::OpenLevel(GetWorld(), "/Game/Maps/MainMenu", true, "");
	}

}


void UNetworkGameInstance::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnStartSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	// Get the Online Subsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogNetwork, Warning, TEXT("No OnlineSubsystem found!"));
		return;
	}
	
	// Get the Session Interface to clear the Delegate
	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogNetwork, Warning, TEXT("SessionInterface was invalid!"));
		return;
	}


	// Clear the delegate, since we are done with this call
	SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);

	// If the start was successful, we can open a NewMap if we want. Make sure to use "listen" as a parameter!
	if (bWasSuccessful)
	{
		UGameplayStatics::OpenLevel(GetWorld(), "/Game/Maps/LobbyMap", true, "listen");
		//FString NetworkNumber = GetWorld()->GetNetDriver()->LowLevelGetNetworkNumber();
		//GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Green, FString::Printf(TEXT("Network number: %s"), *NetworkNumber));
	}
}


void UNetworkGameInstance::FindSessions(TSharedPtr<const FUniqueNetId> UserId, FString SearchString, bool bIsLAN, bool bIsPresence)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		// If something goes wrong, just call the delegate function directly with "false"
		OnFindSessionsComplete(false);
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogNetwork, Warning, TEXT("SessionInterface was invalid!"));
		return;
	}

	if (!UserId.IsValid())
	{
		UE_LOG(LogNetwork, Warning, TEXT(" UserID was invalid!"));
		return;
	}
	

	/*
		Fill in all the search settings, like LAN, how many results, etc
	*/
	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	SessionSearch->bIsLanQuery = bIsLAN;
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->PingBucketSize = 50;
	
	// We only want to set this Query Setting if "bIsPresence" is true
	if (bIsPresence)
	{
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);
	}

	SessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, SearchString, EOnlineComparisonOp::Near);

	TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();

	// Set the delegate to the delegate-handle of the FindSession function
	OnFindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

	// Call the SessionInterface function. The delegate gets called when this is finished
	SessionInterface->FindSessions(*UserId, SearchSettingsRef);
	
}


void UNetworkGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OFindSessionsComplete bSuccess: %d"), bWasSuccessful));

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogNetwork, Warning, TEXT("No OnlineSubsystem found!"));
		return;
	}
	
	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogNetwork, Warning, TEXT("SessionInterface was invalid!"));
		return;
	}

	// Clear the delegate handle, since we finished this call
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

	// Just debugging the number of search results. Can be displayed in UMG or something later on
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Num Search Results: %d"), SessionSearch->SearchResults.Num()));


	SearchResults.Empty();

	// If we have found at least 1 session, we just going to debug them. You could add them to a list of UMG Widgets, like it is done in the BP version!
	if (SessionSearch->SearchResults.Num() > 0)
	{
		// "SessionSearch->SearchResults" is an Array that contains all the information. You can access the Session in this and get a lot of information.
		// This can be customized later on with your own classes to add more information that can be set and displayed
		for (int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++)
		{
			// OwningUserName is just the SessionName for now. I guess you can create your own Host Settings class and GameSession Class and add a proper GameServer Name here.
			// This is something you can't do in Blueprint for example!
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange, FString::Printf(TEXT("Session Number: %d | Sessionname: %s "), SearchIdx + 1, *(SessionSearch->SearchResults[SearchIdx].Session.OwningUserName)));

			FSearchResultInfo ResultInfo;
			FString SearchName;
			if (SessionSearch->SearchResults[SearchIdx].Session.SessionSettings.Get(SEARCH_KEYWORDS, SearchName))
			{
				ResultInfo.Name = SearchName;
			}
			else
			{
				ResultInfo.Name = SessionSearch->SearchResults[SearchIdx].Session.OwningUserName;
			}
			
			ResultInfo.Ping = SessionSearch->SearchResults[SearchIdx].PingInMs;
			ResultInfo.ResultIndex = SearchIdx;


			// Apply search filter here (since it doesn't seem to work as expected in the subsystem session finder)
			FString SearchKeyword;
			if (SessionSearch->QuerySettings.Get(SEARCH_KEYWORDS, SearchKeyword))
			{
				if (SearchKeyword.IsEmpty() || ResultInfo.Name.Contains(SearchKeyword))
				{
					SearchResults.Add(ResultInfo);
				}
			}
			else
			{
				SearchResults.Add(ResultInfo);
			}
		}
	}

	// Let blueprints know 
	BPEvent_SessionSearchResultsUpdated();
}


bool UNetworkGameInstance::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult & SearchResult)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogNetwork, Warning, TEXT("No OnlineSubsystem found!"));
		return false;
	}

	// Get the Session Interface, so we can call the "CreateSession" function on it
	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogNetwork, Warning, TEXT("SessionInterface was invalid!"));
		return false;
	}

	if (!UserId.IsValid())
	{
		UE_LOG(LogNetwork, Warning, TEXT(" UserID was invalid!"));
		return false;
	}


	// Set the handle for OnJoinSessionCompleteDelegate
	OnJoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

	// Call the "JoinSession" function with the given SearchResult.
	// SessionSearch->SearchResults can be used to get a SearchResult
	return SessionInterface->JoinSession(*UserId, SessionName, SearchResult);
}


void UNetworkGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnJoinSessionComplete %s, %d"), *SessionName.ToString(), static_cast<int32>(Result)));

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogNetwork, Warning, TEXT("No OnlineSubsystem found!"));
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogNetwork, Warning, TEXT("SessionInterface was invalid!"));
		return;
	}

	
	// Clear the OnJoinSessionCompleteDelegateHandle
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

	// Get the first local PlayerController, so we can call "ClientTravel" to get to the Server Map
	// This is something the Blueprint Node "Join Session" does automatically!
	APlayerController* const PlayerController = GetFirstLocalPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogNetwork, Warning, TEXT("OnJoinSessionComplete couldn't find PlayerController"));
		return;
	}

	// We need a FString to use ClientTravel and we can let the SessionInterface contruct such a
	// string for us by giving him the SessionName and an empty String. We want to do this, 
	// because every OnlineSubsystem uses different TravelURLs.
	FString TravelURL;

	if (SessionInterface->GetResolvedConnectString(SessionName, TravelURL))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Calling ClientTravel to: %s"), *TravelURL));

		// Finally, call ClientTravel
		PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
	}

}


void UNetworkGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnDestroySessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogNetwork, Warning, TEXT("No OnlineSubsystem found!"));
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogNetwork, Warning, TEXT("SessionInterface was invalid!"));
		return;
	}

	// Clear the delegate handle
	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);

	if (bWasSuccessful)
	{
		UGameplayStatics::OpenLevel(GetWorld(), "Maps/MainMenu", true);
	}
}



void UNetworkGameInstance::HostOnlineGame(FString UserSessionName, bool bIsLan, int32 MaxNumPlayers)
{
	// Create a local player where we can get the UserID from
	ULocalPlayer* const LocalPlayer = GetFirstGamePlayer();

	// Call our custom Hostsession function. GameSessionName is a GameInstance variable
	HostSession(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), UserSessionName, bIsLan, true, MaxNumPlayers);

}


void UNetworkGameInstance::FindOnlineGame(FString SearchString)
{
	ULocalPlayer* const LocalPlayer = GetFirstGamePlayer();
	FindSessions(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), SearchString, false, true);
}


void UNetworkGameInstance::JoinOnlineGame(int SearchResultIndex)
{
	ULocalPlayer* const LocalPlayer = GetFirstGamePlayer();

	if (SessionSearch->SearchResults.IsValidIndex(SearchResultIndex))
	{
		JoinSession(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, SessionSearch->SearchResults[SearchResultIndex]);
	}
	
}


void UNetworkGameInstance::DestroySessionAndLeaveGame()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogNetwork, Warning, TEXT("No OnlineSubsystem found!"));
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogNetwork, Warning, TEXT("SessionInterface was invalid!"));
		return;
	}


	SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
	SessionInterface->DestroySession(GameSessionName);
}
