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
	// Enhanced Input bindings can be configured here or in Blueprint
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

bool APowderPlayerController::InputTouch(uint32 Handle, ETouchType::Type Type,
	const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchpadIndex)
{
	int32 ViewportSizeX, ViewportSizeY;
	GetViewportSize(ViewportSizeX, ViewportSizeY);

	float ScreenMidX = ViewportSizeX * 0.5f;

	switch (Type)
	{
	case ETouchType::Began:
		bTouchActive = true;
		TouchHoldDuration = 0.0f;
		// Left half = carve left (-1), Right half = carve right (+1)
		TouchCarveInput = (TouchLocation.X < ScreenMidX) ? -1.0f : 1.0f;
		break;

	case ETouchType::Moved:
		// Update side if finger crosses midpoint
		TouchCarveInput = (TouchLocation.X < ScreenMidX) ? -1.0f : 1.0f;
		break;

	case ETouchType::Ended:
	case ETouchType::ForceChanged:
		bTouchActive = false;
		TouchHoldDuration = 0.0f;
		break;

	default:
		break;
	}

	return true;
}

void APowderPlayerController::UpdateCarveFromTouch()
{
	// Handled in Tick for smooth interpolation
}
