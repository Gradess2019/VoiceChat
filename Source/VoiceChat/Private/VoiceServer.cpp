// Fill out your copyright notice in the Description page of Project Settings.


#include "VoiceServer.h"
#include "Common/TcpListener.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "VoiceChat.h"
#include "VoiceModule.h"
#include "VoiceServerThread.h"

int UVoiceServer::GetDefaultPort()
{
	auto Port = 0;
	GConfig->GetInt(
		TEXT("VoiceServer"),
		TEXT("DefaultPort"),
		Port,
		FVoiceChatModule::GetConfigPath()
	);
	return Port;
}

void UVoiceServer::InitIP()
{
	auto bCanBindAll = false;
	auto AddressPtr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);
	IP = AddressPtr->ToString(false) + ":" + FString::FromInt(GetDefaultPort());
}

void UVoiceServer::StartTCPServer()
{
	InitIP();
	FIPv4Endpoint Endpoint;
	FIPv4Endpoint::Parse(IP, Endpoint);
	Listener = MakeShared<FTcpListener>(Endpoint);
	Listener->OnConnectionAccepted().BindUObject(this, &UVoiceServer::ConnectionAccept);
	if (Listener->GetSocket()) {

		int32 OutRecieveBufferSize = 0;

		Listener->GetSocket()->SetReceiveBufferSize(SERVER_RECEIVE_BUFFER_SIZE, OutRecieveBufferSize);

		UE_LOG(LogVoice, Display, TEXT("Server started"
			"\n\tLocal IP: %s"
			"\n\tRecieve buffer size: %d"),
			*Listener.Get()->GetLocalEndpoint().ToString(),
			OutRecieveBufferSize
		);
	}
}

bool UVoiceServer::ConnectionAccept(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	RegisteredSockets.Add(ClientEndpoint, MakeShareable(ClientSocket));
	if (!ServerThread)
	{
		ServerThread = MakeShared<FVoiceServerThread>(this);
	}
	return true;
}

bool UVoiceServer::CheckSockets()
{
	if (RegisteredSockets.Num() <= 1) { return false; }
	auto AvailableSockets = TMap<FIPv4Endpoint, TSharedPtr<FSocket, ESPMode::ThreadSafe>>(RegisteredSockets);
	
	TArray<FIPv4Endpoint> Keys;
	AvailableSockets.GenerateKeyArray(Keys);

	for (auto CurrentClientEndpoint : Keys)
	{
		auto CurrentClientSocket = AvailableSockets.Find(CurrentClientEndpoint)->Get();

		auto Data = TArray<uint8>();
		Data.SetNumUninitialized(MAX_VOICE_PACKAGE_SIZE);
		//CurrentClientSocket->Wait(ESocketWaitConditions::WaitForRead, WAIT_ONE_RATE);

		auto BytesRead = 0;
		auto bSuccess = CurrentClientSocket->Recv(Data.GetData(), Data.Num(), BytesRead);
		Data.SetNum(BytesRead);

		if (BytesRead > 0)
		{
			UE_LOG(LogVoice, Display, TEXT("Server: Bytes read: %d; Data: %d"), BytesRead, Data[0]);
			for (auto TargetClientEndpoint : Keys)
			{
				if (TargetClientEndpoint == CurrentClientEndpoint) { continue; }

				auto TargetClient = AvailableSockets.Find(TargetClientEndpoint)->Get();
				auto BytesSent = 0;

				TargetClient->SendTo(Data.GetData(), Data.Num(), BytesSent, TargetClientEndpoint.ToInternetAddr().Get());
			}
		}
	}

	return true;
}

