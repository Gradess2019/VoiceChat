// MARTA 2020

#include "VoiceComponent.h"
#include "OnlineSubsystem.h"
#include "VoiceModule.h"
#include <typeinfo>
#include "Kismet/KismetSystemLibrary.h"

UVoiceComponent::UVoiceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVoiceComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UVoiceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

