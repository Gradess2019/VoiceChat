// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable CppRedundantVirtualSpecifierOnOverrideFunction
#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"

class UVoiceServer;

/**
 * 
 */
class VOICECHAT_API FVoiceServerThread : FRunnable
{
public:
	
	FVoiceServerThread(UVoiceServer* InServer);
	~FVoiceServerThread();

	virtual uint32 Run() override;
	virtual void Stop() override;
	
protected:

	FRunnableThread* Thread;
	UVoiceServer* Server;
	bool Stopped;
};
