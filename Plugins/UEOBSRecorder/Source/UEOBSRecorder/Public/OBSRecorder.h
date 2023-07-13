// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IWebSocket.h"
#include "OBSRecorder.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogOBSRecorder, Log, All);

DECLARE_LOG_CATEGORY_EXTERN(LogWebSocket, Log, All);

enum EOBSResponse : int8
{
	OpCode0 = 0,
	//Hello: First message sent from the server immediately on client connection. Contains authentication information if auth is required. Also contains RPC version for version negotiation.
	OpCode2 = 2,
	//Identified: The identify request was received and validated, and the connection is now ready for normal operation.
	OpCode5 = 5,
	//Event: An event coming from OBS has occured. Eg scene switched, source muted.
	OpCode7 = 7,
	//RequestResponse: obs-websocket is responding to a request coming from a client.
};

enum EClientRequest : int8
{
	OpCode1 = 1,
	//Identify: Response to Hello message, should contain authentication string if authentication is required, along with PubSub subscriptions and other session parameters.
	OpCode3 = 3,
	//Re-identify: Sent at any time after initial identification to update the provided session parameters.
	OpCode6 = 6,
	//Request: Client is making a request to obs-websocket. Eg get current scene, create source. 
};


/**
 * FRequestData
 * 
 **/
USTRUCT()
struct FRequestData
{
	GENERATED_BODY()
	FRequestData()
	{
	}
	FRequestData(const FString& RequestType, const FString& RequestId): requestType(RequestType), requestId(RequestId)
	{
	}
private:
	UPROPERTY()
	FString requestType;
	UPROPERTY()
	FString requestId;
};

/**
 * FMessage
 * 
 **/
USTRUCT()
struct FMessage
{
	GENERATED_BODY()
	FMessage()
	{
	}
	FMessage(const EClientRequest ClientRequest, const FRequestData& Request)
		: op(ClientRequest), d(Request)
	{
	}

	UPROPERTY()
	uint8 op;
	UPROPERTY()
	FRequestData d;
};

/**
 * 
 */
UCLASS()
class UEOBSRECORDER_API UOBSRecorder : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	TSharedPtr<IWebSocket> WebSocket;


	/**
	* public void OBSRecorder::StartConnection \n
	* Start a new connection with OBS websocket.
	* @return Success value
	**/
	UFUNCTION(BlueprintCallable)
	void StartConnection(bool& Success);

	
	/**
	* public void OBSRecorder::StartRecord \n
	* Start recording.
	**/
	UFUNCTION(BlueprintCallable)
	void StartRecord();


	/**
	* public void OBSRecorder::StopRecord \n
	* Stop recording.
	**/
	UFUNCTION(BlueprintCallable)
	void StopRecord();
	

	//Generates the key
	static FString GenerateAuthenticationKey(const FString& Password, const FString& Salt, const FString& Challenge);

	static FString HexToBase64(FString& HexString);


private:

	/**
	 * public void RespondOpCode0::StartConnection
	 * Response(OpCode 1) to OpCode 0 message, should contain authentication string if authentication is required, along with PubSub subscriptions and other session parameters.
	 * @param HelloMessageJson: Json response sent from obs-websocket when websocket client freshly connected.
	 * @param Password: websocket password
	 **/
	void Identify(const TSharedPtr<FJsonObject> HelloMessageJson, const FString& Password);

	const FString FormJsonRequestMessage(const FMessage& Message);
};
