#include "ServerSideRewindComponent.h"
#include "Components/BoxComponent.h"
#include "ServerSideRewind/Character/FirstPersonCharacter.h"


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

	Character = Character == nullptr ? Cast<AFirstPersonCharacter>(GetOwner()) : Character;
	if (Character == nullptr || !Character->HasAuthority()) { return; }

	FServerSideRewindSnapshot ServerSideRewindSnapshot;
	SaveServerSideRewindSnapshot(ServerSideRewindSnapshot);
	ShowServerSideRewindSnapshot(ServerSideRewindSnapshot);
}

void UServerSideRewindComponent::SaveServerSideRewindSnapshot(FServerSideRewindSnapshot& Snapshot)
{
	if (Character == nullptr) { return; }

	Snapshot.Character = Character;
	Snapshot.Time = GetWorld()->GetTimeSeconds();

	for (UBoxComponent*& HitBox : Character->HitBoxes)
	{
		if (HitBox == nullptr) { break; }

		FHitBoxSnapshot HitBoxSnapshot;
		HitBoxSnapshot.Location = HitBox->GetComponentLocation();
		HitBoxSnapshot.Rotation = HitBox->GetComponentRotation();
		HitBoxSnapshot.Extent = HitBox->GetScaledBoxExtent();
		Snapshot.HitBoxSnapshots.Add(HitBoxSnapshot);
	}
}

void UServerSideRewindComponent::ShowServerSideRewindSnapshot(const FServerSideRewindSnapshot& Snapshot)
{
	for (FHitBoxSnapshot HitBox : Snapshot.HitBoxSnapshots)
	{
		DrawDebugBox(GetWorld(), HitBox.Location, HitBox.Extent, 
			FQuat(HitBox.Rotation), FColor::Red, false, 3.0f);
	}
}

bool UServerSideRewindComponent::CheckForKill(AFirstPersonCharacter* HitCharacter, 
	float Time, FVector Start, FVector End)
{
	return false;
}
