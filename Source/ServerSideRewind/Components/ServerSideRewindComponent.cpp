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

	SaveServerSideRewindSnapshot();
}

void UServerSideRewindComponent::TakeServerSideRewindSnapshot(FServerSideRewindSnapshot& Snapshot)
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

void UServerSideRewindComponent::SaveServerSideRewindSnapshot()
{
	/** Try getting owning character */
	Character = Character == nullptr ? Cast<AFirstPersonCharacter>(GetOwner()) : Character;
	if (Character == nullptr || !Character->HasAuthority()) { return; }

	/** Handle first and second snapshot */
	if (ServerSideRewindSnapshotHistory.Num() <= 1)
	{
		/** Take snapshot and save to snapshot history */
		FServerSideRewindSnapshot ServerSideRewindSnapshot;
		TakeServerSideRewindSnapshot(ServerSideRewindSnapshot);
		ServerSideRewindSnapshotHistory.AddHead(ServerSideRewindSnapshot);

		/** Show snapshot on screen */
		ShowServerSideRewindSnapshot(ServerSideRewindSnapshot);
	}
	/** Handle all other snapshots */
	else
	{
		/** Total stored time in snapshot history */
		float SnapshotHistoryLength = ServerSideRewindSnapshotHistory.GetHead()->
			GetValue().Time - ServerSideRewindSnapshotHistory.GetTail()->GetValue().Time;
		
		/** Remove snapshots older than MaxRewindTime */
		while (SnapshotHistoryLength > MaxRewindTime)
		{
			ServerSideRewindSnapshotHistory.RemoveNode(ServerSideRewindSnapshotHistory.GetTail());
			SnapshotHistoryLength = ServerSideRewindSnapshotHistory.GetHead()-> GetValue().Time - 
				ServerSideRewindSnapshotHistory.GetTail()->GetValue().Time;
		}

		/** Take snapshot and save to snapshot history */
		FServerSideRewindSnapshot ServerSideRewindSnapshot;
		TakeServerSideRewindSnapshot(ServerSideRewindSnapshot);
		ServerSideRewindSnapshotHistory.AddHead(ServerSideRewindSnapshot);

		/** Show snapshot on screen */
		ShowServerSideRewindSnapshot(ServerSideRewindSnapshot);
	}
}

void UServerSideRewindComponent::ShowServerSideRewindSnapshot(const FServerSideRewindSnapshot& Snapshot)
{
	/** Draw every hitbox to screen */
	for (FHitBoxSnapshot HitBox : Snapshot.HitBoxSnapshots)
	{
		DrawDebugBox(GetWorld(), HitBox.Location, HitBox.Extent, 
			FQuat(HitBox.Rotation), FColor::Red, false, MaxRewindTime);
	}
}

bool UServerSideRewindComponent::CheckForKill(AFirstPersonCharacter* HitCharacter, 
	float Time, FVector Start, FVector End)
{
	return false;
}
