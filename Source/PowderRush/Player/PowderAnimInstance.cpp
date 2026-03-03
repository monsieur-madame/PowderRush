#include "Player/PowderAnimInstance.h"
#include "Player/PowderMovementComponent.h"

void UPowderAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
	{
		return;
	}

	UPowderMovementComponent* MoveComp = Pawn->FindComponentByClass<UPowderMovementComponent>();
	if (!MoveComp)
	{
		return;
	}

	// Carve lean: normalized carve angle -1 (left) to +1 (right)
	float MaxCarve = MoveComp->MaxCarveAngle;
	CarveLean = (MaxCarve > KINDA_SMALL_NUMBER)
		? FMath::Clamp(MoveComp->GetCarveAngle() / MaxCarve, -1.0f, 1.0f)
		: 0.0f;

	EdgeDepth = MoveComp->GetEdgeDepth();
	SpeedNorm = MoveComp->GetSpeedNormalized();
	bAirborne = MoveComp->IsAirborne();
	bCarving = MoveComp->IsCarving();

	// Slope pitch in heading direction: project slope normal onto heading plane
	FVector Normal = MoveComp->GetSlopeNormal();
	float DesiredYaw = MoveComp->GetDesiredYaw();
	FVector HeadingDir = FRotator(0.0f, DesiredYaw, 0.0f).Vector();
	// Pitch of the slope along heading: angle between the slope surface and horizontal
	// Dot of normal with up gives cos(slope angle), but we want pitch in heading direction
	float NormalDotHeading = FVector::DotProduct(Normal, HeadingDir);
	float NormalDotUp = FVector::DotProduct(Normal, FVector::UpVector);
	if (NormalDotUp > KINDA_SMALL_NUMBER)
	{
		SlopePitch = FMath::RadiansToDegrees(FMath::Atan2(-NormalDotHeading, NormalDotUp));
	}
	else
	{
		SlopePitch = 0.0f;
	}
}
