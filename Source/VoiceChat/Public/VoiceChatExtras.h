#pragma once
#include "CoreMinimal.h"
#include "VoiceChatExtras.generated.h"

UENUM(BlueprintType)
enum class EVoiceCaptureExtendedState : uint8
{
	VCES_UnInitialized UMETA(DisplayName = "UnInitialized"),
	VCES_NotCapturing UMETA(DisplayName = "NotCapturing"),
	VCES_Ok UMETA(DisplayName = "Ok"),
	VCES_NoData UMETA(DisplayName = "NoData"),
	VCES_Stopping UMETA(DisplayName = "Stopping"),
	VCES_BufferTooSmall UMETA(DisplayName = "BufferTooSmall"),
	VCES_Error UMETA(DisplayName = "Error")
};
