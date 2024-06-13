#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ServerSideRewindComponent.generated.h"


class AFirstPersonCharacter;


/**
* Struct used to save snapshots of a single hitbox.
* Helper struct for FServerSideRewindSnapshot.
*/
USTRUCT(BlueprintType)
struct FHitBoxSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector Extent;
};

/**
* Struct used to save snapshots of a character.
*/
USTRUCT(BlueprintType)
struct FServerSideRewindSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	AFirstPersonCharacter* Character;

	UPROPERTY()
	TArray<FHitBoxSnapshot> HitBoxSnapshots;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SERVERSIDEREWIND_API UServerSideRewindComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UServerSideRewindComponent();
	friend class AFirstPersonCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	/** Character owning this component */
	UPROPERTY()
	AFirstPersonCharacter* Character;

	/** Max amount of seconds to go back in time */
	float MaxRewindTime = 3.0f;

	/** Server side rewind snapshots going back as far as MaxRewindTime allows */
	TDoubleLinkedList<FServerSideRewindSnapshot> ServerSideRewindSnapshotHistory;

	/** Takes snapshot of the current character state */
	void TakeServerSideRewindSnapshot(FServerSideRewindSnapshot& Snapshot);

	/** Saves snapshot of the current character state */
	void SaveServerSideRewindSnapshot();

	/** Draws hitboxes (Debug only) */
	void ShowServerSideRewindSnapshot(const FServerSideRewindSnapshot& Snapshot);

	/** Checks for kill using server side rewind */
	bool CheckForKill(AFirstPersonCharacter* HitCharacter, float Time, FVector Start, FVector End);
};
