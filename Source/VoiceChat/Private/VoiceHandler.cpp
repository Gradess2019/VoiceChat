#include "VoiceHandler.h"
#include "HAL/RunnableThread.h"

FVoiceHandler::~FVoiceHandler()
{
	if (Thread != nullptr)
	{
		Thread->Kill();
		delete Thread;
	}
}

uint32 FVoiceHandler::Run()
{
	// Set default value of Result variable
	uint32 Result = THREAD_STOPPED_BEFORE_CYCLE;
	while (!bStopped)
	{
		if (VoiceComponent)
		{
			Result = DoTask();
		} else
		{
			Stop();
			return VOICE_COMPONENT_NULL;
		}
	}
	return Result;
}

void FVoiceHandler::Stop()
{
	bStopped = true;
}

