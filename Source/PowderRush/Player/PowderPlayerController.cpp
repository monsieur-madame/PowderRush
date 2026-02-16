#include "Player/PowderPlayerController.h"
#include "Player/PowderCharacter.h"
#include "Player/PowderMovementComponent.h"
#include "Player/PowderTrickComponent.h"
#include "Core/PowderGameMode.h"
#include "Core/PowderTypes.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
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

	// Bind touch events
	InputComponent->BindTouch(IE_Pressed, this, &APowderPlayerController::HandleTouchBegin);
	InputComponent->BindTouch(IE_Released, this, &APowderPlayerController::HandleTouchEnd);
	InputComponent->BindTouch(IE_Repeat, this, &APowderPlayerController::HandleTouchMove);

	// Keyboard bindings for desktop testing (A/D or Left/Right arrows)
	InputComponent->BindKey(EKeys::A, IE_Pressed, this, &APowderPlayerController::HandleKeyCarveLeftPressed);
	InputComponent->BindKey(EKeys::A, IE_Released, this, &APowderPlayerController::HandleKeyCarveLeftReleased);
	InputComponent->BindKey(EKeys::D, IE_Pressed, this, &APowderPlayerController::HandleKeyCarveRightPressed);
	InputComponent->BindKey(EKeys::D, IE_Released, this, &APowderPlayerController::HandleKeyCarveRightReleased);
	InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &APowderPlayerController::HandleKeyCarveLeftPressed);
	InputComponent->BindKey(EKeys::Left, IE_Released, this, &APowderPlayerController::HandleKeyCarveLeftReleased);
	InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &APowderPlayerController::HandleKeyCarveRightPressed);
	InputComponent->BindKey(EKeys::Right, IE_Released, this, &APowderPlayerController::HandleKeyCarveRightReleased);

	// Keyboard trick bindings (W/S for backflip/frontflip)
	InputComponent->BindKey(EKeys::W, IE_Pressed, this, &APowderPlayerController::HandleKeyTrickUp);
	InputComponent->BindKey(EKeys::S, IE_Pressed, this, &APowderPlayerController::HandleKeyTrickDown);
	InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &APowderPlayerController::HandleKeyTrickUp);
	InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &APowderPlayerController::HandleKeyTrickDown);

	// Restart key for desktop testing
	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &APowderPlayerController::HandleRestart);
}

void APowderPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UPowderMovementComponent* Movement = GetMovementComp();
	if (!Movement)
	{
		return;
	}

	bool bAirborne = Movement->IsAirborne();

	// Track SpreadEagle hold (two-finger hold while airborne)
	if (bAirborne && bTouchActive && bSecondFingerActive)
	{
		BothFingersHoldTime += DeltaTime;
		if (BothFingersHoldTime >= SpreadEagleHoldThreshold)
		{
			if (UPowderTrickComponent* TrickComp = GetTrickComp())
			{
				TrickComp->RequestTrick(EPowderGestureDirection::HoldBoth);
			}
			BothFingersHoldTime = 0.0f;
		}
	}

	// Skip carve input while airborne (gestures only)
	if (bAirborne)
	{
		return;
	}

	// Ground carve input: touch takes priority, then keyboard
	if (bTouchActive && !bIsAirborneTouch)
	{
		TouchHoldDuration += DeltaTime;
		float Intensity = FMath::Clamp(TouchHoldDuration / 0.25f, 0.4f, 1.0f);
		Movement->SetCarveInput(TouchCarveInput * Intensity);
	}
	else if (bKeyboardCarveLeft || bKeyboardCarveRight)
	{
		KeyboardHoldDuration += DeltaTime;
		float Intensity = FMath::Clamp(KeyboardHoldDuration / 0.25f, 0.4f, 1.0f);
		float Direction = 0.0f;
		if (bKeyboardCarveLeft) Direction -= 1.0f;
		if (bKeyboardCarveRight) Direction += 1.0f;
		Movement->SetCarveInput(Direction * Intensity);
	}
	else
	{
		Movement->ReleaseCarve();
		KeyboardHoldDuration = 0.0f;
	}
}

UPowderMovementComponent* APowderPlayerController::GetMovementComp()
{
	if (!CachedMovement)
	{
		if (APowderCharacter* PowderChar = Cast<APowderCharacter>(GetPawn()))
		{
			CachedMovement = PowderChar->GetPowderMovement();
		}
	}
	return CachedMovement.Get();
}

UPowderTrickComponent* APowderPlayerController::GetTrickComp()
{
	if (!CachedTrickComp)
	{
		if (APowderCharacter* PowderChar = Cast<APowderCharacter>(GetPawn()))
		{
			CachedTrickComp = PowderChar->GetTrickComponent();
		}
	}
	return CachedTrickComp.Get();
}

void APowderPlayerController::HandleRestart()
{
	if (APowderGameMode* GM = Cast<APowderGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (GM->GetRunState() == EPowderRunState::ScoreScreen)
		{
			GM->RestartRun();
		}
	}
}

