// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundWaveProceduralExtended.h"

#include "Components/SynthComponent.h"

void USoundWaveProceduralExtended::QueueAudio_Extended_Implementation(TArray<uint8>& AudioData, int32 BufferSize)
{
	QueueAudio(AudioData.GetData(), BufferSize);
}

void USoundWaveProceduralExtended::ResetAudio_Extended_Implementation()
{
	ResetAudio();
}

int32 USoundWaveProceduralExtended::GetAvailableAudioByteCount_Extended_Implementation()
{
	return GetAvailableAudioByteCount();
}

int32 USoundWaveProceduralExtended::GetSampleByteSize_Extended_Implementation()
{
	return SampleByteSize;
}

int32 USoundWaveProceduralExtended::GetNumBufferUnderrunSamples_Extended_Implementation()
{
	return NumBufferUnderrunSamples;
}

int32 USoundWaveProceduralExtended::GetNumSamplesToGeneratePerCallback_Extended_Implementation()
{
	return NumSamplesToGeneratePerCallback;
}


