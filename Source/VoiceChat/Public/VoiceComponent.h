// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoiceComponent.generated.h"

class IVoiceCapture;
class USoundWaveProcedural;
class USoundWave;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VOICECHAT_API UVoiceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVoiceComponent();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual void Start();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual void Stop();

protected:

	virtual void BeginPlay() override;

	TSharedPtr<IVoiceCapture> VoiceCapture;

	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	TArray<uint8> VoiceCaptureBuffer;

	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	bool PlayVoiceCaptureFlag;

	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	FTimerHandle VoiceCaptureTickTimer;
	
	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	FTimerHandle PlayVoiceCaptureTimer;

	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	USoundWaveProcedural* SoundWave;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Voice")
	USoundWave* TestSoundWave;
	
	UPROPERTY(BlueprintReadWrite, Category = "Voice")
	UAudioComponent* AudioComponent;

	UPROPERTY()
	USceneComponent* RootComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	float VoiceCaptureVolume;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Voice")
	void VoiceCaptureTick();

	virtual void VoiceCaptureTick_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Voice")
	void PlayVoiceCapture();

	virtual void PlayVoiceCapture_Implementation();
	
public:	

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
};
