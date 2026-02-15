#include "Player/PowderPlayerController.h"
#include "Player/PowderCharacter.h"
#include "Player/PowderMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

APowderPlayerController::APowderPlayerController()
{
	bShowMouseCursor = false;
	PrimaryActorTick.bCanEverTick = true;
}

void APowderPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bEnableTouchEvents = true;

	// Add input mapping context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APowderPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind touch events via input component
	InputComponent->BindTouch(IE_Pressed, this, &APowderPlayerController::HandleTouchBegin);
	InputComponent->BindTouch(IE_Released, this, &APowderPlayerController::HandleTouchEnd);
	InputComponent->BindTouch(IE_Repeat, this, &APowderPlayerController::HandleTouchMove);
}

void APowderPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Cache movement component reference
	if (!CachedMovement)
	{
		if (APowderCharacter* PowderChar = Cast<APowderCharacter>(GetPawn()))
		{
			CachedMovement = PowderChar->GetPowderMovement();
		}
	}

	// Update carve input to movement component
	if (CachedMovement)
	{
		if (bTouchActive)
		{
			TouchHoldDuration += DeltaTime;
			// Carve intensity ramps up with hold duration (max at 0.5s hold)
			float Intensity = FMath::Clamp(TouchHoldDuration / 0.5f, 0.3f, 1.0f);
			CachedMovement->SetCarveInput(TouchCarveInput * Intensity);
		}
		else
		{
			CachedMovement->ReleaseCarve();

			// Auto-activate boost when releasing a full meter carve
			if (CachedMovement->GetBoostMeter() >= 1.0f)
			{
				CachedMovement->ActivateBoost();
			}
		}
	}
}

void APowderPlayerController::HandleTouchBegin(ETouchIndex::Type FingerIndex, FVector Location)
{
	bTouchActive = true;
	TouchHoldDuration = 0.0f;
	ProcessTouchLocation(Location);
}

void APowderPlayerController::HandleTouchEnd(ETouchIndex::Type FingerIndex, FVector Location)
{
	bTouchActive = false;
	TouchHoldDuration = 0.0f;
}

void APowderPlayerController::HandleTouchMove(ETouchIndex::Type FingerIndex, FVector Location)
{
	ProcessTouchLocation(Location);
}

void APowderPlayerController::ProcessTouchLocation(const FVector& Location)
{
	int32 ViewportSizeX, ViewportSizeY;
	GetViewportSize(ViewportSizeX, ViewportSizeY);

	float ScreenMidX = ViewportSizeX * 0.5f;
	// Left half = carve left (-1), Right half = carve right (+1)
	TouchCarveInput = (Location.X < ScreenMidX) ? -1.0f : 1.0f;
}
