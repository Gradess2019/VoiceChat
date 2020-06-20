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
#include "LocalVoiceHandler.h"
#include "RemoteVoiceHandler.h"

UVoiceComponent::UVoiceComponent()
{
	Address = FString("127.0.0.1:7778");
	Rate = 0.3f;
}

void UVoiceComponent::BeginPlay()
{
	Super::BeginPlay();
	Owner = Cast<APawn>(GetOwner());

	if (!IsValid(Owner) && Owner->IsLocallyControlled())
	{
		DestroyComponent();
		return;
	}
	
	VoiceCapture = FVoiceModule::Get().CreateVoiceCapture();
	VoiceEncoder = FVoiceModule::Get().CreateVoiceEncoder();
	VoiceDecoder = FVoiceModule::Get().CreateVoiceDecoder();
	InitRemoteVoiceHandler();

	CreateSocket();
	if (!ConnectToVoiceServer())
	{
		SetConnectToServerTimer();
	}
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

void UVoiceComponent::InitRemoteVoiceHandler()
{
	SelfPtr = MakeShareable(this);
	RemoteVoiceHandler = MakeShared<FRemoteVoiceHandler>(SelfPtr, Rate);

	if (!RemoteVoiceHandler.IsValid())
	{
		UE_LOG(LogVoice, Error, TEXT("Could not initialize RemoteVoiceHandler"));
		return;
	}

	UE_LOG(LogVoice, Display, TEXT("Voice handlers were initialized successfully"));
}

void UVoiceComponent::CreateSocket()
{
	auto SocketDescription = FString("Client TCP socket");
	ClientSocket = MakeShareable<FSocket>(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, SocketDescription));
	if (!ClientSocket.IsValid())
	{
		UE_LOG(LogVoice, Error, TEXT("Could not create socket. Func: %s, Line: %s"), *FString(__FUNCTION__), *FString::FromInt(__LINE__));
		return;
	}
	ClientSocket.Get()->SetNonBlocking();
	UE_LOG(LogVoice, Display, TEXT("Client socket was created successfully."));
}

bool UVoiceComponent::ConnectToVoiceServer()
{
	if (!ClientSocket.IsValid()) { return false; }
	if (ClientSocket.Get()->GetConnectionState() == ESocketConnectionState::SCS_Connected) { return true; }

	FIPv4Endpoint Endpoint;
	FIPv4Endpoint::Parse(Address, Endpoint);

	auto Addr = FIPv4Endpoint(Endpoint).ToInternetAddr();
	auto bConnected = ClientSocket.Get()->Connect(Addr.Get());

	ClientSocket.Get()->SetReuseAddr();

	UE_LOG(LogVoice, Display, TEXT("Try to connect to %s; Connected: %d"), *Addr->ToString(true), bConnected);

	if (bConnected && RemoteVoiceHandler.IsValid())
	{
		RemoteVoiceHandler->Start();
		Owner->GetWorldTimerManager().ClearTimer(ConnectToServerTimer);
	}

	return bConnected;
}

void UVoiceComponent::SetConnectToServerTimer()
{
	FTimerDelegate ConnectToServerDelegate;
	ConnectToServerDelegate.BindUFunction(this, FName("ConnectToVoiceServer"));
	Owner->GetWorldTimerManager().SetTimer(
		ConnectToServerTimer,
		0.5f,
		true
	);
}

bool UVoiceComponent::CheckSocketConnection(bool bInTryToConnect)
{
	if (!ClientSocket.IsValid()) { return false; }
	return bInTryToConnect ? ConnectToVoiceServer() : ClientSocket->GetConnectionState() == ESocketConnectionState::SCS_Connected;
}

void UVoiceComponent::Send(const TArray<uint8>& InData)
{
	if (!SocketConnected()) { return; }

	auto BytesSent = 0;
	UE_LOG(LogTemp, Warning, TEXT("Client: Sending %d"), InData[0]);
	auto bResult = ClientSocket.Get()->Send(InData.GetData(), InData.Num(), BytesSent);
	UE_LOG(LogTemp, Warning, TEXT("Client: Bytes sent: %d; Result: %d"), BytesSent, bResult);
}

bool UVoiceComponent::CheckSocket(bool bInTryToCreate)
{
	if (!ClientSocket.IsValid() && bInTryToCreate)
	{
		CreateSocket();
	}

	return ClientSocket.IsValid();
}

void UVoiceComponent::ReceiveVoiceData()
{
	if (!SocketConnected()) { return; }

	uint32 PendingDataSize = 0;
	if (ClientSocket.Get()->HasPendingData(PendingDataSize))
	{
		if (PendingDataSize < 0) { return; }
	}

	auto Data = TArray<uint8>();
	auto BytesRead = 0;

	Data.SetNumUninitialized(MAX_VOICE_PACKAGE_SIZE);
	ClientSocket.Get()->Recv(Data.GetData(), Data.Num(), BytesRead);

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

bool UVoiceComponent::SocketConnected(bool bInTryToConnect)
{
	return CheckSocket(bInTryToConnect) && CheckSocketConnection(bInTryToConnect);
}

void UVoiceComponent::Start()
{
	if (!VoiceCapture.IsValid()) { return; }

	LocalVoiceHandler = MakeShared<FLocalVoiceHandler>(SelfPtr, Rate);
	if (!LocalVoiceHandler.IsValid())
	{
		UE_LOG(LogVoice, Error, TEXT("LocalVoiceHandler is null! Func: %s; Line: %s"), *FString(__FUNCTION__), *FString::FromInt(__LINE__));
		return;
	}

	VoiceCapture->Start();
	bCapturing = true;
	LocalVoiceHandler->Start();

}

void UVoiceComponent::Stop()
{
	if (LocalVoiceHandler.IsValid())
	{
		LocalVoiceHandler->Stop();
	}

	if (!VoiceCapture.IsValid()) { return; }
	VoiceCapture->Stop();
	bCapturing = false;
}

void UVoiceComponent::CaptureAndSendVoiceData_Implementation()
{
	CaptureVoice();

	if (ReplicatedBuffer.Num() > 0)
	{
		Send(ReplicatedBuffer);

		ReplicatedBuffer.Reset();
	}

	if (AudioComponent->IsPlaying()) { return; }

	AudioComponent->Play();
}

void UVoiceComponent::CaptureVoice_Implementation()
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