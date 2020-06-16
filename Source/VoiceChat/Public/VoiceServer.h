// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/LockFreeFixedSizeAllocator.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "TimerManager.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "VoiceServer.generated.h"

#define MAX_VOICE_PACKAGE_SIZE 4096
#define COMMAND_SIZE 4
#define REGISTER_COMMAND TArray<uint8>({ 1, 1, 1, 1 })
#define VOICE_INPUT_COMMAND TArray<uint8>({ 1, 1, 1, 2 })

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
	bool InitIP(UPARAM(DisplayName = "LAN") bool bInLan);

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void StartTCPServer();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	bool CheckSockets();
	
	bool ConnectionAccept(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

protected:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voice")
	FString Address;

	TMap<FIPv4Endpoint, TSharedPtr<FSocket, ESPMode::ThreadSafe>> RegisteredSockets;

	TSharedPtr<FTcpListener> Listener;

	TSharedPtr<FVoiceServerThread> ServerThread;

	UPROPERTY()
	FTimerHandle SocketTimer;

	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
