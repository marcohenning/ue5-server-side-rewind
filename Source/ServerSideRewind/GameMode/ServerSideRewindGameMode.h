#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ServerSideRewindGameMode.generated.h"


/**
* Custom game mode used only for storing information about whether to use server side rewind.
*/
UCLASS()
class SERVERSIDEREWIND_API AServerSideRewindGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	bool bUseServerSideRewind = false;
};
