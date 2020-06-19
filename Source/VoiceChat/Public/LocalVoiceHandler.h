// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoiceHandler.h"

/**
 * 
 */
class VOICECHAT_API FLocalVoiceHandler : FVoiceHandler
{
public:

	explicit FLocalVoiceHandler(const TSharedPtr<UVoiceComponent, ESPMode::ThreadSafe>& VoiceComponent)
		: FVoiceHandler(VoiceComponent) {}

	uint32 DoTask() override;

};
