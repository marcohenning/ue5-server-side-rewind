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

FServerSideRewindSnapshot UServerSideRewindComponent::FindSnapshotToCheck(
	AFirstPersonCharacter* TargetCharacter, float Time)
{
	/** Checking for nullptr */
	if (TargetCharacter == nullptr || TargetCharacter->GetServerSideRewindComponent() == nullptr ||
		TargetCharacter->GetServerSideRewindComponent()->ServerSideRewindSnapshotHistory.GetHead() == nullptr ||
		TargetCharacter->GetServerSideRewindComponent()->ServerSideRewindSnapshotHistory.GetTail() == nullptr)
	{
		return FServerSideRewindSnapshot();
	}

	/** Get snapshot history */
	const TDoubleLinkedList<FServerSideRewindSnapshot>& History = TargetCharacter->
		GetServerSideRewindComponent()->ServerSideRewindSnapshotHistory;

	/** Get oldest and latest times in history */
	const float OldestTime = History.GetTail()->GetValue().Time;
	const float LatestTime = History.GetHead()->GetValue().Time;

	/** Too far back in the past */
	if (OldestTime > Time) { return FServerSideRewindSnapshot(); }

	/** Exact match between hit time and oldest time, simply return oldest snapshot */
	if (OldestTime == Time) { return History.GetTail()->GetValue(); }

	/** Hit time newer than or equal to latest snapshot, simply return latest snapshot */
	if (LatestTime <= Time) { return History.GetHead()->GetValue(); }

	/** Find first snapshot that is equal to or older than hit time and return it */
	TDoubleLinkedList<FServerSideRewindSnapshot>::TDoubleLinkedListNode* CurrentNode = History.GetHead();
	while (CurrentNode->GetValue().Time > Time)
	{
		if (CurrentNode->GetNextNode() == nullptr) { break; }
		CurrentNode = CurrentNode->GetNextNode();
	}
	return CurrentNode->GetValue();
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
	if (HitCharacter == nullptr) { return false; }

	/** Save current snapshot to reset hitboxes after checking for kill */
	FServerSideRewindSnapshot CurrentSnapshot;
	TakeServerSideRewindSnapshot(HitCharacter, CurrentSnapshot);

	/** Find snapshot to check */
	FServerSideRewindSnapshot SnapshotToCheck = FindSnapshotToCheck(HitCharacter, Time);

	/** Move hitboxes to their position at the time of the snapshot to check */
	MoveHitBoxesToSnapshot(HitCharacter, SnapshotToCheck);

	/** TODO: Implement check */
	return false;
}
