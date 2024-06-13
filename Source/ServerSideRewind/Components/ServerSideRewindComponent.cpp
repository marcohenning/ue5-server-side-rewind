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

void UServerSideRewindComponent::TakeServerSideRewindSnapshot(AFirstPersonCharacter* TargetCharacter, 
	FServerSideRewindSnapshot& Snapshot)
{
	if (TargetCharacter == nullptr) { return; }

	Snapshot.Character = TargetCharacter;
	Snapshot.Time = GetWorld()->GetTimeSeconds();

	for (auto& HitBox : TargetCharacter->HitBoxes)
	{
		if (HitBox.Value == nullptr) { break; }

		FHitBoxSnapshot HitBoxSnapshot;
		HitBoxSnapshot.Location = HitBox.Value->GetComponentLocation();
		HitBoxSnapshot.Rotation = HitBox.Value->GetComponentRotation();
		HitBoxSnapshot.Extent = HitBox.Value->GetScaledBoxExtent();
		Snapshot.HitBoxSnapshots.Add(HitBox.Key, HitBoxSnapshot);
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
		TakeServerSideRewindSnapshot(Character, ServerSideRewindSnapshot);
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
		TakeServerSideRewindSnapshot(Character, ServerSideRewindSnapshot);
		ServerSideRewindSnapshotHistory.AddHead(ServerSideRewindSnapshot);

		/** Show snapshot on screen */
		ShowServerSideRewindSnapshot(ServerSideRewindSnapshot);
	}
}

void UServerSideRewindComponent::ShowServerSideRewindSnapshot(const FServerSideRewindSnapshot& Snapshot)
{
	/** Draw every hitbox to screen */
	for (auto& HitBox : Snapshot.HitBoxSnapshots)
	{
		DrawDebugBox(GetWorld(), HitBox.Value.Location, HitBox.Value.Extent,
			FQuat(HitBox.Value.Rotation), FColor::Red, false, MaxRewindTime);
	}
}

void UServerSideRewindComponent::MoveHitBoxesToSnapshot(AFirstPersonCharacter* TargetCharacter, 
	const FServerSideRewindSnapshot& Snapshot)
{
	if (TargetCharacter == nullptr) { return; }

	for (auto& HitBox : TargetCharacter->HitBoxes)
	{
		if (HitBox.Value == nullptr) { break; }

		HitBox.Value->SetWorldLocation(Snapshot.HitBoxSnapshots[HitBox.Key].Location);
		HitBox.Value->SetWorldRotation(Snapshot.HitBoxSnapshots[HitBox.Key].Rotation);
		HitBox.Value->SetBoxExtent(Snapshot.HitBoxSnapshots[HitBox.Key].Extent);
	}
}

bool UServerSideRewindComponent::CheckForKill(AFirstPersonCharacter* HitCharacter,
	float Time, FVector Start, FVector End)
{
	return false;
}
