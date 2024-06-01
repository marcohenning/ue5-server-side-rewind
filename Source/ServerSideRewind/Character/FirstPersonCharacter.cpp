#include "FirstPersonCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ServerSideRewind/GameMode/ServerSideRewindGameMode.h"


AFirstPersonCharacter::AFirstPersonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = true;

	/** Set up first person camera component */
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), FName("head"));
	FirstPersonCamera->bUsePawnControlRotation = true;

	/** Set movement variables */
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	GetCharacterMovement()->MaxAcceleration = 1500.0f;
	GetCharacterMovement()->AirControl = 1.0f;

	/** Let mesh block visibility channel to be able to hit character with line traces */
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	/** Set multiplayer replication frequencies */
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void AFirstPersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	/** Add input mapping context */
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContextCharacter, 0);
		}
	}

	/** Add crosshair widget to the viewport */
	if (CrosshairWidgetClass)
	{
		CreateWidget<UUserWidget>(GetWorld(), CrosshairWidgetClass)->AddToViewport();
	}
}

void AFirstPersonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFirstPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	/** Set up input action bindings */
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFirstPersonCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFirstPersonCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(KillAction, ETriggerEvent::Started, this, &AFirstPersonCharacter::KillButtonPressed);
	}
}

void AFirstPersonCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	/** Default movement implementation */
	if (GetController())
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AFirstPersonCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();

	/** Default look implementation */
	if (GetController())
	{
		AddControllerYawInput(LookVector.X);
		AddControllerPitchInput(LookVector.Y);
	}
}

void AFirstPersonCharacter::KillButtonPressed()
{
	/** Get viewport center */
	FVector2D ViewportSize;
	if (GEngine->GameViewport) { GEngine->GameViewport->GetViewportSize(ViewportSize); }
	FVector2D ViewportCenter(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);

	/** Convert viewport center from screen space to world space */
	FVector ViewportCenterWorldPosition;
	FVector ViewportCenterWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(
		this, 0), ViewportCenter, ViewportCenterWorldPosition, ViewportCenterWorldDirection);

	/** Start of the line trace offset by 1 meter to avoid hitting own mesh */
	FVector Start = ViewportCenterWorldPosition + ViewportCenterWorldDirection * 100.0f;
	/** End of the line trace 1000 meters from start */
	FVector End = Start + ViewportCenterWorldDirection * 100000.0f;

	/** Perform line trace on client (used for drawing debug lines and boxes) */
	if (!HasAuthority())
	{
		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);

		/** Draw debug line */
		if (HitResult.bBlockingHit)
		{
			DrawDebugLine(GetWorld(), Start, HitResult.Location, FColor(255, 0, 0), false, 2.0f, 0, 1.0f);

			/** Draw debug box if hit actor is of type AFirstPersonCharacter */
			if (HitResult.GetActor())
			{
				AFirstPersonCharacter* HitCharacter = Cast<AFirstPersonCharacter>(HitResult.GetActor());
				if (HitCharacter)
				{
					DrawDebugBox(GetWorld(), HitResult.Location, FVector(6.0f), FColor::Red, false, 10.0f, 0, 1.0f);
				}
			}
		}
		else { DrawDebugLine(GetWorld(), Start, End, FColor(255, 0, 0), false, 10.0f, 0, 1.0f); }
	}

	/** Request server to check for kill */
	if (bScreenToWorld) { ServerKillButtonPressed(Start, End); }
}

void AFirstPersonCharacter::CheckForKill(FVector Start, FVector End)
{
	UE_LOG(LogTemp, Warning, TEXT("Check for kill normally."))
}

void AFirstPersonCharacter::CheckForKillServerSideRewind(FVector Start, FVector End)
{
	UE_LOG(LogTemp, Warning, TEXT("Check for kill using server-side-rewind."))
}

void AFirstPersonCharacter::ServerKillButtonPressed_Implementation(FVector Start, FVector End)
{
	/** Get game mode */
	AGameModeBase* GameModeBase = UGameplayStatics::GetGameMode(this);
	if (GameModeBase == nullptr) { return; }
	
	/** Cast to custom game mode */
	AServerSideRewindGameMode* GameMode = Cast<AServerSideRewindGameMode>(GameModeBase);
	if (GameMode == nullptr) { return; }

	/** Initiate checking for kill depending on server side rewind settings */
	if (GameMode->bUseServerSideRewind) { CheckForKillServerSideRewind(Start, End); }
	else { CheckForKill(Start, End); }
}
