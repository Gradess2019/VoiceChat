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

	RootComponent = GetOwner()->GetRootComponent();

	VoiceCapture = FVoiceModule::Get().CreateVoiceCapture();

	AudioComponent = NewObject<UAudioComponent>();
	AudioComponent->AttachTo(RootComponent);
	AudioComponent->bAutoActivate = true;
	AudioComponent->Activate(true);
	AudioComponent->bAlwaysPlay = true;
	AudioComponent->PitchMultiplier = 0.85f;
	AudioComponent->VolumeMultiplier = 5.f;

	SoundWave = NewObject<USoundWaveProcedural>();
	SoundWave->SetSampleRate(22050);
	SoundWave->NumChannels = 1;
	SoundWave->
	SoundWave->Duration = INDEFINITELY_LOOPING_DURATION;
	SoundWave->SoundGroup = SOUNDGROUP_Voice;
	SoundWave->bLooping = false;
	SoundWave->bProcedural = true;
	SoundWave->Pitch = 0.85f;
	SoundWave->Volume = 5.f;

	GetOwner()->GetWorldTimerManager().SetTimer(
		PlayVoiceCaptureTimer,
		this,
		&UVoiceComponent::PlayVoiceCapture,
		0.1f,
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
	EVoiceCaptureState::Type CaptureState = VoiceCapture->GetCaptureState(AvailableBytes);

	VoiceCaptureBuffer.Reset();
	PlayVoiceCaptureFlag = false;

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
			VoiceCaptureTotalSquared += ((float)VoiceCaptureSample * (float)VoiceCaptureSample);
		}

		float VoiceCaptureMeanSquare = (2 * (VoiceCaptureTotalSquared / VoiceCaptureBuffer.Num()));
		float VoiceCaptureRms = FMath::Sqrt(VoiceCaptureMeanSquare);
		float VoiceCaptureFinalVolume = ((VoiceCaptureRms / 32768.0) * 200.f);

		VoiceCaptureVolume = VoiceCaptureFinalVolume;

		SoundWave->QueueAudio(VoiceCaptureBuffer.GetData(), VoiceCaptureReadBytes);
		AudioComponent->SetSound(SoundWave);

		PlayVoiceCaptureFlag = true;
	}

}

void UVoiceComponent::PlayVoiceCapture_Implementation()
{
	if (!PlayVoiceCaptureFlag)
	{
		AudioComponent->FadeOut(0.3f, 0.f);
		return;
	}

	if (AudioComponent->IsPlaying()) { return; }

	AudioComponent->Play();
}