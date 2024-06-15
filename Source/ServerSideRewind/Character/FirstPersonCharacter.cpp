#include "FirstPersonCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ServerSideRewind/GameMode/ServerSideRewindGameMode.h"
#include "Components/BoxComponent.h"
#include "ServerSideRewind/Components/ServerSideRewindComponent.h"
#include "GameFramework/GameStateBase.h"


AFirstPersonCharacter::AFirstPersonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = true;

	/** Set up first person camera component */
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), FName("head"));
	FirstPersonCamera->bUsePawnControlRotation = true;

	/** Set up server side rewind component */
	ServerSideRewindComponent = CreateDefaultSubobject<UServerSideRewindComponent>(
		TEXT("ServerSideRewindComponent"));

	/** Set movement variables */
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	GetCharacterMovement()->MaxAcceleration = 1500.0f;
	GetCharacterMovement()->AirControl = 1.0f;

	/** Set up mesh collision */
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	/** Set multiplayer replication frequencies */
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	/**
	* Hit boxes used for server-side rewind
	*/
	HitBoxHead = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxHead"));
	HitBoxHead->SetupAttachment(GetMesh(), "head");
	HitBoxes.Add("head", HitBoxHead);

	HitBoxUpperTorso = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxUpperTorso"));
	HitBoxUpperTorso->SetupAttachment(GetMesh(), "spine_03");
	HitBoxes.Add("spine_03", HitBoxUpperTorso);

	HitBoxLowerTorso = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxLowerTorso"));
	HitBoxLowerTorso->SetupAttachment(GetMesh(), "spine_01");
	HitBoxes.Add("spine_01", HitBoxLowerTorso);

	HitBoxRightUpperArm = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxRightUpperArm"));
	HitBoxRightUpperArm->SetupAttachment(GetMesh(), "upperarm_r");
	HitBoxes.Add("upperarm_r", HitBoxRightUpperArm);

	HitBoxRightLowerArm = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxRightLowerArm"));
	HitBoxRightLowerArm->SetupAttachment(GetMesh(), "lowerarm_r");
	HitBoxes.Add("lowerarm_r", HitBoxRightLowerArm);

	HitBoxRightHand = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxRightHand"));
	HitBoxRightHand->SetupAttachment(GetMesh(), "hand_r");
	HitBoxes.Add("hand_r", HitBoxRightHand);

	HitBoxLeftUpperArm = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxLeftUpperArm"));
	HitBoxLeftUpperArm->SetupAttachment(GetMesh(), "upperarm_l");
	HitBoxes.Add("upperarm_l", HitBoxLeftUpperArm);

	HitBoxLeftLowerArm = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxLeftLowerArm"));
	HitBoxLeftLowerArm->SetupAttachment(GetMesh(), "lowerarm_l");
	HitBoxes.Add("lowerarm_l", HitBoxLeftLowerArm);

	HitBoxLeftHand = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxLeftHand"));
	HitBoxLeftHand->SetupAttachment(GetMesh(), "hand_l");
	HitBoxes.Add("hand_l", HitBoxLeftHand);

	HitBoxRightUpperLeg = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxRightUpperLeg"));
	HitBoxRightUpperLeg->SetupAttachment(GetMesh(), "thigh_r");
	HitBoxes.Add("thigh_r", HitBoxRightUpperLeg);

	HitBoxRightLowerLeg = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxRightLowerLeg"));
	HitBoxRightLowerLeg->SetupAttachment(GetMesh(), "calf_r");
	HitBoxes.Add("calf_r", HitBoxRightLowerLeg);

	HitBoxRightFoot = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxRightFoot"));
	HitBoxRightFoot->SetupAttachment(GetMesh(), "foot_r");
	HitBoxes.Add("foot_r", HitBoxRightFoot);

	HitBoxLeftUpperLeg = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxLeftUpperLeg"));
	HitBoxLeftUpperLeg->SetupAttachment(GetMesh(), "thigh_l");
	HitBoxes.Add("thigh_l", HitBoxLeftUpperLeg);

	HitBoxLeftLowerLeg = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxLeftLowerLeg"));
	HitBoxLeftLowerLeg->SetupAttachment(GetMesh(), "calf_l");
	HitBoxes.Add("calf_l", HitBoxLeftLowerLeg);

	HitBoxLeftFoot = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxLeftFoot"));
	HitBoxLeftFoot->SetupAttachment(GetMesh(), "foot_l");
	HitBoxes.Add("foot_l", HitBoxLeftFoot);

	for (auto HitBox : HitBoxes)
	{
		if (HitBox.Value)
		{
			HitBox.Value->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);
			HitBox.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			HitBox.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1,
				ECollisionResponse::ECR_Block);
			HitBox.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
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
	/** Get game state if nullptr, otherwise use the member variable */
	GameState = GameState == nullptr ? UGameplayStatics::GetGameState(this) : GameState;
	if (GameState == nullptr) { return; }

	/** Get viewport center */
	FVector2D ViewportSize;
	if (GEngine->GameViewport) { GEngine->GameViewport->GetViewportSize(ViewportSize); }
	FVector2D ViewportCenter(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);

	/** Convert viewport center from screen space to world space */
	FVector ViewportCenterWorldPosition;
	FVector ViewportCenterWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(
		this, 0), ViewportCenter, ViewportCenterWorldPosition, ViewportCenterWorldDirection);

	if (!bScreenToWorld) { return; }

	/** Start of the line trace offset by 1 meter to avoid hitting own mesh */
	FVector Start = ViewportCenterWorldPosition + ViewportCenterWorldDirection * 100.0f;
	/** End of the line trace 1000 meters from start */
	FVector End = Start + ViewportCenterWorldDirection * 100000.0f;

	AFirstPersonCharacter* HitCharacter = (AFirstPersonCharacter*) nullptr;

	/** Perform line trace */
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);

	/** Draw debug line */
	if (HitResult.bBlockingHit)
	{
		DrawDebugLine(GetWorld(), Start, HitResult.Location, FColor(255, 0, 0), false, 10.0f, 0, 1.0f);

		/** Draw debug box if hit actor is of type AFirstPersonCharacter */
		if (HitResult.GetActor())
		{
			HitCharacter = Cast<AFirstPersonCharacter>(HitResult.GetActor());
			if (HitCharacter)
			{
				DrawDebugBox(GetWorld(), HitResult.Location, FVector(8.0f), FColor(255, 0, 0), false, 10.0f, 0, 1.0f);
			}
		}
	}
	else { DrawDebugLine(GetWorld(), Start, End, FColor(255, 0, 0), false, 2.0f, 0, 1.0f); }

	/** Handle server (Check for kill without server side rewind) */
	if (HasAuthority()) { CheckForKill(Start, End); }

	/** Handle clients (Request server to check for kill) */
	else { ServerKillButtonPressed(HitCharacter, GameState->GetServerWorldTimeSeconds(), Start, End); }
}

