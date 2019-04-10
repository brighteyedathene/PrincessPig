// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "NetworkGameInstance.generated.h"


USTRUCT(BlueprintType)
struct FSearchResultInfo
{
	GENERATED_BODY()

	/** Name - Maybe a searchable term in future */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	FString Name;

	/** Ping in ms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	int32 Ping;

	/** Index of this session in the SearchResults array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	int ResultIndex;
};

/**
 * 
 */
UCLASS()
class PRINCESSPIG_API UNetworkGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UNetworkGameInstance();

	/* Variable to store session settings */
	TSharedPtr<class FOnlineSessionSettings> SessionSettings;
	
	/* Variable to store search settings and results */
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	
	/* Delegate called when session is created */
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;

	/* Delegate called when session is started */
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	
	/** Delegate for searching for sessions */
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;

	/** Delegate for destroying a session */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	/** Delegate for joining a session */
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

	/** Handle to registered delegate for creating a session */
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;

	/** Handle to registered delegate for starting a session */
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	
	/** Handle to registered delegate for searching a session */
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	/** Handle to registered delegate for joining a session */
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	/** Handle to registered delegate for destroying a session */
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;


	/**
	*	Function to host a game!
	*
	*	@Param		UserID			User that started the request
	*	@Param		SessionName		Name of the Session
	*	@Param		bIsLAN			Is this is LAN Game?
	*	@Param		bIsPresence		"Is the Session to create a presence Session"
	*	@Param		MaxNumPlayers	        Number of Maximum allowed players on this "Session" (Server)
	*/
	bool HostSession(TSharedPtr<const FUniqueNetId> UserId, FString UserSessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers);


	/**
	*	Function fired when a session create request has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);


	/**
	*	Function fired when a session start request has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);


	/**
	*	Find an online session
	*
	*	@param UserId user that initiated the request
	*	@param bIsLAN are we searching LAN matches
	*	@param bIsPresence are we searching presence sessions
	*/
	void FindSessions(TSharedPtr<const FUniqueNetId> UserId, FString SearchString, bool bIsLAN, bool bIsPresence);


	/**
	*	Delegate fired when a session search query has completed
	*
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnFindSessionsComplete(bool bWasSuccessful);


	/**
	*	Joins a session via a search result
	*
	*	@param SessionName name of session
	*	@param SearchResult Session to join
	*
	*	@return bool true if successful, false otherwise
	*/
	bool JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);


	/**
	*	Delegate fired when a session join request has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);


	/**
	*	Delegate fired when a destroying an online session has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void HostOnlineGame(FString UserSessionName, bool bIsLan, int32 MaxNumPlayers);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void FindOnlineGame(FString SearchString);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void JoinOnlineGame(int SearchResultIndex);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void DestroySessionAndLeaveGame();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	TArray<FSearchResultInfo> SearchResults;


	UFUNCTION(BlueprintImplementableEvent, Category = "Network")
	void BPEvent_SessionSearchResultsUpdated();
};
