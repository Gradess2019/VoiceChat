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

	UVoiceComponent();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voice")
	virtual bool IsSocketValidAndConnected(UPARAM(DisplayName = "Try to connect") bool bInTryToConnect = true);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voice")
	virtual bool IsSocketValid(bool bInTryToCreate = false);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voice")
	virtual bool IsSocketConnected(UPARAM(DisplayName = "Try to connect") bool bInTryToConnect = false);

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual void Start();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual void Stop();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voice")
	virtual bool IsCapturing();

protected:
	
	TSharedPtr<IVoiceCapture> VoiceCapture;
	TSharedPtr<IVoiceEncoder> VoiceEncoder;
	TSharedPtr<IVoiceDecoder> VoiceDecoder;
	
	TSharedPtr<FRemoteVoiceHandler> RemoteVoiceHandler;
	TSharedPtr<FLocalVoiceHandler> LocalVoiceHandler;
	TSharedPtr<FSocket, ESPMode::ThreadSafe> ClientSocket;

	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	int64 NumOfBytes;

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
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void InitRemoteVoiceHandler();

	UFUNCTION()
	virtual void CreateSocket();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	virtual bool ConnectToVoiceServer();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Voice")
	void CaptureAndSendVoiceData();
	virtual void CaptureAndSendVoiceData_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Voice")
	void CaptureVoice();
	virtual void CaptureVoice_Implementation();

	UFUNCTION()
	virtual void Send(UPARAM(DisplayName = "Data") const TArray<uint8>& InData);

	UFUNCTION()
	virtual void ReceiveVoiceData();

	friend class FRemoteVoiceHandler;
	friend class FLocalVoiceHandler;

};
