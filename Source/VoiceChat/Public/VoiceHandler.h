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

	FVoiceHandler(UVoiceComponent* InVoiceComponent, float InSleepTime)
		: VoiceComponent(InVoiceComponent), SleepTime(InSleepTime), Thread(nullptr), bStopped(false) {}

	//FVoiceHandler(const TSharedPtr<FVoiceHandler>& InVoiceHandler);
	
	virtual ~FVoiceHandler() override;

	virtual void Start();
	virtual void Stop() override;
	uint32 Run() override;
	virtual uint32 DoTask() = 0;

protected:

	UPROPERTY()
	UVoiceComponent* VoiceComponent;

	float SleepTime;

	FRunnableThread* Thread;

	bool bStopped;
};

