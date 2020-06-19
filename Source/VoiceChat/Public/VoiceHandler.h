#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "VoiceComponent.h"

#define VOICE_COMPONENT_NULL 255
#define THREAD_STOPPED_BEFORE_CYCLE 254
#define RECEIVE_VOICE_DATA_SUCCESS 0
#define SEND_VOICE_DATA_SUCCESS 1

class VOICECHAT_API FVoiceHandler : FRunnable
{
public:


	explicit FVoiceHandler(const TSharedPtr<UVoiceComponent, ESPMode::ThreadSafe>& VoiceComponent)
		: VoiceComponent(VoiceComponent) {}

	virtual ~FVoiceHandler() override;

	uint32 Run() override;
	virtual void Stop() override;

	virtual uint32 DoTask() = 0;

protected:

	FRunnableThread* Thread = nullptr;

	bool bStopped = false;

	TSharedPtr<UVoiceComponent, ESPMode::ThreadSafe> VoiceComponent;
};

