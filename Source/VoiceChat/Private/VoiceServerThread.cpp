// Fill out your copyright notice in the Description page of Project Settings.

#include "VoiceServerThread.h"
#include "HAL/RunnableThread.h"
#include "VoiceServer.h"

FVoiceServerThread::FVoiceServerThread(UVoiceServer* InServer)
{
	Stopped = false;
	Server = InServer;
	Thread = FRunnableThread::Create(this, TEXT("Voice server thread"), 8 * 1024);
}

FVoiceServerThread::~FVoiceServerThread()
{
	if (Thread != nullptr)
	{
		Thread->Kill(true);
		delete Thread;
	}
}

uint32 FVoiceServerThread::Run()
{
	while (!(Stopped || Server == nullptr))
	{
		if (Server->CheckSockets()) { continue; }
	}
	return 0;
}

void FVoiceServerThread::Stop()
{
	Stopped = true;
}
