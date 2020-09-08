 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundWaveProcedural.h"
#include "SoundWaveProceduralExtended.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class VOICECHAT_API USoundWaveProceduralExtended : public USoundWaveProcedural
{
	GENERATED_BODY()

public:

	/** Add data to the FIFO that feeds the audio device. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voice", meta = (DisplayName = "Queue Audio"))
	void QueueAudio_Extended(UPARAM(ref) TArray<uint8>& AudioData, int32 BufferSize);
	virtual void QueueAudio_Extended_Implementation(TArray<uint8>& AudioData, int32 BufferSize);

	/** Remove all queued data from the FIFO. This is only necessary if you want to start over, or GeneratePCMData() isn't going to be called, since that will eventually drain it. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voice", meta = (DisplayName = "Reset Audio"))
	void ResetAudio_Extended();
	virtual void ResetAudio_Extended_Implementation();
	
	/** Query bytes queued for playback */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "Voice", meta = (DisplayName = "Get Available Audio Byte Count"))
	int32 GetAvailableAudioByteCount_Extended();
	virtual int32 GetAvailableAudioByteCount_Extended_Implementation();

	/** Size in bytes of a single sample of audio in the procedural audio buffer. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "Voice", meta = (DisplayName = "Get Sample Byte Size"))
	int32 GetSampleByteSize_Extended();
	virtual int32 GetSampleByteSize_Extended_Implementation();
	
	// Number of samples to pad with 0 if there isn't enough audio available
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "Voice", meta = (DisplayName = "Get Num Buffer Underrun Samples"))
	int32 GetNumBufferUnderrunSamples_Extended();
	virtual int32 GetNumBufferUnderrunSamples_Extended_Implementation();
	
	// The number of PCM samples we want to generate. This can't be larger than SamplesNeeded in GeneratePCMData callback, but can be less.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "Voice", meta = (DisplayName = "Get Num Samples To Generate Per Callback"))
	int32 GetNumSamplesToGeneratePerCallback_Extended();
	virtual int32 GetNumSamplesToGeneratePerCallback_Extended_Implementation();

};
