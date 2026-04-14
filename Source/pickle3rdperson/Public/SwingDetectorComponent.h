#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SwingDetectorComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PICKLE3RDPERSON_API USwingDetectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USwingDetectorComponent();

    // Push one new position sample into the detector.
    // Returns true if the current sample satisfies the swing condition.
    UFUNCTION(BlueprintCallable, Category = "Swing Detection")
    bool PushJointPosition(float CurrentPosition);

    // Clears all history so detection starts fresh
    UFUNCTION(BlueprintCallable, Category = "Swing Detection")
    void ResetDetector();

    // Optional helpers for debugging in Blueprint
    UFUNCTION(BlueprintPure, Category = "Swing Detection")
    float GetLastVelocity() const { return LastVelocity; }

    UFUNCTION(BlueprintPure, Category = "Swing Detection")
    float GetLastCumulativeVelocity() const { return LastCumulativeVelocity; }

    UFUNCTION(BlueprintPure, Category = "Swing Detection")
    float GetLastRecentMax() const { return LastRecentMax; }

    UFUNCTION(BlueprintPure, Category = "Swing Detection")
    float GetLastDropFromMax() const { return LastDropFromMax; }

    UFUNCTION(BlueprintPure, Category = "Swing Detection")
    bool HasValidHistory() const { return bHasPreviousPosition; }

protected:
    virtual void BeginPlay() override;

    // Number of previous samples used for velocity sum / recent max
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Detection")
    int32 WindowSize = 5;

    // Sspreadsheet thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Detection")
    float CumulativeVelocityThreshold = -0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Detection")
    float DropFromMaxThreshold = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swing Detection")
    int32 CooldownSamples = 10; // Cooldown

private:
    bool bHasPreviousPosition = false;
    float PreviousPosition = 0.0f;
    bool bWasConditionTrueLastSample = false; // for rising edge check

    int32 SamplesRemainingInCooldown = 0; // Cooldown

    TArray<float> RecentVelocities;
    TArray<float> RecentPositions;

    float LastVelocity = 0.0f;
    float LastCumulativeVelocity = 0.0f;
    float LastRecentMax = 0.0f;
    float LastDropFromMax = 0.0f;

    void TrimArrayToWindow(TArray<float>& ArrayToTrim);
};