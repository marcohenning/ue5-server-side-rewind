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
	TMap<FName, FHitBoxSnapshot> HitBoxSnapshots;
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
};
