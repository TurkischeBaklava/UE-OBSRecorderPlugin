// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IWebSocket.h"
#include "OBSRecorder.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogOBSRecorder, Log, All);

DECLARE_LOG_CATEGORY_EXTERN(LogWebSocket, Log, All);

enum EObsResponse : int8
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
	//Identify: Response to Hello message, should contain authentication string if authentication is required, along with PubSub subscriptions and other session parameters.
	OpCode1 = 1,
	//Re-identify: Sent at any time after initial identification to update the provided session parameters.
	OpCode3 = 3,
	//Request: Client is making a request to obs-websocket. Eg get current scene, create source. 
	OpCode6 = 6,
};

UENUM(BlueprintType)
enum class ERecordRequest : uint8
{
	StartRecord UMETA(DisplayName= "StartRecord"),
	StopRecord UMETA(DisplayName= "StopRecord"),
	PauseRecord UMETA(DisplayName= "PauseRecord"),
	ResumeRecord UMETA(DisplayName= "ResumeRecord"),
	ToggleRecord UMETA(DisplayName= "ToggleRecord"),
	ToggleRecordPause UMETA(DisplayName= "ToggleRecordPause")
};



/**
 *	OBSRecorder subsystem created for websocket communication and operations
 *	created by @alper.gunes
 * 
 **/
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
	 *	Make the selected request from ERecordRequest to OBS.
	 *	@param RecordRequest is intended request.
	 * 
	 **/
	UFUNCTION(BlueprintCallable)
	void MakeRecordRequest(const ERecordRequest RecordRequest);


private:
	/**
	 * public void RespondOpCode0::StartConnection
	 * Response(OpCode 1) to OpCode 0 message, should contain authentication string if authentication is required, along with PubSub subscriptions and other session parameters.
	 * @param HelloMessageJson: Json response sent from obs-websocket when websocket client freshly connected.
	 * @param Password: websocket password
	 **/
	void Identify(const TSharedPtr<FJsonObject> HelloMessageJson, const FString& Password);

	//Generates the key
	static FString GenerateAuthenticationKey(const FString& Password, const FString& Salt, const FString& Challenge);

	static FString HexToBase64(FString& HexString);

	const FString FormJsonMessage(const EClientRequest OpCode, TSharedPtr<FJsonObject> DataJsonObject);
};
