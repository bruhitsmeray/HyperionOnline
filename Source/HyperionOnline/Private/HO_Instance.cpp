// Fill out your copyright notice in the Description page of Project Settings.


#include "HO_Instance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineIdentityInterface.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

IOnlineSubsystem* OnlineSubsystem;

UHO_Instance::UHO_Instance(){
	bIsLoggedIn = false;
	bIsSessionLive = false;
}

void UHO_Instance::Init()
{
	Super::Init();
	if(GameType == "Online") {
		OnlineSubsystem = IOnlineSubsystem::Get();
		Login();
	}
}

void UHO_Instance::Shutdown()
{
	Super::Shutdown();
}

FString UHO_Instance::GetOnlinePlatformUserName(){
	if(OnlinePlatformUserName.IsEmpty() || OnlinePlatformUserName == "DummyDisplayName") {
		return UKismetSystemLibrary::GetPlatformUserName();
	}
	return OnlinePlatformUserName;
}

void UHO_Instance::Login(){
	if(GameType == "Online") {
		if (OnlineSubsystem) {
			if (IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface()) {
				FOnlineAccountCredentials Credentials;
				Credentials.Id = FString("localhost:6300");
				Credentials.Token = FString("C1");
				Credentials.Type = FString("developer");

				Identity->OnLoginCompleteDelegates->AddUObject(this, &UHO_Instance::OnLoginComplete);
				Identity->Login(0, Credentials);
			}
		}
	} else {
		UE_LOG(LogTemp, Warning, TEXT("You can't login while the GameType is set to: Singleplayer."));
	}
}

void UHO_Instance::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
	const FString& Error){
	bIsLoggedIn = bWasSuccessful;
	if(OnlineSubsystem) {
		UE_LOG(LogTemp, Warning, TEXT("Client successfully logged in. Code: %d"), bWasSuccessful);
		
		if(IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface()) {
			OnlinePlatformUserName = Identity->GetUserAccount(UserId)->GetDisplayName();
			Identity->ClearOnLoginCompleteDelegates(0, this);
		} else {
			UE_LOG(LogTemp, Warning, TEXT("Client failed to call the identity interface."));
		}
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Client failed to log in. Code: %d"), bWasSuccessful);
	}
}

void UHO_Instance::CreateEOSSession(bool bIsDedicatedServer, bool bIsLanServer, int32 NumberOfPlayers){
	if(OnlineSubsystem && bIsLoggedIn) {
		if(IOnlineSessionPtr Session = OnlineSubsystem->GetSessionInterface()) {
			FOnlineSessionSettings SessionSettings;
			SessionSettings.bIsDedicated = bIsDedicatedServer;
			SessionSettings.bIsLANMatch = bIsLanServer;
			SessionSettings.bUsesPresence = false;
			SessionSettings.bAllowInvites = true;
			SessionSettings.bAllowJoinInProgress = true;
			SessionSettings.bUseLobbiesIfAvailable = true;
			SessionSettings.bUseLobbiesVoiceChatIfAvailable = true;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.NumPublicConnections = NumberOfPlayers;
			SessionSettings.Set(SEARCH_KEYWORDS, FString("AlephTrialServer"), EOnlineDataAdvertisementType::ViaOnlineService);
			
			Session->OnCreateSessionCompleteDelegates.AddUObject(this, &UHO_Instance::OnCreateEOSSessionComplete);
			Session->CreateSession(0, FName("Session"), SessionSettings);
		}
	}
}

void UHO_Instance::OnCreateEOSSessionComplete(FName SessionName, bool bWasSuccesful){
	if(bWasSuccesful) {
		if(OnlineSubsystem) {
			if(IOnlineSessionPtr Session = OnlineSubsystem->GetSessionInterface()) {
				Session->ClearOnCreateSessionCompleteDelegates(this);
				GetWorld()->ServerTravel("dev_arena?listen");
			}
		}
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Session creation failed. Please try again!"));
	}
}

void UHO_Instance::FindAndJoinSession(){
	if(OnlineSubsystem && bIsLoggedIn) {
		if(IOnlineSessionPtr Session = OnlineSubsystem->GetSessionInterface()) {
			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->bIsLanQuery = false;
			SessionSearch->MaxSearchResults = 20;
			SessionSearch->QuerySettings.SearchParams.Empty();
			Session->OnFindSessionsCompleteDelegates.AddUObject(this, &UHO_Instance::OnFindAndJoinSessionComplete);
			Session->FindSessions(0, SessionSearch.ToSharedRef());
		}
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Find session failed because of the Client not being logged in!"));
	}
}

void UHO_Instance::OnFindAndJoinSessionComplete(bool bWasSuccesful){
	if(bWasSuccesful) {
		if(OnlineSubsystem && bIsLoggedIn) {
			IOnlineSessionPtr Session = OnlineSubsystem->GetSessionInterface();
			if(Session && SessionSearch->SearchResults.Num() >= 0){
				Session->OnJoinSessionCompleteDelegates.AddUObject(this, &UHO_Instance::OnJoinSessionComplete);
				Session->JoinSession(0, FName("Session"), SessionSearch->SearchResults[0]);
			} else {
				CreateEOSSession(false, false, 2);
			}
		}
	} else {
		CreateEOSSession(false, false, 2);
	}
}

void UHO_Instance::JoinEOSSession(){
	
}

void UHO_Instance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result){
	if(Result == EOnJoinSessionCompleteResult::Success) {
		if(APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0)) {
			FString JoinAddress;
			if(OnlineSubsystem) {
				IOnlineSessionPtr Session = OnlineSubsystem->GetSessionInterface();
				if(Session) {
					Session->GetResolvedConnectString(FName("Session"), JoinAddress);
					if(!JoinAddress.IsEmpty()) {
						Controller->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
					}
				}
			}
		}
	}
}
