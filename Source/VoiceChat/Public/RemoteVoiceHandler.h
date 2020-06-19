// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoiceHandler.h"

/**
 * 
 */
class VOICECHAT_API FRemoteVoiceHandler : FVoiceHandler
{
public:

	explicit FRemoteVoiceHandler(const TSharedPtr<UVoiceComponent, ESPMode::ThreadSafe>& VoiceComponent)
		: FVoiceHandler(VoiceComponent) {}

	uint32 DoTask() override;
};
