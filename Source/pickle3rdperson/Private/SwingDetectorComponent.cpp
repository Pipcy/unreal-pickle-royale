#include "SwingDetectorComponent.h"

USwingDetectorComponent::USwingDetectorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USwingDetectorComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool USwingDetectorComponent::PushJointPosition(float CurrentPosition)
{
    // First sample: initialize only, cannot compute velocity yet.
    if (!bHasPreviousPosition)
    {
        bHasPreviousPosition = true;
        PreviousPosition = CurrentPosition;
        SamplesRemainingInCooldown = 0;

        RecentPositions.Empty();
        RecentVelocities.Empty();

        RecentPositions.Add(CurrentPosition);

        LastVelocity = 0.0f;
        LastCumulativeVelocity = 0.0f;
        LastRecentMax = CurrentPosition;
        LastDropFromMax = 0.0f;
        bWasConditionTrueLastSample = false;

        return false;
    }

    // 1) velocity = current_pos - prev_pos
    const float Velocity = CurrentPosition - PreviousPosition;
    LastVelocity = Velocity;

    // Add current values into history
    RecentVelocities.Add(Velocity);
    TrimArrayToWindow(RecentVelocities);

    RecentPositions.Add(CurrentPosition);
    TrimArrayToWindow(RecentPositions);

    // Update previous position for next sample
    PreviousPosition = CurrentPosition;

    // 2) cumulative_velocity = sum of previous 5 velocity values
    float CumulativeVelocity = 0.0f;
    for (const float Value : RecentVelocities)
    {
        CumulativeVelocity += Value;
    }
    LastCumulativeVelocity = CumulativeVelocity;

    // 3) recent_max = max of previous 5 position values
    float RecentMax = CurrentPosition;
    if (RecentPositions.Num() > 0)
    {
        RecentMax = RecentPositions[0];
        for (const float Value : RecentPositions)
        {
            if (Value > RecentMax)
            {
                RecentMax = Value;
            }
        }
    }
    LastRecentMax = RecentMax;

    // 4) drop_from_max = recent_max - current_pos
    const float DropFromMax = RecentMax - CurrentPosition;
    LastDropFromMax = DropFromMax;

    // 5) detection
    const bool bConditionTrue =
        (CumulativeVelocity < CumulativeVelocityThreshold) &&
        (DropFromMax > DropFromMaxThreshold);

    // Rising edge only
    const bool bRisingEdgeThisSample = bConditionTrue && !bWasConditionTrueLastSample;

    // Update latch for next sample
    bWasConditionTrueLastSample = bConditionTrue;

    // Count cooldown down once per sample
    if (SamplesRemainingInCooldown > 0)
    {
        --SamplesRemainingInCooldown;
    }

    // Only allow a detection if we're not cooling down
    const bool bCanTrigger = (SamplesRemainingInCooldown <= 0);
    const bool bDetectedThisSample = bRisingEdgeThisSample && bCanTrigger;

    // Start cooldown after a successful detection
    if (bDetectedThisSample)
    {
        SamplesRemainingInCooldown = FMath::Max(0, CooldownSamples);
    }

    return bDetectedThisSample;
}

void USwingDetectorComponent::ResetDetector()
{
    bHasPreviousPosition = false;
    PreviousPosition = 0.0f;

    RecentVelocities.Empty();
    RecentPositions.Empty();

    LastVelocity = 0.0f;
    LastCumulativeVelocity = 0.0f;
    LastRecentMax = 0.0f;
    LastDropFromMax = 0.0f;
    bWasConditionTrueLastSample = false;
    SamplesRemainingInCooldown = 0;
}

void USwingDetectorComponent::TrimArrayToWindow(TArray<float>& ArrayToTrim)
{
    const int32 SafeWindowSize = FMath::Max(1, WindowSize);

    while (ArrayToTrim.Num() > SafeWindowSize)
    {
        ArrayToTrim.RemoveAt(0);
    }
}