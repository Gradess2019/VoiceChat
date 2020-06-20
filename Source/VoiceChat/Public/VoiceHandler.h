#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "VoiceComponent.h"

#define VOICE_COMPONENT_NULL 255
#define THREAD_RETURN_DEFAULT_VALUE 254
#define RECEIVE_VOICE_DATA_SUCCESS 0
#define SEND_VOICE_DATA_SUCCESS 1

class VOICECHAT_API FVoiceHandler : public FRunnable
{
public:

	FVoiceHandler(const TSharedPtr<UVoiceComponent, ESPMode::ThreadSafe>& InVoiceComponent, float InSleepTime)
		: VoiceComponent(InVoiceComponent), SleepTime(InSleepTime), Thread(nullptr), bStopped(false) {}

	virtual ~FVoiceHandler() override;

	uint32 Run() override;
	virtual void Stop() override;

	virtual uint32 DoTask() = 0;

	virtual void Start();

protected:

	TSharedPtr<UVoiceComponent, ESPMode::ThreadSafe> VoiceComponent;

	float SleepTime;

	FRunnableThread* Thread;

	bool bStopped;
};

