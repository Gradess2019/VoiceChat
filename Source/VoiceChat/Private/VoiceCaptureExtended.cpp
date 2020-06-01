// Fill out your copyright notice in the Description page of Project Settings.


#include "VoiceCaptureExtended.h"
#include "VoiceModule.h"

UVoiceCaptureExtended::UVoiceCaptureExtended()
{
	bAutoInitialization = true;
	DeviceName = DEFAULT_DEVICE_NAME;
	SampleRate = UVOIPStatics::GetVoiceSampleRate();
	NumChannels = DEFAULT_NUM_VOICE_CHANNELS;
}

void UVoiceCaptureExtended::BeginPlay()
{
	Super::BeginPlay();

	if (!bAutoInitialization) { return; }
	InitVoiceCapture();
}

void UVoiceCaptureExtended::InitVoiceCapture_Implementation()
{
	VoiceCapture = FVoiceModule::Get().CreateVoiceCapture(DeviceName, SampleRate, NumChannels);
}

void UVoiceCaptureExtended::InitVoiceCaptureWithParams_Implementation(const FString& InDeviceName, int32 InSampleRate, int32 InNumChannels)
{
	DeviceName = InDeviceName;
	SampleRate = InSampleRate;
	NumChannels = InNumChannels;

	InitVoiceCapture();
}
