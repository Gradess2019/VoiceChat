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
	if (VoiceCapture) { return; }

	UE_LOG(LogVoice, Error, TEXT("Voice capture is null! File: %s; Function: %s; Line: %s; "), __FILE__, __FUNCTION__, __LINE__);
	DestroyComponent();
}

void UVoiceCaptureExtended::InitVoiceCaptureWithParams_Implementation(const FString& InDeviceName, int32 InSampleRate, int32 InNumChannels)
{
	InitProperties(InDeviceName, InSampleRate, InNumChannels);
	InitVoiceCapture();
}

void UVoiceCaptureExtended::InitProperties_Implementation(const FString& InDeviceName, int32 InSampleRate, int32 InNumChannels)
{
	DeviceName = InDeviceName;
	SampleRate = InSampleRate;
	NumChannels = InNumChannels;
}

bool UVoiceCaptureExtended::Start_Implementation()
{
	return VoiceCapture->Start();
}

void UVoiceCaptureExtended::Stop_Implementation()
{
	return VoiceCapture->Stop();
}

void UVoiceCaptureExtended::Shutdown_Implementation()
{
	VoiceCapture->Shutdown();
}

bool UVoiceCaptureExtended::ChangeDevice_Implementation(const FString& InDeviceName, int32 InSampleRate, int32 InNumChannels)
{
	InitProperties(InDeviceName, InSampleRate, InNumChannels);
	auto bResult = VoiceCapture->ChangeDevice(DeviceName, SampleRate, NumChannels);
	if (!bResult)
	{
		UE_LOG(LogVoice, Error, TEXT("Changed voice capture is null! File: %s; Function: %s; Line: %s; "), __FILE__, __FUNCTION__, __LINE__);
		DestroyComponent();
	}
	
	return bResult;
}

bool UVoiceCaptureExtended::IsCapturing_Implementation()
{
	return VoiceCapture->IsCapturing();
}

void UVoiceCaptureExtended::GetCaptureState_Implementation(int64& OutAvailableVoiceData, EVoiceCaptureExtendedState& bOutResult)
{
	auto bResult = VoiceCapture->GetCaptureState(reinterpret_cast<uint32&>(OutAvailableVoiceData));
	bOutResult = static_cast<EVoiceCaptureExtendedState>(bResult);
}
