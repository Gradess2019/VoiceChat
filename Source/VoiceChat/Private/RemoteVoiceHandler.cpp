// Fill out your copyright notice in the Description page of Project Settings.

#include "RemoteVoiceHandler.h"

uint32 FRemoteVoiceHandler::DoTask()
{
	auto bConnected = VoiceComponent->IsSocketConnected();
	if (VoiceComponent->IsPendingKillOrUnreachable()) { return VOICE_COMPONENT_NULL; }
	
	if (bConnected)
	{
		VoiceComponent->ReceiveVoiceData();
		if (VoiceComponent->IsPendingKillOrUnreachable()) { return VOICE_COMPONENT_NULL; }
	}
	else {
		VoiceComponent->ConnectToVoiceServer();
		FPlatformProcess::Sleep(SleepTime);
	}
	return RECEIVE_VOICE_DATA_SUCCESS;
}
