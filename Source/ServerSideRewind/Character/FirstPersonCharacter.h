#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FirstPersonCharacter.generated.h"


class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;


/**
* Custom character using the default Unreal Engine 4 mannequin.
*/
UCLASS()
class SERVERSIDEREWIND_API AFirstPersonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFirstPersonCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

private:
	/** First person camera component */
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* FirstPersonCamera;

	/** Character mapping context */
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* MappingContextCharacter;

	/** Input action for basic movement */
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Input action for looking around */
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Input action for jumping */
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Input action for spotting */
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UInputAction* KillAction;

	/** Crosshair widget class added to the viewport in beginplay */
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called when the spot button is pressed */
	void KillButtonPressed();

	/** Server remote procedure call to check for kill */
	UFUNCTION(Server, Reliable)
	void ServerKillButtonPressed(FVector Start, FVector End);

	void CheckForKill(FVector Start, FVector End);
	void CheckForKillServerSideRewind(FVector Start, FVector End);
};
