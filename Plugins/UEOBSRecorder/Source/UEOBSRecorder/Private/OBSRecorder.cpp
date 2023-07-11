// Fill out your copyright notice in the Description page of Project Settings.


#include "OBSRecorder.h"
#include "IWebSocket.h"
#include "SHA256Hash.h"
#include "HashSHA256BPLibrary.h"
#include "JsonBlueprintFunctionLibrary.h"
#include "JsonObjectWrapper.h"
#include "Containers/UnrealString.h"
#include "WebSocketsModule.h"

void UOBSRecorder::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
		UE_LOG(LogTemp, Warning, TEXT("WS module is loaded!"));
	}

	const FString URL = TEXT("ws://localhost:4456");
	const FString Protocol = TEXT("ws");

	WebSocket = FWebSocketsModule::Get().CreateWebSocket(URL, Protocol);
	// FWebSocketsModule::Get().OnWebSocketCreated

	WebSocket->OnConnected().AddLambda([]()
	{
		UE_LOG(LogTemp, Warning, TEXT("Connected to ws server"));
	});

	WebSocket->OnConnectionError().AddLambda([](const FString& Error)
	{
		UE_LOG(LogTemp, Warning, TEXT("Connection error: %s"), *Error);
	});

	WebSocket->OnMessage().AddLambda([&](const FString Message)
	{
		UE_LOG(LogTemp, Warning, TEXT("Message received: %s"), *Message);

		FJsonObjectWrapper JsonObjectWrapper;
		UJsonBlueprintFunctionLibrary::FromString(this->GetWorld(), Message, JsonObjectWrapper);
		const TSharedPtr<FJsonObject> OBSJsonResponse = JsonObjectWrapper.JsonObject;

		const FString MessageType = OBSJsonResponse->GetStringField("op");
		UE_LOG(LogTemp, Warning, TEXT("OpCode: %s"), *MessageType);
		
		//Generate key if OpCode 0 and verify
		if ("0" == MessageType)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hello from OBSWebsocket"));

			FString Challenge = OBSJsonResponse->GetObjectField("d")->GetObjectField("authentication")->GetStringField(
				"challenge");
			UE_LOG(LogTemp, Warning, TEXT("Challenge key:  %s"),*Challenge);

			
			FString Salt = OBSJsonResponse->GetObjectField("d")->GetObjectField("authentication")->
			                                GetStringField("salt");
			UE_LOG(LogTemp, Warning, TEXT("Salt key:  %s"),*Salt);

			const FString AuthenticationKey = GenerateAuthenticationKey(Salt, Challenge);
			
			const FString VerifyMessage = FString::Printf(
				TEXT("{\"op\": 1,\"d\": {\"rpcVersion\": 1,\"authentication\": \"%s\",\"eventSubscriptions\": 33}}"),*AuthenticationKey);
			UE_LOG(LogTemp, Warning, TEXT("Verify Message: %s"),*VerifyMessage);
			WebSocket->Send(VerifyMessage);
		}
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


FString UOBSRecorder::GenerateAuthenticationKey(const FString& Salt, const FString& Challenge)
{
	//Concatenate the websocket password with the salt provided by the server (password + salt)
	FString Key = "yVlXQondHRJWsWuS" + Salt;

	UE_LOG(LogTemp,Error,TEXT("Password + salt: %s"), *Key);
	
	//Generate an SHA256 binary hash of the result and base64 encode it, known as a base64 secret
	//Concatenate the base64 secret with the challenge sent by the server (base64_secret + challenge)

	FSHA256Hash Fsha256Hash;
	
	UHashSHA256BPLibrary::FromString(Fsha256Hash, Key);

	uint8* Source = new uint8[32];
	const uint32 Length = HexToBytes(Fsha256Hash.GetHash(),Source);							//From Hex string to byte array
	UE_LOG(LogTemp,Error,TEXT("Base64(Key(SHA256)): %s"), *FBase64::Encode(Source,Length)); //Encode byte array

	
	Key = FBase64::Encode(Source,Length) + Challenge;
	//UE_LOG(LogTemp,Error,TEXT("Base64(Key(SHA256)): %s"), *Base64.Encode(Fsha256Hash.GetHash()));
	
	UE_LOG(LogTemp,Error,TEXT("Base64(Key(SHA256))+Challenge = Key: %s"), *Key);
	
	
	//Generate a binary SHA256 hash of that result and base64 encode it. You now have your authentication string.
	FSHA256Hash Fsha256Hash2;
	
	UHashSHA256BPLibrary::FromString(Fsha256Hash2, Key);
	
	UE_LOG(LogTemp,Error,TEXT("Key(SHA2562): %s"), *Fsha256Hash2.GetHash());

	HexToBytes(Fsha256Hash2.GetHash(),Source);
	Key = FBase64::Encode(Source,Length);
	UE_LOG(LogTemp,Error,TEXT("Base64(Key(SHA2562)): %s"), *Key);

	delete[] Source;
	return Key;
}

