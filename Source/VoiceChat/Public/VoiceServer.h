// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/LockFreeFixedSizeAllocator.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "TimerManager.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "VoiceServer.generated.h"

#define MAX_VOICE_PACKAGE_SIZE 2048

#define MEGABYTE 1024 * 1024

#define SERVER_RECEIVE_BUFFER_SIZE 6 * MEGABYTE

#define CLIENT_RECEIVE_BUFFER_SIZE 2 * MEGABYTE
#define CLIENT_SEND_BUFFER_SIZE 2 * MEGABYTE

#define VOICE_RATE 0.3f
#define WAIT_ONE_RATE FTimespan::FromSeconds(VOICE_RATE)

class FSocket;
class FTcpListener;
class FVoiceServerThread;

UCLASS(BlueprintType, Blueprintable)
class VOICECHAT_API UVoiceServer : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Voice")
	static int GetDefaultPort();
	
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void InitIP();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void StartTCPServer();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	bool CheckSockets();
	
	bool ConnectionAccept(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

protected:

	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	FString IP;

	TMap<FIPv4Endpoint, TSharedPtr<FSocket, ESPMode::ThreadSafe>> RegisteredSockets;

	TSharedPtr<FTcpListener> Listener;

	TSharedPtr<FVoiceServerThread> ServerThread;

};
