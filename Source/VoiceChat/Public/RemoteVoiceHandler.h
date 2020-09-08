// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoiceHandler.h"

/**
 * 
 */
class VOICECHAT_API FRemoteVoiceHandler : public FVoiceHandler
{
public:

	FRemoteVoiceHandler(UVoiceComponent* InVoiceComponent, float InSleepTime)
		: FVoiceHandler(InVoiceComponent, InSleepTime) {}

	uint32 DoTask() override;
};
