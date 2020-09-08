// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

class FVoiceChatModule : public IModuleInterface
{
public:

	static FString GetConfigFileName() { return "MultiConfig.ini"; }
	static FString GetConfigPath() { return FString(FPaths::ProjectDir().Append("MultiConfig.ini")); }

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	
};
