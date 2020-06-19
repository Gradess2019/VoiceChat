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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VOICECHAT_API UVoiceComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UVoiceComponent();
	void CreateSocket();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual void Start();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual void Stop();

protected:

	virtual void BeginPlay() override;
	void ConnectToVoiceServer();

	TSharedPtr<IVoiceCapture> VoiceCapture;
	TSharedPtr<IVoiceEncoder> VoiceEncoder;
	TSharedPtr<IVoiceDecoder> VoiceDecoder;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voice")
	FString Address;
	
	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	TArray<uint8> VoiceCaptureBuffer;

	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	TArray<uint8> ReplicatedBuffer;

	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	TArray<uint8> AdditionalReplicatedBuffer;

	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	bool PlayVoiceCaptureFlag;

	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	FTimerHandle RecieveVoiceDataTimer;
	
	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	FTimerHandle PlayVoiceCaptureTimer;

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
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voice")
	bool bCapturing;

	TSharedPtr<FSocket, ESPMode::ThreadSafe> ClientSocket;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Voice")
	void VoiceCaptureTick();

	virtual void VoiceCaptureTick_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Voice")
	void PlayVoiceCapture();

	virtual void PlayVoiceCapture_Implementation();

	// Voice buffer replication
	UFUNCTION(NetMulticast, Unreliable)
	void SetBuffer_Multicast(const TArray<uint8>& InVoiceBuffer);

	// TCP
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void Send(UPARAM(DisplayName = "Data") const TArray<uint8>& InData);

	UFUNCTION()
	void RecieveVoiceData();

	friend class FRemoteVoiceHandler;
	friend class FLocalVoiceHandler;

public:	

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
};
