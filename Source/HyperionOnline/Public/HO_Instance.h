// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "HO_Instance.generated.h"

/**
 * 
 */
UCLASS()
class HYPERIONONLINE_API UHO_Instance : public UGameInstance
{
	GENERATED_BODY()

public:
	UHO_Instance();
	virtual void Init() override;
	virtual void Shutdown() override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Preferences", meta = (GetOptions = "GameTypeList"))
	FString GameType = "Offline";
	UFUNCTION(BlueprintCallable, Category="Preferences")
	TArray<FString> GameTypeList() const {
	return {
		"Offline",
		"Online"
		};
	};

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Online|Status")
	bool bIsLoggedIn;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Online|Status")
	bool bIsSessionLive;
	UFUNCTION(BlueprintCallable, Category="Online|Info")
	FString GetOnlinePlatformUserName();
	FString OnlinePlatformUserName;

public:
	UFUNCTION(Exec, BlueprintCallable, Category="Online|Login")
	void Login();
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	UFUNCTION(Exec, BlueprintCallable, Category="Online|Sessions")
	void CreateEOSSession(bool bIsDedicatedServer, bool bIsLanServer, int32 NumberOfPlayers);
	void OnCreateEOSSessionComplete(FName SessionName, bool bWasSuccesful);

	UFUNCTION(Exec, BlueprintCallable, Category="Online|Sessions")
	void FindAndJoinSession();
	void OnFindAndJoinSessionComplete(bool bWasSuccesful);

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UFUNCTION(Exec, BlueprintCallable, Category="Online|Sessions")
	void JoinEOSSession();
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
