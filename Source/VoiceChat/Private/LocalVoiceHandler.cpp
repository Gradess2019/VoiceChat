// Fill out your copyright notice in the Description page of Project Settings.

#include "LocalVoiceHandler.h"

uint32 FLocalVoiceHandler::DoTask()
{
	VoiceComponent->PlayVoiceCapture();
	return SEND_VOICE_DATA_SUCCESS;
}
