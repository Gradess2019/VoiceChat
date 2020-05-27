// Fill out your copyright notice in the Description page of Project Settings.

#include "VoiceComponent.h"
#include "OnlineSubsystem.h"
#include "VoiceModule.h"
#include <typeinfo>
#include "Kismet/KismetSystemLibrary.h"
#include "OnlineSubsystemUtils.h"

UVoiceComponent::UVoiceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVoiceComponent::BeginPlay()
{
	Super::BeginPlay();

	VoiceCapture = FVoiceModule::Get().CreateVoiceCapture();
	Encoder = FVoiceModule::Get().CreateVoiceEncoder();
	Decoder = FVoiceModule::Get().CreateVoiceDecoder();

	SynthComponent = CreateVoiceSynthComponent(UVOIPStatics::GetVoiceSampleRate());
	SynthComponent->SetVolumeMultiplier(3.f);
	SynthComponent->OpenPacketStream(128000, UVOIPStatics::GetNumBufferedPackets(), UVOIPStatics::GetBufferingDelay());
}

void UVoiceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	//if (!VoiceCapture->IsCapturing()) { return; }

	//DecompressedBuffer.Empty();
	
	uint32 NewVoiceDataBytes = 0;
	uint32 ByteWritten = 0;
	uint64 NewSampleCount = 0;
	
	auto State = VoiceCapture->GetCaptureState(NewVoiceDataBytes);
	DecompressedBuffer.AddUninitialized(NewVoiceDataBytes);

	auto AvailableSamples = VoiceCapture->GetMicrophoneAudio(4096 * 2, 1.f)->GetNumSamplesAvailable();
	VoiceCapture->GetVoiceData(DecompressedBuffer.GetData(), NewVoiceDataBytes, ByteWritten, NewSampleCount);

	if (NewSampleCount != 0)
	{
		UE_LOG(LogTemp,
			Warning,
			TEXT("NewBytes = %d, WrittenBytes = %d; Samples = %d, AvailableSamples = %d"),
			NewVoiceDataBytes,
			ByteWritten,
			NewSampleCount,
			AvailableSamples
		);
		SynthComponent->ResetBuffer(NewSampleCount, UVOIPStatics::GetBufferingDelay());
		SynthComponent->Start();
		SynthComponent->SubmitPacket(
			(float*)DecompressedBuffer.GetData(),
			UVOIPStatics::GetMaxUncompressedVoiceDataSizePerChannel(),
			NewSampleCount,
			EVoipStreamDataFormat::Int16
		);
	}

	if (SynthComponent->IsIdling())
	{
		SynthComponent->Stop();
	}
}

void UVoiceComponent::Start()
{
	if (VoiceCapture->IsCapturing()) { return; }

	VoiceCapture->Start();
}

void UVoiceComponent::Stop()
{
	if (!VoiceCapture->IsCapturing()) { return; }

	VoiceCapture->Stop();
}

