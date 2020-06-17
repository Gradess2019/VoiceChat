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

FString UVoiceServer::GetDefaultAddress()
{
	auto Address = FString();
	GConfig->GetString(
		TEXT("VoiceServer"),
		TEXT("DefaultAddress"),
		Address,
		FVoiceChatModule::GetConfigPath()
	);
	return Address;
}

FString UVoiceServer::GetDefaultIP()
{
	return GetDefaultAddress() + ":" + FString::FromInt(GetDefaultPort());
}

bool UVoiceServer::InitIP(bool bInLan)
{
	if (bInLan)
	{
		IP = GetDefaultIP();
		StartTCPServer();
		return true;
	}

	auto Http = &FHttpModule::Get();

	if (!Http)
	{
		return false;
	}

	if (!Http->IsHttpEnabled())
	{
		return false;
	}

	auto TargetHost = FString("http://api.ipify.org");
	auto Request = Http->CreateRequest();
	
	Request->SetVerb("GET");
	Request->SetURL(TargetHost);
	Request->SetHeader("User-Agent", "VictoryBPLibrary/1.0");
	Request->SetHeader("Content-Type", "text/html");

	Request->OnProcessRequestComplete().BindUObject(this, &UVoiceServer::OnResponseReceived);
	if (!Request->ProcessRequest())
	{
		return false;
	}

	return true;
}

void UVoiceServer::StartTCPServer()
{
	FIPv4Endpoint Endpoint;
	FIPv4Endpoint::Parse("192.168.1.70:8930", Endpoint);
	Listener = MakeShared<FTcpListener>(Endpoint);
	Listener->OnConnectionAccepted().BindUObject(this, &UVoiceServer::ConnectionAccept);

	if (Listener->GetSocket()) {

		int32 OutRecieveBufferSize = 0;
		int32 OutSendBufferSize = 0;

		//Listener->GetSocket()->SetReceiveBufferSize(SERVER_RECEIVE_BUFFER_SIZE, OutRecieveBufferSize);
		//Listener->GetSocket()->SetSendBufferSize(SERVER_SEND_BUFFER_SIZE, OutRecieveBufferSize);

		UE_LOG(LogVoice, Display, TEXT("Server started\n"
			"\tRecieve buffer size: %d\n"
			"\tSend buffer size: %d"),
			OutRecieveBufferSize,
			OutSendBufferSize
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

	TArray<FIPv4Endpoint> Keys;
	RegisteredSockets.GenerateKeyArray(Keys);

	for (auto CurrentClientEndpoint : Keys)
	{
		auto CurrentClientSocket = RegisteredSockets.Find(CurrentClientEndpoint)->Get();

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

				auto TargetClient = RegisteredSockets.Find(TargetClientEndpoint)->Get();
				auto BytesSent = 0;

				TargetClient->SendTo(Data.GetData(), Data.Num(), BytesSent, TargetClientEndpoint.ToInternetAddr().Get());
			}
		}
	}

	return true;
}

void UVoiceServer::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		IP = Response->GetContentAsString() + ":" + FString::FromInt(GetDefaultPort());
		UE_LOG(LogVoice, Display, TEXT("Response received successfully. IP: %s"), *IP)
	} else
	{
		IP = GetDefaultIP();
		UE_LOG(LogVoice, Display, TEXT("Response doesn't received. The default IP will be used: %s"), *IP)
	}
	StartTCPServer();
}