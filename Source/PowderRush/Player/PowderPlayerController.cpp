#include "Player/PowderPlayerController.h"
#include "Player/PowderCharacter.h"
#include "Player/PowderMovementComponent.h"
#include "Player/PowderTrickComponent.h"
#include "Core/PowderGameMode.h"
#include "Core/PowderTypes.h"
#include "UI/PowderHUD.h"
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

	// Ollie (Space key for desktop testing)
	InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &APowderPlayerController::HandleOllie);

	// Pause toggle (Escape)
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &APowderPlayerController::HandlePauseToggle);

	// Restart key for desktop testing
	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &APowderPlayerController::HandleRestart);
}

void APowderPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Skip carve processing when not running
	APowderGameMode* GM = Cast<APowderGameMode>(GetWorld()->GetAuthGameMode());
	if (GM && GM->GetRunState() != EPowderRunState::Running)
	{
		return;
	}

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
	bool bHasRequestedInput = false;
	float RequestedInput = 0.0f;

	if (bTouchActive && !bIsAirborneTouch)
	{
		TouchHoldDuration += DeltaTime;
		const float RampAlpha = FMath::Clamp(TouchHoldDuration / Movement->CarveRampTime, 0.0f, 1.0f);
		const float EasedRamp = FMath::Lerp(
			Movement->CarveRampMinIntensity,
			1.0f,
			FMath::Pow(RampAlpha, Movement->CarveRampEaseExponent));
		RequestedInput = TouchCarveInput * EasedRamp;
		bHasRequestedInput = true;
		KeyboardHoldDuration = 0.0f;
	}
	else if (bKeyboardCarveLeft || bKeyboardCarveRight)
	{
		KeyboardHoldDuration += DeltaTime;
		const float RampAlpha = FMath::Clamp(KeyboardHoldDuration / Movement->CarveRampTime, 0.0f, 1.0f);
		const float EasedRamp = FMath::Lerp(
			Movement->CarveRampMinIntensity,
			1.0f,
			FMath::Pow(RampAlpha, Movement->CarveRampEaseExponent));
		float Direction = 0.0f;
		if (bKeyboardCarveLeft)
		{
			Direction -= 1.0f;
		}
		if (bKeyboardCarveRight)
		{
			Direction += 1.0f;
		}
		RequestedInput = Direction * EasedRamp;
		bHasRequestedInput = true;
	}
	else
	{
		KeyboardHoldDuration = 0.0f;
	}

	CarveSideSwitchCooldownTimer = FMath::Max(0.0f, CarveSideSwitchCooldownTimer - DeltaTime);

	if (bHasRequestedInput)
	{
		const float RequestedSign = FMath::Sign(RequestedInput);
		if (RequestedSign != 0.0f && LastRequestedCarveSign != 0.0f && RequestedSign != LastRequestedCarveSign)
		{
			CarveSideSwitchCooldownTimer = FMath::Max(CarveSideSwitchCooldownTimer, CarveSideSwitchCooldown);
		}
		LastRequestedCarveSign = RequestedSign;
		CarveReleaseGraceTimer = CarveReleaseGraceTime;

		float AppliedInput = RequestedInput;
		if (CarveSideSwitchCooldownTimer > 0.0f && CarveSideSwitchCooldown > KINDA_SMALL_NUMBER)
		{
			const float SuppressionAlpha = FMath::Clamp(CarveSideSwitchCooldownTimer / CarveSideSwitchCooldown, 0.0f, 1.0f);
			const float DampedScale = FMath::Lerp(1.0f, 0.35f, SuppressionAlpha);
			AppliedInput *= DampedScale;
		}

		EffectiveCarveInput = AppliedInput;
		Movement->SetCarveInput(EffectiveCarveInput);
	}
	else
	{
		LastRequestedCarveSign = 0.0f;
		if (CarveReleaseGraceTimer > 0.0f)
		{
			CarveReleaseGraceTimer = FMath::Max(0.0f, CarveReleaseGraceTimer - DeltaTime);
			if (CarveReleaseGraceTime > KINDA_SMALL_NUMBER)
			{
				const float GraceAlpha = FMath::Clamp(CarveReleaseGraceTimer / CarveReleaseGraceTime, 0.0f, 1.0f);
				EffectiveCarveInput *= GraceAlpha;
				Movement->SetCarveInput(EffectiveCarveInput);
			}
			else
			{
				EffectiveCarveInput = 0.0f;
				Movement->ReleaseCarve();
			}
		}
		else
		{
			EffectiveCarveInput = 0.0f;
			Movement->ReleaseCarve();
		}
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

APowderHUD* APowderPlayerController::GetPowderHUD()
{
	return Cast<APowderHUD>(GetHUD());
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

void APowderPlayerController::HandlePauseToggle()
{
	APowderGameMode* GM = Cast<APowderGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM)
	{
		return;
	}

	if (GM->GetRunState() == EPowderRunState::Running)
	{
		GM->PauseRun();
	}
	else if (GM->GetRunState() == EPowderRunState::Paused)
	{
		GM->ResumeRun();
	}
}

void APowderPlayerController::HandleTouchBegin(ETouchIndex::Type FingerIndex, FVector Location)
{
	APowderGameMode* GM = Cast<APowderGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM)
	{
		return;
	}

	EPowderRunState State = GM->GetRunState();

	// Route to menu tap for non-running states
	if (State == EPowderRunState::InMenu || State == EPowderRunState::Paused || State == EPowderRunState::ScoreScreen)
	{
		if (APowderHUD* HUD = GetPowderHUD())
		{
			HUD->OnMenuTap(Location.X, Location.Y);
		}
		return;
	}

	// During running: check pause button area (top-left)
	if (State == EPowderRunState::Running)
	{
		if (APowderHUD* HUD = GetPowderHUD())
		{
			if (HUD->IsPauseAreaHit(Location.X, Location.Y))
			{
				GM->PauseRun();
				return;
			}
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

		// Double-tap detection for ollie (grounded only)
		float Now = GetWorld()->GetTimeSeconds();
		if (Movement && (Now - LastTapTime) < DoubleTapWindow)
		{
			Movement->Ollie();
		}
		LastTapTime = Now;
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

void APowderPlayerController::HandleOllie()
{
	if (UPowderMovementComponent* Movement = GetMovementComp())
	{
		Movement->Ollie();
	}
}
