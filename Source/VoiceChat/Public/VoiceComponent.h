// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoiceComponent.generated.h"

class IVoiceCapture;
class IVoiceDecoder;
class IVoiceEncoder;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VOICECHAT_API UVoiceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVoiceComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	IVoiceCapture* VoiceCapture;
	
	UPROPERTY()
	IVoiceEncoder* Encoder;

	UPROPERTY()
	IVoiceDecoder* Decoder;

public:	

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
};
