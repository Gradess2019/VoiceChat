// Fill out your copyright notice in the Description page of Project Settings.

#include "RemoteVoiceHandler.h"

uint32 FRemoteVoiceHandler::DoTask()
{
	auto bConnected = VoiceComponent->CheckSocketConnection();
	if (bConnected)
	{
		VoiceComponent->ReceiveVoiceData();
	}
	else {
		FPlatformProcess::Sleep(SleepTime);
	}
	return RECEIVE_VOICE_DATA_SUCCESS;
}
