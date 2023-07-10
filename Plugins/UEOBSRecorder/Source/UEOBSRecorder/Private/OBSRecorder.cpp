// Fill out your copyright notice in the Description page of Project Settings.


#include "OBSRecorder.h"
#include "IWebSocket.h"
#include "WebSocketsModule.h"
#include "GameFramework/SpectatorPawn.h"

void UOBSRecorder::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
		UE_LOG(LogTemp,Warning,TEXT("WS module is loaded!"));
	}

	const FString URL = TEXT("ws://localhost:4456");
	const FString Protocol = TEXT("ws");
	
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(URL,Protocol);
	// FWebSocketsModule::Get().OnWebSocketCreated
	
	WebSocket->OnConnected().AddLambda([]()
	{
		UE_LOG(LogTemp,Warning,TEXT("Connected to ws server"));
	});

	WebSocket->OnConnectionError().AddLambda([](const FString& Error)
	{
		UE_LOG(LogTemp,Warning,TEXT("Connection error: %s"),*Error);
	});

	WebSocket->OnMessage().AddLambda([](const FString Message)
	{
		UE_LOG(LogTemp,Warning,TEXT("Message received: %s"),*Message);
	});
	
	WebSocket->Connect();
}




void UOBSRecorder::Deinitialize()
{
	if (WebSocket->IsConnected())
	{
		WebSocket->Close();
	}
	Super::Deinitialize();
}

FString UOBSRecorder::GenerateAuthenticationKey(FString& Password, FString& Salt, FString& Challenge)
{
	FString Key = Password + Salt;

	
	
	
	return Key;
}



