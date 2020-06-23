// Fill out your copyright notice in the Description page of Project Settings.


#include "VoiceServer.h"
#include "Common/TcpListener.h"
#include "Dom/JsonObject.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "VoiceChat.h"
#include "VoiceModule.h"
#include "VoiceServerThread.h"

void UVoiceServer::Init()
{
	LocalIP = TEXT("");
	auto Http = &FHttpModule::Get();
	auto APIUrl = TEXT("https://api.ipify.org/?format=json");

	if (!Http)
	{
		return;
	}

	TSharedRef< IHttpRequest > Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UVoiceServer::OnResponseReceived);
	Request->SetURL(APIUrl);
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->ProcessRequest();
}

void UVoiceServer::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonObject;

		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			PublicIP = AppendDefaultPort(JsonObject->GetStringField("ip"));
			UE_LOG(LogVoice, Display, TEXT("Got public server IP"));
			
			StartTCPServer();
		}
	}
	else
	{
		UE_LOG(LogVoice, Error, TEXT("Get My Public IP : Request failed."));
	}
}

FString UVoiceServer::AppendDefaultPort(FString Address)
{
	return Address + ":" + FString::FromInt(GetDefaultPort());
}

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

void UVoiceServer::StartTCPServer()
{
	InitLocalIP();
	FIPv4Endpoint Endpoint;
	FIPv4Endpoint::Parse(LocalIP, Endpoint);
	Listener = MakeShared<FTcpListener>(Endpoint);
	Listener->OnConnectionAccepted().BindUObject(this, &UVoiceServer::ConnectionAccept);
	if (Listener->GetSocket()) {

		int32 OutRecieveBufferSize = 0;

		Listener->GetSocket()->SetReceiveBufferSize(SERVER_RECEIVE_BUFFER_SIZE, OutRecieveBufferSize);

		UE_LOG(LogVoice, Display, TEXT("Server started"
			"\n\tLocal IP: %s"
			"\n\tPublic IP: %s"
			"\n\tRecieve buffer size: %d"),
			*Listener.Get()->GetLocalEndpoint().ToString(),
			*PublicIP,
			OutRecieveBufferSize
		);
	}
}

void UVoiceServer::InitLocalIP()
{
	auto bCanBindAll = false;
	const auto AddressPtr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);
	const auto bAppendPort = false;
	LocalIP = AppendDefaultPort(AddressPtr->ToString(bAppendPort));
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
		auto CurrentClientSocket = AvailableSockets.FindRef(CurrentClientEndpoint);;
		if (!CurrentClientSocket.IsValid()) { continue; }

		auto Data = TArray<uint8>();
		Data.SetNumUninitialized(MAX_VOICE_PACKAGE_SIZE);

		auto BytesRead = 0;
		CurrentClientSocket->Recv(Data.GetData(), Data.Num(), BytesRead);
		Data.SetNum(BytesRead);

		if (BytesRead > 0)
		{
			UE_LOG(LogVoice, Display, TEXT("Server: Bytes read: %d; Data: %d"), BytesRead, Data[0]);
			for (auto TargetClientEndpoint : Keys)
			{
				if (TargetClientEndpoint == CurrentClientEndpoint) { continue; }

				auto TargetClient = AvailableSockets.FindRef(TargetClientEndpoint);
				auto BytesSent = 0;

				TargetClient->SendTo(Data.GetData(), Data.Num(), BytesSent, TargetClientEndpoint.ToInternetAddr().Get());
			}
		}
	}

	return true;
}

