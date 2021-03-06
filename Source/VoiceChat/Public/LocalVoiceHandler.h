// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoiceHandler.h"

/**
 * 
 */
class VOICECHAT_API FLocalVoiceHandler : public FVoiceHandler
{
public:

	FLocalVoiceHandler(UVoiceComponent* InVoiceComponent, float InSleepTime)
		: FVoiceHandler(InVoiceComponent, InSleepTime) {}

	uint32 DoTask() override;

};
