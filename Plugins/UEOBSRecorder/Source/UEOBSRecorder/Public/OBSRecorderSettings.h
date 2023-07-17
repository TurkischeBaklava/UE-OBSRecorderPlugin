// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "OBSRecorderSettings.generated.h"

/**
 * 
 */
UCLASS(Config=EditorPerProjectUserSettings,meta=(DisplayName="OBS Recorder"))
class UEOBSRECORDER_API UOBSRecorderSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FName GetContainerName() const { return FName("Project"); }
	virtual FName GetCategoryName() const { return FName("Plugins"); }
#endif

	UPROPERTY(config, EditAnywhere, Category="WebSocket",
		meta=(DisplayName="OBS WebSocket Server Password", ToolTip=
			"Find the websocket password from your obs-websocket settings"))
	FString OBSWebSocketPassword;

	UPROPERTY(config, EditAnywhere, Category="WebSocket",
		meta=(DisplayName="Local Host", ToolTip=
			"Local host"))
	FString Host = "ws://localhost:";

	UPROPERTY(config, EditAnywhere, Category="WebSocket",
		meta=(DisplayName="OBS WebSocket Server Port", ToolTip=
			"Find the websocket port from your obs-websocket settings"))
	FString ServerPort;
	
};
