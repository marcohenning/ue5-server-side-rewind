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

void UServerSideRewindComponent::SaveServerSideRewindSnapshot(FServerSideRewindSnapshot& Snapshot)
{

}

void UServerSideRewindComponent::ShowServerSideRewindSnapshot(const FServerSideRewindSnapshot& Snapshot)
{

}

bool UServerSideRewindComponent::CheckForKill(AFirstPersonCharacter* HitCharacter, 
	float Time, FVector Start, FVector End)
{
	return false;
}