void AFirstPersonCharacter::CheckForKill(FVector Start, FVector End)
{
	/** Perform line trace */
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);

	/** Enable ragdoll if hit character is of type AFirstPersonCharacter */
	if (HitResult.bBlockingHit && HitResult.GetActor())
	{
		AFirstPersonCharacter* HitCharacter = Cast<AFirstPersonCharacter>(HitResult.GetActor());
		if (HitCharacter) { HitCharacter->MulticastRagdoll(); }
	}
}

void AFirstPersonCharacter::CheckForKillServerSideRewind(AFirstPersonCharacter* HitCharacter, 
	float Time, FVector Start, FVector End)
{
	if (ServerSideRewindComponent == nullptr || HitCharacter == nullptr) { return; }

	/** Kill player if check was successful */
	if (ServerSideRewindComponent->CheckForKill(HitCharacter, Time, Start, End))
	{
		HitCharacter->MulticastRagdoll();
	}
}

void AFirstPersonCharacter::ServerKillButtonPressed_Implementation(AFirstPersonCharacter* HitCharacter, 
	float Time, FVector Start, FVector End)
{
	/** Get game mode */
	AGameModeBase* GameModeBase = UGameplayStatics::GetGameMode(this);
	if (GameModeBase == nullptr) { return; }
	
	/** Cast to custom game mode */
	AServerSideRewindGameMode* GameMode = Cast<AServerSideRewindGameMode>(GameModeBase);
	if (GameMode == nullptr) { return; }

	/** Initiate checking for kill depending on server side rewind settings */
	if (GameMode->bUseServerSideRewind) { CheckForKillServerSideRewind(HitCharacter, Time, Start, End); }
	else { CheckForKill(Start, End); }
}

void AFirstPersonCharacter::MulticastRagdoll_Implementation()
{
	/** Enable ragdoll */
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();
	GetMesh()->bBlendPhysics = true;

	/** Stop character movement */
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
}
