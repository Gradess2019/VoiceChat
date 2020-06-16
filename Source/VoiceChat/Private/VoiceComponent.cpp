// Fill out your copyright notice in the Description page of Project Settings.

#include "VoiceComponent.h"

#include "OnlineSubsystem.h"
#include "VoiceModule.h"
#include "Kismet/KismetSystemLibrary.h"
#include "OnlineSubsystemUtils.h"
#include "SocketSubsystem.h"
#include "TimerManager.h"
#include "Common/TcpListener.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetArrayLibrary.h"
#include "VoiceServer.h"
#include "Engine/NetConnection.h"

UVoiceComponent::UVoiceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	Address = FString("127.0.0.1:7778");
	Rate = 0.3f;

	auto SocketDescription = FString("Test TCP server");
	ClientSocket = MakeShareable<FSocket>(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, SocketDescription));
	ClientSocket->SetNonBlocking();
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
		&UVoiceComponent::RecieveVoiceData,
		Rate,
		true
	);

}

void UVoiceComponent::Send(const TArray<uint8>& InData)
{
	auto SocketDescription = FString("Test TCP server");
	if (ClientSocket->GetConnectionState() == ESocketConnectionState::SCS_NotConnected)
	{
		FIPv4Endpoint Endpoint;
		FIPv4Endpoint::Parse(Address, Endpoint);
		auto Addr = FIPv4Endpoint(Endpoint).ToInternetAddr();
		auto bConnected = ClientSocket->Connect(Addr.Get());
		ClientSocket->SetReuseAddr();
		UE_LOG(LogTemp, Warning, TEXT("Client: Connect to %s. Connected: %d"), *Addr.Get().ToString(true), bConnected);
	}

	auto BytesSent = 0;
	UE_LOG(LogTemp, Warning, TEXT("Client: Sending %d"), InData[0]);
	auto bResult = ClientSocket->Send(InData.GetData(), InData.Num(), BytesSent);
	UE_LOG(LogTemp, Warning, TEXT("Client: Bytes sent: %d; Result: %d"), BytesSent, bResult);
}

void UVoiceComponent::RecieveVoiceData()
{
	if (ClientSocket->GetConnectionState() == ESocketConnectionState::SCS_NotConnected) { return; }
	uint32 PendingDataSize = 0;
	if (ClientSocket.Get()->HasPendingData(PendingDataSize))
	{
		if (PendingDataSize < 0) { return; }
	}
	auto Data = TArray<uint8>();
	auto BytesRead = 0;

	Data.SetNumUninitialized(MAX_VOICE_PACKAGE_SIZE);
	//ClientSocket->Wait(ESocketWaitConditions::WaitForRead, WAIT_ONE_RATE);
	ClientSocket->Recv(Data.GetData(), Data.Num(), BytesRead);

	if (BytesRead > 0)
	{
		Data.SetNum(BytesRead);
		UE_LOG(LogTemp, Warning, TEXT("Client: Bytes got: %d; Data: %d"), BytesRead, Data[0]);
		auto TempRawSize = UVOIPStatics::GetMaxUncompressedVoiceDataSizePerChannel();
		auto TempDecodeBuffer = TArray<uint8>();
		TempDecodeBuffer.SetNumUninitialized(TempRawSize);
		VoiceDecoder->Decode(Data.GetData(), Data.Num(), TempDecodeBuffer.GetData(), TempRawSize);
		SoundWave->QueueAudio(TempDecodeBuffer.GetData(), TempRawSize);
	}
}

void UVoiceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bCapturing = VoiceCapture->IsCapturing();
}

void UVoiceComponent::Start()
{
	VoiceCapture->Start();

	Owner->GetWorldTimerManager().SetTimer(
		PlayVoiceCaptureTimer,
		this,
		&UVoiceComponent::PlayVoiceCapture,
		Rate,
		true
	);
}

void UVoiceComponent::Stop()
{
	VoiceCapture->Stop();
	if (!PlayVoiceCaptureTimer.IsValid()) { return; }

	PlayVoiceCaptureTimer.Invalidate();
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

		float VoiceCaptureMeanSquare = 2 * (VoiceCaptureTotalSquared / VoiceCaptureBuffer.Num());
		float VoiceCaptureRms = FMath::Sqrt(VoiceCaptureMeanSquare);
		float VoiceCaptureFinalVolume = VoiceCaptureRms / 32768.0 * 200.f;

		VoiceCaptureVolume = VoiceCaptureFinalVolume;

		auto TempEncodeBuffer = TArray<uint8>();
		auto CompressedSize = UVOIPStatics::GetMaxCompressedVoiceDataSize();
		TempEncodeBuffer.SetNumUninitialized(CompressedSize);
		VoiceEncoder->Encode(VoiceCaptureBuffer.GetData(), VoiceCaptureReadBytes, TempEncodeBuffer.GetData(), CompressedSize);
		TempEncodeBuffer.SetNum(CompressedSize);
		ReplicatedBuffer.Append(TempEncodeBuffer);
	}
}

void UVoiceComponent::PlayVoiceCapture_Implementation()
{
	VoiceCaptureTick();

	if (ReplicatedBuffer.Num() > 0)
	{
		//SetBuffer(ReplicatedBuffer);
		Send(ReplicatedBuffer);

		ReplicatedBuffer.Reset();
	}

	if (AudioComponent->IsPlaying()) { return; }

	AudioComponent->Play();
}

void UVoiceComponent::SetBuffer_Multicast_Implementation(const TArray<uint8>& InVoiceBuffer)
{
	if (Owner == nullptr || Owner->IsLocallyControlled()) { return; }

	auto TempRawSize = UVOIPStatics::GetMaxUncompressedVoiceDataSizePerChannel();
	auto TempDecodeBuffer = TArray<uint8>();
	TempDecodeBuffer.SetNumUninitialized(TempRawSize);
	VoiceDecoder->Decode(InVoiceBuffer.GetData(), InVoiceBuffer.Num(), TempDecodeBuffer.GetData(), TempRawSize);
	SoundWave->QueueAudio(TempDecodeBuffer.GetData(), TempRawSize);
}

