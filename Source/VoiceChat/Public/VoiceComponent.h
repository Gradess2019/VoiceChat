// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoiceComponent.generated.h"

class IVoiceCapture;
class IVoiceDecoder;
class IVoiceEncoder;
class UVoipListenerSynthComponent;

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
	TSharedPtr<IVoiceEncoder> Encoder;
	TSharedPtr<IVoiceDecoder> Decoder;

	TArray<uint8> DecompressedBuffer;

	UVoipListenerSynthComponent* SynthComponent;

public:	

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
};
