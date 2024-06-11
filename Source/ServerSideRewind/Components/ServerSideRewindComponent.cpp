#include "ServerSideRewindComponent.h"


UServerSideRewindComponent::UServerSideRewindComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UServerSideRewindComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UServerSideRewindComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
