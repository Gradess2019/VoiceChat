// Fill out your copyright notice in the Description page of Project Settings.


#include "VoiceServer.h"
#include "VoiceComponent.h"
#include "Common/TcpListener.h"
#include "Engine/World.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "VoiceServerThread.h"

bool UVoiceServer::InitIP(bool bInLan)
{
	if (bInLan)
	{
		Address = FString("127.0.0.1:7778");
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

	FString TargetHost = "http://api.ipify.org";
	TSharedRef < IHttpRequest > Request = Http->CreateRequest();
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
	FIPv4Endpoint::Parse(Address, Endpoint);
	Listener = MakeShared<FTcpListener>(Endpoint);
	Listener->OnConnectionAccepted().BindUObject(this, &UVoiceServer::ConnectionAccept);
	UE_LOG(LogTemp, Warning, TEXT("Server started"));
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
	if (RegisteredSockets.Num() <= 0) { return false; }
	uint32 PendingData = 0;

	//auto bContainsData = Listener->GetSocket()->HasPendingData(PendingData);
	//if (!bContainsData) { return false; }

	TArray<FIPv4Endpoint> Keys;
	RegisteredSockets.GenerateKeyArray(Keys);
	
	auto CurrentClientEndpoint = Keys[0];
	auto CurrentClient = RegisteredSockets.Find(CurrentClientEndpoint)->Get();
	auto Data = TArray<uint8>();
	auto BytesRead = 0;
	
	Data.SetNumUninitialized(MAX_VOICE_PACKAGE_SIZE);
	CurrentClient->Wait(ESocketWaitConditions::WaitForRead, WAIT_ONE_RATE);
	auto bSuccess = CurrentClient->Recv(Data.GetData(), Data.Num(), BytesRead);
	Data.SetNum(BytesRead);

	if (BytesRead > 0)
	{
		for (auto TargetClientEndpoint : Keys)
		{
			if (TargetClientEndpoint == CurrentClientEndpoint) { continue; }
			auto TargetClient = RegisteredSockets.Find(TargetClientEndpoint)->Get();

			UE_LOG(LogTemp, Warning, TEXT("Server: Bytes read: %d; Data: %d"), BytesRead, Data[0]);
			auto BytesSent = 0;
			TargetClient->SendTo(Data.GetData(), Data.Num(), BytesSent, TargetClientEndpoint.ToInternetAddr().Get());
		}
	} else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Server: There are no data"));
	}

	return true;
}

void UVoiceServer::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	Address = Response->GetContentAsString() + ":7778";
	StartTCPServer();
}