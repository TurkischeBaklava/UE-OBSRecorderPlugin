// Fill out your copyright notice in the Description page of Project Settings.


#include "OBSRecorder.h"
#include "IWebSocket.h"
#include "SHA256Hash.h"
#include "HashSHA256BPLibrary.h"
#include "JsonBlueprintFunctionLibrary.h"
#include "JsonObjectWrapper.h"
#include "Containers/UnrealString.h"
#include "WebSocketsModule.h"

DEFINE_LOG_CATEGORY(LogWebSocket);
DEFINE_LOG_CATEGORY(LogOBSRecorder);


void UOBSRecorder::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//Load WebSocket module if already not
	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
		UE_LOG(LogWebSocket, Warning, TEXT("WS module is loaded!"));
	}

	const FString Host = TEXT("ws://localhost:");
	const FString Port = TEXT("4456");
	const FString URL = Host + Port;
	const FString Protocol = TEXT("ws");
	const FString Password = TEXT("yVlXQondHRJWsWuS");

	WebSocket = FWebSocketsModule::Get().CreateWebSocket(URL, Protocol);

	WebSocket->OnConnected().AddLambda([Port,Protocol]()
	{
		UE_LOG(LogWebSocket, Display, TEXT("Connected to websocket server succesfully: \n\tPort: %s\n\tProtocol: %s\n"),
		       *Port, *Protocol);
	});


	WebSocket->OnMessage().AddLambda([&,Password](const FString Message)
	{
		UE_LOG(LogOBSRecorder, Display, TEXT("Message received: %s"), *Message);

		FJsonObjectWrapper JsonObjectWrapper;
		UJsonBlueprintFunctionLibrary::FromString(this->GetWorld(), Message, JsonObjectWrapper);
		const TSharedPtr<FJsonObject> OBSJsonResponse = JsonObjectWrapper.JsonObject;

		const FString MessageType = OBSJsonResponse->GetStringField("op");

		//Identify if receive OpCode0
		if (MessageType == FString::FromInt(OpCode0)) Identify(OBSJsonResponse, Password);
		//Log OpCode 2
		else if (MessageType == FString::FromInt(OpCode2)) UE_LOG(LogOBSRecorder, Log,
		                                    TEXT(
			                                    "The identify request was received and validated, and the connection is now ready for normal operation."
		                                    ));
		
	});

	WebSocket->OnConnectionError().AddLambda([Port,Protocol](const FString& ErrorMessage)
	{
		UE_LOG(LogWebSocket, Error,
		       TEXT("Failed to connect to WebSocket server: \n\tPort: %s\n\tProtocol: %s\n\tError Message: %s\n"),
		       *Port, *Protocol, *ErrorMessage);
	});

	WebSocket->OnClosed().AddLambda([Port,Protocol](int32 StatusCode, const FString& Reason, bool bWasClean)
	{
		UE_LOG(LogWebSocket, Display,
		       TEXT("WebSocket connection closed: \n\tPort: %s\n\tProtocol: %s\n\tStatus Code: %d\n\tReason: %s\n\tWas Clean: %d\n"),
		       *Port, *Protocol,StatusCode,*Reason,bWasClean);
	});
}


void UOBSRecorder::Deinitialize()
{
	if (WebSocket->IsConnected())
	{
		WebSocket->Close();
	}
	Super::Deinitialize();
}


void UOBSRecorder::StartConnection(bool& Success)
{
	if (!WebSocket->IsConnected())
	{
		WebSocket->Connect();
	}
}

FString UOBSRecorder::GenerateAuthenticationKey(const FString& Password, const FString& Salt, const FString& Challenge)
{
	//Concatenate the websocket password with the salt provided by the server (password + salt)
	FString SecretString = Password + Salt;
	//UE_LOG(LogTemp,Error,TEXT("Password + salt: %s"), *SecretString);

	//Generate an SHA256 binary hash of the result and base64 encode it, known as a base64 secret
	FSHA256Hash Fsha256Hash;
	UHashSHA256BPLibrary::FromString(Fsha256Hash, SecretString);
	FString SHA256 = Fsha256Hash.GetHash();
	//UE_LOG(LogTemp,Error,TEXT("SHA256: %s"), *SHA256);

	//Concatenate the base64 secret with the challenge sent by the server (base64_secret + challenge)
	SecretString = HexToBase64(SHA256) + Challenge;

	//Generate a binary SHA256 hash of that result and base64 encode it. You now have your authentication string.
	UHashSHA256BPLibrary::FromString(Fsha256Hash, SecretString);
	SHA256 = Fsha256Hash.GetHash();
	SecretString = HexToBase64(SHA256);

	return SecretString;
}

FString UOBSRecorder::HexToBase64(FString& HexString)
{
	uint8* Source = new uint8[32]; //TODO: Fix 
	const uint32 Length = HexToBytes(HexString, Source); //Decode Hex String to byte arrays

	HexString = FBase64::Encode(Source, Length); //Encode Base64

	delete[] Source;
	return HexString;
}

void UOBSRecorder::Identify(const TSharedPtr<FJsonObject> HelloMessageJson, const FString& Password)
{
	UE_LOG(LogOBSRecorder, Display, TEXT("Hello OBSWebsocket!"));
	UE_LOG(LogOBSRecorder, Display, TEXT("Generating authenticator key and verifying client..."));

	//Get challenge field
	const FString Challenge = HelloMessageJson->GetObjectField("d")->GetObjectField("authentication")->GetStringField(
		"challenge");

	//Get salt field
	const FString Salt = HelloMessageJson->GetObjectField("d")->GetObjectField("authentication")->
	                                       GetStringField("salt");

	const FString AuthenticationKey = GenerateAuthenticationKey(Password, Salt, Challenge);

	//Create Identify (OpCode 1) message
	//TODO: Fix
	const FString IdentifyMessage = FString::Printf(
		TEXT("{\"op\": 1,\"d\": {\"rpcVersion\": 1,\"authentication\": \"%s\",\"eventSubscriptions\": 33}}"),
		*AuthenticationKey);

	WebSocket->Send(IdentifyMessage); //Sends 
}

void UOBSRecorder::FormJsonMessage(const EClientRequest Request)
{
}
