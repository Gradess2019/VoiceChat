// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoiceChatExtras.h"
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

	/**
	 * Initialize voice capture object
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voice")
	void InitVoiceCapture();
	virtual void InitVoiceCapture_Implementation();

	/**
	 * Initialize voice capture properties
	 * 
	 * @param InDeviceName name of device to capture audio data with, empty for default device
	 * @param InSampleRate sampling rate of voice capture
	 * @param InNumChannels number of channels to capture
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voice")
	void InitProperties(const FString& InDeviceName, int32 InSampleRate, int32 InNumChannels);
	virtual void InitProperties_Implementation(const FString& InDeviceName, int32 InSampleRate, int32 InNumChannels);

	/**
	 * Initialize voice capture object using params
	 * 
	 * @param InDeviceName name of device to capture audio data with, empty for default device
	 * @param InSampleRate sampling rate of voice capture
	 * @param InNumChannels number of channels to capture
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voice")
	void InitVoiceCaptureWithParams(
		UPARAM(meta = (DisplayName = "Device Name")) const FString& InDeviceName = "Default Device",
		UPARAM(meta = (DisplayName = "Sample Rate")) int32 InSampleRate = 16000,
		UPARAM(meta = (DisplayName = "Num Channels")) int32 InNumChannels = 1
	);
	virtual void InitVoiceCaptureWithParams_Implementation(const FString& InDeviceName,	int32 InSampleRate,	int32 InNumChannels);

	/**
	 * Start capturing voice
	 *
	 * @return true if successfully started, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voice")
	bool Start();
	virtual bool Start_Implementation();

	/** 
	 * Stop capturing voice
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voice")
	void Stop();
	virtual void Stop_Implementation();

	/**
	 * Shutdown the voice capture object
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voice")
	void Shutdown();
	virtual void Shutdown_Implementation();

	/**
	 * Change the associated capture device
	 *
	 * @param InDeviceName name of device to capture audio data with, empty for default device
	 * @param InSampleRate sampling rate of voice capture
	 * @param InNumChannels number of channels to capture
	 *
	 * @return true if change was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voice")
	bool ChangeDevice(const FString& InDeviceName, int32 InSampleRate, int32 InNumChannels);
	virtual bool ChangeDevice_Implementation(const FString& InDeviceName, int32 InSampleRate, int32 InNumChannels);

	/**
	 * Is the voice capture object actively capturing
	 *
	 * @return true if voice is being captured, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voice")
	bool IsCapturing();
	virtual bool IsCapturing_Implementation();

	/**
	 * Return the state of the voice data and its availability
	 *
	 * @param OutAvailableVoiceData size, in bytes, of available voice data
	 * @param bOutResult state of the voice capture buffer
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voice")
	void GetCaptureState(
		UPARAM(meta = (DisplayName = "AvailableVoiceData")) int64& OutAvailableVoiceData, 
		UPARAM(meta = (DisplayName = "Result")) EVoiceCaptureExtendedState& bOutResult
	);
	virtual void GetCaptureState_Implementation(int64& OutAvailableVoiceData, EVoiceCaptureExtendedState& bOutResult);
	
protected:

	/**
	 * Voice capture object to interact with
	 */
	TSharedPtr<IVoiceCapture> VoiceCapture;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voice")
	bool bAutoInitialization;

	/**
	 * Name of device to capture audio data with
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voice")
	FString DeviceName;

	/**
	 * Sampling rate of voice capture
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voice")
	int32 SampleRate;

	/**
	 * Number of channels to capture
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voice")
	int32 NumChannels;

	UFUNCTION()
	virtual void BeginPlay() override;
};
