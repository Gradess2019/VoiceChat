// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoiceModule.h"
#include "VoiceCaptureExtended.generated.h"

class UActorComponent;
class IVoiceCapture;
class UVOIPStatics;

/**
 * Class that exposed voice capture functionality in blueprint
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Voice Capture"))
class VOICECHAT_API UVoiceCaptureExtended : public UActorComponent
{
	GENERATED_BODY()

public:

	UVoiceCaptureExtended();

	/** Initialize voice capture object*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voice")
	void InitVoiceCapture();
	virtual void InitVoiceCapture_Implementation();

	/** Initialize voice capture object using params*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voice")
	void InitVoiceCaptureWithParams(
		UPARAM(meta = (DisplayName = "Device Name")) const FString& InDeviceName = "Default Device",
		UPARAM(meta = (DisplayName = "Sample Rate")) int32 InSampleRate = 16000,
		UPARAM(meta = (DisplayName = "Num Channels")) int32 InNumChannels = 1
	);
	virtual void InitVoiceCaptureWithParams_Implementation(const FString& InDeviceName,	int32 InSampleRate,	int32 InNumChannels);

protected:

	/** Voice capture object to interact with*/
	TSharedPtr<IVoiceCapture> VoiceCapture;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voice")
	bool bAutoInitialization;

	/** Name of device to capture audio data with*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voice")
	FString DeviceName;

	/** Sampling rate of voice capture*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voice")
	int32 SampleRate;

	/** Number of channels to capture*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voice")
	int32 NumChannels;

	UFUNCTION()
	virtual void BeginPlay() override;
};
