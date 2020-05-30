// Fill out your copyright notice in the Description page of Project Settings.

#include "VoiceComponent.h"
#include "OnlineSubsystem.h"
#include "VoiceModule.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetArrayLibrary.h"
#include "OnlineSubsystemUtils.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UVoiceComponent::UVoiceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UVoiceComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<APawn>(GetOwner());

	if (!(IsValid(Owner) && Owner->IsLocallyControlled())) {
		PrimaryComponentTick.SetTickFunctionEnable(false);
	}
	else {
		VoiceCapture = FVoiceModule::Get().CreateVoiceCapture();
		VoiceEncoder = FVoiceModule::Get().CreateVoiceEncoder();
	}
	VoiceDecoder = FVoiceModule::Get().CreateVoiceDecoder();

	AudioComponent = Cast<UAudioComponent>(GetOwner()->GetComponentByClass(UAudioComponent::StaticClass()));

	SoundWave = NewObject<USoundWaveProcedural>();
	SoundWave->SetSampleRate(UVOIPStatics::GetVoiceSampleRate());
	SoundWave->NumChannels = 1;
	SoundWave->Duration = INDEFINITELY_LOOPING_DURATION;
	SoundWave->SoundGroup = SOUNDGROUP_Voice;
	SoundWave->bLooping = false;
	SoundWave->bProcedural = true;
	SoundWave->Volume = 1.f;

	AudioComponent->SetSound(SoundWave);
	AudioComponent->Play();

	Owner->GetWorldTimerManager().SetTimer(
		PlayVoiceCaptureTimer,
		this,
		&UVoiceComponent::PlayVoiceCapture,
		0.2f,
		true
	);
}

void UVoiceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	VoiceCaptureTick();
}

void UVoiceComponent::Start()
{
	VoiceCapture->Start();
}

void UVoiceComponent::Stop()
{
	VoiceCapture->Stop();
}

void UVoiceComponent::VoiceCaptureTick_Implementation()
{
	if (!VoiceCapture.IsValid()) { return; }

	uint32 AvailableBytes = 0;
	auto CaptureState = VoiceCapture->GetCaptureState(AvailableBytes);

	VoiceCaptureBuffer.Reset();

	if (CaptureState == EVoiceCaptureState::Ok && AvailableBytes > 0)
	{
		short VoiceCaptureSample = 0;
		uint32 VoiceCaptureReadBytes = 0;
		float VoiceCaptureTotalSquared = 0;

		VoiceCaptureBuffer.SetNumUninitialized(AvailableBytes);
		VoiceCapture->GetVoiceData(
			VoiceCaptureBuffer.GetData(),
			AvailableBytes,
			VoiceCaptureReadBytes
		);

		for (uint32 i = 0; i < (VoiceCaptureReadBytes / 2); i++)
		{
			VoiceCaptureSample = (VoiceCaptureBuffer[i * 2 + 1] << 8) | VoiceCaptureBuffer[i * 2];
			VoiceCaptureTotalSquared += float(VoiceCaptureSample) * float(VoiceCaptureSample);
		}

		float VoiceCaptureMeanSquare = (2 * (VoiceCaptureTotalSquared / VoiceCaptureBuffer.Num()));
		float VoiceCaptureRms = FMath::Sqrt(VoiceCaptureMeanSquare);
		float VoiceCaptureFinalVolume = ((VoiceCaptureRms / 32768.0) * 200.f);

		VoiceCaptureVolume = VoiceCaptureFinalVolume;

		auto TempCompressedSize = UVOIPStatics::GetMaxCompressedVoiceDataSize();
		TempBuffer.SetNumUninitialized(TempCompressedSize);

		VoiceEncoder->Encode(VoiceCaptureBuffer.GetData(), VoiceCaptureReadBytes, TempBuffer.GetData(), TempCompressedSize);

		TempBuffer.SetNum(TempCompressedSize);
		SetBuffer(TempBuffer);
	}
}

void UVoiceComponent::PlayVoiceCapture_Implementation()
{
	if (AudioComponent->IsPlaying()) { return; }

	AudioComponent->Play();
}

bool UVoiceComponent::SetBuffer_Validate(const TArray<uint8>& InReplicatedBuffer)
{
	return true;
}

void UVoiceComponent::SetBuffer_Implementation(const TArray<uint8>& InVoiceBuffer)
{
	SetBuffer_Multicast(InVoiceBuffer);
}

void UVoiceComponent::SetBuffer_Multicast_Implementation(const TArray<uint8>& InVoiceBuffer)
{
	if (Owner == nullptr || Owner->IsLocallyControlled()) { return; }

	auto TempRawSize = UVOIPStatics::GetMaxUncompressedVoiceDataSizePerChannel();
	TempBuffer.SetNumUninitialized(TempRawSize);
	VoiceDecoder->Decode(InVoiceBuffer.GetData(), InVoiceBuffer.Num(), TempBuffer.GetData(), TempRawSize);

	SoundWave->QueueAudio(TempBuffer.GetData(), TempRawSize);
}

