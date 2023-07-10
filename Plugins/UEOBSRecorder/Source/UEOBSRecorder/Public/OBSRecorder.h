// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IWebSocket.h"
#include "OBSRecorder.generated.h"

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
	
	static FString GenerateAuthenticationKey(FString& Password,FString& Salt,FString& Challenge); 
	
};
