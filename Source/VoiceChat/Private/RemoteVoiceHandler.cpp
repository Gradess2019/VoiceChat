// Fill out your copyright notice in the Description page of Project Settings.

#include "RemoteVoiceHandler.h"

uint32 FRemoteVoiceHandler::DoTask()
{
	VoiceComponent->RecieveVoiceData();
	return RECEIVE_VOICE_DATA_SUCCESS;
}
