// Copyright 3DiVi 2024, Inc. All Rights Reserved.

#pragma once

#include <vector>

#include "nuitrack/Nuitrack.h"

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NuitrackUtils.generated.h"

UENUM()
enum class NuitrackGestureType : uint8
{
	GESTURE_WAVING,
	GESTURE_SWIPE_LEFT,
	GESTURE_SWIPE_RIGHT,
	GESTURE_SWIPE_UP,
	GESTURE_SWIPE_DOWN,
	GESTURE_PUSH,
	NONE,
};

UENUM(BlueprintType)
enum class NuitrackJointType : uint8
{
	JOINT_NONE, ///< Reserved joint (unused).

	JOINT_HEAD, ///< Head
	JOINT_NECK, ///< Neck
	JOINT_TORSO, ///< Torso
	JOINT_WAIST, ///< Waist

	JOINT_LEFT_COLLAR, ///< Left collar
	JOINT_LEFT_SHOULDER, ///< Left shoulder
	JOINT_LEFT_ELBOW, ///< Left elbow
	JOINT_LEFT_WRIST, ///< Left wrist
	JOINT_LEFT_HAND, ///< Left hand
	JOINT_LEFT_FINGERTIP, ///< Left fingertip (<b>not used in the current version</b>).

	JOINT_RIGHT_COLLAR, ///< Right collar
	JOINT_RIGHT_SHOULDER, ///< Right shoulder
	JOINT_RIGHT_ELBOW, ///< Right elbow
	JOINT_RIGHT_WRIST, ///< Right wrist
	JOINT_RIGHT_HAND, ///< Right hand
	JOINT_RIGHT_FINGERTIP, ///< Right fingertip (<b>not used in the current version</b>).

	JOINT_LEFT_HIP, ///< Left hip
	JOINT_LEFT_KNEE, ///< Left knee
	JOINT_LEFT_ANKLE, ///< Left ankle
	JOINT_LEFT_FOOT, ///< Left foot (<b>not used in the current version</b>).

	JOINT_RIGHT_HIP, ///< Right hip
	JOINT_RIGHT_KNEE, ///< Right knee
	JOINT_RIGHT_ANKLE, ///< Right ankle
	JOINT_RIGHT_FOOT ///< Right foot (<b>not used in the current version</b>).
};

USTRUCT(BlueprintType)
struct FNuitrackSkeleton
{
	GENERATED_BODY()
	/**
	* @brief User Id. The same as other(@ref UserTracker, @ref HandTracker, @ref GestureRecognizer) modules uses.
	*/
	UPROPERTY()
	int id = 0;

	/**
	* @brief Array of joints. Where each index is ::JointType.
	*/
	std::vector<tdv::nuitrack::Joint> joints;
};

USTRUCT(BlueprintType)
struct FNuitrackDevice
{
	GENERATED_BODY()

	UPROPERTY()
	int id = 0;
};

USTRUCT(BlueprintType)
struct FNuitrackHand
{
	GENERATED_BODY()

	/**
	* @brief The normalized projective x coordinate of the hand (in range [0, 1]).
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nuitrack")
	float x = 0;

	/**
	* @brief The normalized projective y coordinate of the hand (in range [0, 1]).
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nuitrack")
	float y = 0;

	/**
	* Coordinates of the hand in the world system.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nuitrack")
	FVector realLocation = FVector::ZeroVector;

	/**
	* @brief True if the hand makes a click, false otherwise.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nuitrack")
	bool click = false;

	/**
	* @brief Rate of hand clenching.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nuitrack")
	int pressure = 0;
};

USTRUCT(BlueprintType)
struct FNuitrackUser
{
	GENERATED_BODY()

	/**
	* @brief User Id. The same as other(@ref UserTracker, @ref HandTracker, @ref GestureRecognizer) modules uses.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nuitrack")
	int userId = 0;

	/**
	* @brief Get Skeleton struct
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nuitrack")
	FNuitrackSkeleton skeleton;

	/**
	* @brief Get Gesture type
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nuitrack")
	NuitrackGestureType gestureType = NuitrackGestureType::NONE;

	/**
	* @brief Get Left Hand struct
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nuitrack")
	FNuitrackHand leftHand;

	/**
	* @brief Get Right Hand struct
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nuitrack")
	FNuitrackHand rightHand;
};

USTRUCT(BlueprintType)
struct FNuitrackJoint
{
	GENERATED_BODY()

	NuitrackJointType type = NuitrackJointType::JOINT_NONE;

	UPROPERTY()
	float confidence = 0;

	UPROPERTY()
	FVector real = FVector::ZeroVector;

	UPROPERTY()
	FVector proj = FVector::ZeroVector;

	UPROPERTY()
	FRotator rot = FRotator::ZeroRotator;
};

UENUM(BlueprintType)
enum class SensorRotationType : uint8
{
	NORMAL,
	PORTRAIT_LEFT,
	UPSIDE_DOWN,
	PORTRAIT_RIGHT,
};

UENUM(BlueprintType)
enum class FOVType : uint8
{
	HORIZONTAL_FOV,
	VERTICAL_FOV,
};

UCLASS()
class UNuitrackUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	static void Update();

	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	static void StartNuitrack(bool AIMode, bool mirror, bool depthToColorReg, SensorRotationType sensorRotation, FString key);

	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	static void StopNuitrack();

	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	static UTexture2D* GetColorTexture(bool& success);

	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	static UTexture2D* GetDepthTexture(bool& success);

	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	static TArray<int> GetDepthData(int& width, int& height, bool& success);

	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	static void GetFloor(FVector& floor, FVector& floorNormal, float& sensorHeight, bool& success);

	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	static UTexture2D* GetUserTexture(TArray<FColor> userColors, bool& success);

	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	static void GetJoint(FNuitrackSkeleton skeleton, NuitrackJointType type, bool mirroredRotation, FTransform& transform3D, FVector& projectionPos, float& confidence);

	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	static TArray<FNuitrackUser> GetUsers();

	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	static TArray<FNuitrackDevice> GetDevices();

	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	static FString GetInstancesJson();

	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	static float GetFOV(FOVType fovType);

	static int GetSkeletonsCount();
	static FNuitrackSkeleton GetSkeleton(int id);

	static FVector RealToLocation(tdv::nuitrack::Joint joint);
	static FVector ProjToLocation(tdv::nuitrack::Joint joint);
	static FQuat OrientToRotation(tdv::nuitrack::Joint joint);
	static FQuat OrientToMirroredRotation(tdv::nuitrack::Joint joint);
	static void UpdateTexture(TArray<FColor> surfData, UTexture2D* texturePtr);
	static void UpdateTexture(TArray<FFloat16Color> surfData, UTexture2D* texturePtr);
};
