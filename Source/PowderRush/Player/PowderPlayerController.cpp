#include "Player/PowderPlayerController.h"
#include "Player/PowderCharacter.h"
#include "Player/PowderMovementComponent.h"
#include "Core/PowderGameMode.h"
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

	// Determine active carve input: touch takes priority, then keyboard
	bool bAnyInput = bTouchActive || bKeyboardCarveLeft || bKeyboardCarveRight;

	if (bTouchActive)
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

		// Auto-activate boost when releasing a full meter carve
		if (Movement->GetBoostMeter() >= 1.0f)
		{
			Movement->ActivateBoost();
		}

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
