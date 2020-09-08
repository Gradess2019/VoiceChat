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
	uint32 Result = THREAD_RETURN_DEFAULT_VALUE;
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

void FVoiceHandler::Start()
{
	Thread = FRunnableThread::Create(this, TEXT("RemoteVoiceHandlerThread"));
}

void FVoiceHandler::Stop()
{
	bStopped = true;
}



