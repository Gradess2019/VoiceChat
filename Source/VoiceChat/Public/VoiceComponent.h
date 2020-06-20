// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoiceComponent.generated.h"


class IVoiceCapture;
class IVoiceEncoder;
class IVoiceDecoder;
class USoundWaveProcedural;
class USoundWave;
class APawn;
class UAudioComponent;
class FSocket;
class FRemoteVoiceHandler;
class FLocalVoiceHandler;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VOICECHAT_API UVoiceComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void InitRemoteVoiceHandler();
	void SetConnectToServerTimer();
	UVoiceComponent();
	void CreateSocket();
	bool CheckSocketConnection(bool bInTryToConnect = false);
	bool CheckSocket(bool bInTryToCreate = false);

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual void Start();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual void Stop();

protected:

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	bool ConnectToVoiceServer();
	bool SocketConnected(bool bInTryToConnect = true);

	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	int64 NumOfBytes;
	
	TSharedPtr<IVoiceCapture> VoiceCapture;
	TSharedPtr<IVoiceEncoder> VoiceEncoder;
	TSharedPtr<IVoiceDecoder> VoiceDecoder;
	
	TSharedPtr<FRemoteVoiceHandler> RemoteVoiceHandler;
	TSharedPtr<FLocalVoiceHandler> LocalVoiceHandler;
	TSharedPtr<FSocket, ESPMode::ThreadSafe> ClientSocket;

	TSharedPtr<UVoiceComponent, ESPMode::ThreadSafe> SelfPtr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voice")
	FString Address;
	
	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	TArray<uint8> VoiceCaptureBuffer;

	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	TArray<uint8> ReplicatedBuffer;

	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	TArray<uint8> AdditionalReplicatedBuffer;

	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	USoundWaveProcedural* SoundWave;

	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	UAudioComponent* AudioComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	float VoiceCaptureVolume;

	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	APawn* Owner;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voice")
	float Rate;
	
	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	bool bCapturing;
	
	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	FTimerHandle ConnectToServerTimer;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Voice")
	void CaptureVoice();

	virtual void CaptureVoice_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Voice")
	void CaptureAndSendVoiceData();

	virtual void CaptureAndSendVoiceData_Implementation();

	// TCP
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void Send(UPARAM(DisplayName = "Data") const TArray<uint8>& InData);

	UFUNCTION()
	void ReceiveVoiceData();

	friend class FRemoteVoiceHandler;
	friend class FLocalVoiceHandler;

};