void APowderPlayerController::HandleTouchBegin(ETouchIndex::Type FingerIndex, FVector Location)
{
	// During score screen, any tap restarts the run
	if (APowderGameMode* GM = Cast<APowderGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (GM->GetRunState() == EPowderRunState::ScoreScreen)
		{
			GM->RestartRun();
			return;
		}
	}

	// Track second finger for SpreadEagle
	if (FingerIndex != ETouchIndex::Touch1)
	{
		bSecondFingerActive = true;
		BothFingersHoldTime = 0.0f;
		return;
	}

	UPowderMovementComponent* Movement = GetMovementComp();
	bool bAirborne = Movement && Movement->IsAirborne();

	if (bAirborne)
	{
		// Record swipe start for gesture detection
		bIsAirborneTouch = true;
		TouchStartPosition = FVector2D(Location.X, Location.Y);
		TouchStartTime = GetWorld()->GetTimeSeconds();
	}
	else
	{
		bIsAirborneTouch = false;
	}

	bTouchActive = true;
	TouchHoldDuration = 0.0f;
	ProcessTouchLocation(Location);
}

void APowderPlayerController::HandleTouchEnd(ETouchIndex::Type FingerIndex, FVector Location)
{
	// Second finger released
	if (FingerIndex != ETouchIndex::Touch1)
	{
		bSecondFingerActive = false;
		BothFingersHoldTime = 0.0f;
		return;
	}

	// If this was an airborne touch, classify the swipe
	if (bIsAirborneTouch)
	{
		FVector2D TouchEndPosition(Location.X, Location.Y);
		FVector2D SwipeDelta = TouchEndPosition - TouchStartPosition;
		float ElapsedTime = GetWorld()->GetTimeSeconds() - TouchStartTime;

		if (ElapsedTime <= SwipeTimeWindow && SwipeDelta.Size() >= SwipeThreshold)
		{
			EPowderGestureDirection Direction = ClassifySwipe(SwipeDelta);
			if (Direction != EPowderGestureDirection::None)
			{
				if (UPowderTrickComponent* TrickComp = GetTrickComp())
				{
					TrickComp->RequestTrick(Direction);
				}
			}
		}

		bIsAirborneTouch = false;
	}

	bTouchActive = false;
	TouchHoldDuration = 0.0f;
}

void APowderPlayerController::HandleTouchMove(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (!bIsAirborneTouch)
	{
		ProcessTouchLocation(Location);
	}
}

void APowderPlayerController::ProcessTouchLocation(const FVector& Location)
{
	int32 ViewportSizeX, ViewportSizeY;
	GetViewportSize(ViewportSizeX, ViewportSizeY);

	float ScreenMidX = ViewportSizeX * 0.5f;
	// Left half = carve left (-1), Right half = carve right (+1)
	TouchCarveInput = (Location.X < ScreenMidX) ? -1.0f : 1.0f;
}

EPowderGestureDirection APowderPlayerController::ClassifySwipe(FVector2D Delta) const
{
	// Determine dominant axis
	if (FMath::Abs(Delta.X) > FMath::Abs(Delta.Y))
	{
		// Horizontal swipe
		return (Delta.X < 0.0f) ? EPowderGestureDirection::Left : EPowderGestureDirection::Right;
	}
	else
	{
		// Vertical swipe (screen Y is inverted: negative = up)
		return (Delta.Y < 0.0f) ? EPowderGestureDirection::Up : EPowderGestureDirection::Down;
	}
}

// Keyboard handlers — route to tricks when airborne, carve when on ground
void APowderPlayerController::HandleKeyCarveLeftPressed()
{
	UPowderMovementComponent* Movement = GetMovementComp();
	if (Movement && Movement->IsAirborne())
	{
		if (UPowderTrickComponent* TrickComp = GetTrickComp())
		{
			TrickComp->RequestTrick(EPowderGestureDirection::Left);
		}
		return;
	}
	bKeyboardCarveLeft = true;
	KeyboardHoldDuration = 0.0f;
}

void APowderPlayerController::HandleKeyCarveLeftReleased()
{
	bKeyboardCarveLeft = false;
}

void APowderPlayerController::HandleKeyCarveRightPressed()
{
	UPowderMovementComponent* Movement = GetMovementComp();
	if (Movement && Movement->IsAirborne())
	{
		if (UPowderTrickComponent* TrickComp = GetTrickComp())
		{
			TrickComp->RequestTrick(EPowderGestureDirection::Right);
		}
		return;
	}
	bKeyboardCarveRight = true;
	KeyboardHoldDuration = 0.0f;
}

void APowderPlayerController::HandleKeyCarveRightReleased()
{
	bKeyboardCarveRight = false;
}

void APowderPlayerController::HandleKeyTrickUp()
{
	UPowderMovementComponent* Movement = GetMovementComp();
	if (Movement && Movement->IsAirborne())
	{
		if (UPowderTrickComponent* TrickComp = GetTrickComp())
		{
			TrickComp->RequestTrick(EPowderGestureDirection::Up);
		}
	}
}

void APowderPlayerController::HandleKeyTrickDown()
{
	UPowderMovementComponent* Movement = GetMovementComp();
	if (Movement && Movement->IsAirborne())
	{
		if (UPowderTrickComponent* TrickComp = GetTrickComp())
		{
			TrickComp->RequestTrick(EPowderGestureDirection::Down);
		}
	}
}
