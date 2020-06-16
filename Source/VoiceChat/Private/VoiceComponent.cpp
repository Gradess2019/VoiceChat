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


UVoiceComponent::UVoiceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	Rate = 0.3f;

}

void UVoiceComponent::StartTcpServer()
{
	auto Listener = new FTcpListener(FIPv4Endpoint(FIPv4Address(127, 0, 0, 1), 8890));
	Listener->OnConnectionAccepted().BindUObject(this, &UVoiceComponent::ConnectionAccept);
	UE_LOG(LogTemp, Warning, TEXT("Server started"));
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
}

bool UVoiceComponent::ConnectionAccept(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	auto Data = TArray<uint8>();
	Data.SetNumUninitialized(2048);
	auto CDO = GetMutableDefault<UVoiceComponent>();
	
	auto BytesRead = 0;

	ClientSocket->Wait(ESocketWaitConditions::WaitForRead, FTimespan(0, 0, 3));
	auto bSuccess = ClientSocket->Recv(Data.GetData(), Data.Num(), BytesRead);

	Data.SetNum(BytesRead);
	
	if (BytesRead == 0)
	{
		Data.SetNum(1);
		Data[0] = 255;
	}
	
	if (bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server: Bytes read: %d; Data: %d"), BytesRead, Data[0]);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Error"));
	}

	auto TempRawSize = UVOIPStatics::GetMaxUncompressedVoiceDataSizePerChannel();
	auto TempDecodeBuffer = TArray<uint8>();
	TempDecodeBuffer.SetNumUninitialized(TempRawSize);
	VoiceDecoder->Decode(Data.GetData(), Data.Num(), TempDecodeBuffer.GetData(), TempRawSize);
	SoundWave->QueueAudio(TempDecodeBuffer.GetData(), TempRawSize);

	return true;
}

void UVoiceComponent::Send(const TArray<uint8>& InData)
{
	auto SocketDescription = FString("Test TCP server");
	auto Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, SocketDescription);
	
	auto BytesSent = 0;
	auto Address = FIPv4Endpoint(FIPv4Address(127, 0, 0, 1), 8890).ToInternetAddr();
	auto bConnected = Socket->Connect(Address.Get());

	UE_LOG(LogTemp, Warning, TEXT("Client: Sending %d to %s. Connected: %d"), InData[0], *Address.Get().ToString(true), bConnected);
	auto bResult = Socket->Send(InData.GetData(), InData.Num(), BytesSent);

	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	UE_LOG(LogTemp, Warning, TEXT("Client: Bytes sent: %d; Result: %d"), BytesSent, bResult);
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
	auto TempDecodeBuffer = TArray<uint8>();
	TempDecodeBuffer.SetNumUninitialized(TempRawSize);
	VoiceDecoder->Decode(InVoiceBuffer.GetData(), InVoiceBuffer.Num(), TempDecodeBuffer.GetData(), TempRawSize);
	SoundWave->QueueAudio(TempDecodeBuffer.GetData(), TempRawSize);
}

