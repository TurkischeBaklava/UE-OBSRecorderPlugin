// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IWebSocket.h"
#include "OBSRecorder.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogOBSRecorder, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogWebSocket, Log, All);

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

	UFUNCTION(BlueprintCallable)
	void StartConnection();
	
	//Generates the key
	static FString GenerateAuthenticationKey(const FString& Password,const FString& Salt,const FString& Challenge);
	
	static FString HexToBase64(FString& HexString);
	

private:

	void RespondOpCode0(const TSharedPtr<FJsonObject> OBSJsonResponse,const FString& Password);
	void RespondOpCode2(){UE_LOG(LogOBSRecorder,Log,TEXT("The identify request was received and validated, and the connection is now ready for normal operation."))}
};
