// Copyright 3DiVi 2024, Inc. All Rights Reserved.

#include "NuitrackUtils.h"
#include "NuitrackManager.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TextureResource.h"
#include "Logging/StructuredLog.h"
#include "GenericPlatform/GenericPlatformMath.h"

#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#define LOCTEXT_NAMESPACE "SNotificationButtonsWidget"

using namespace tdv::nuitrack;

namespace
{
	UTexture2D* ColorTexture;
	UTexture2D* DepthTexture;
	UTexture2D* UserTexture;

	FString LicenseErrorMessage {"To use Nuitrack, you need activate AI Subscription. https://nuitrack.com/#pricing (Perpetual and Trial are not supported in UE)"};
	bool successColorFrame {false};

	uint64_t lastDepthFrameTimestamp {0};
	uint64_t lastUserFrameTimestamp  {0};
	uint64_t lastColorFrameTimestamp {0};
}
	
void UNuitrackUtils::StopNuitrack()
{
	NuitrackManager_Stop_impl();

	ColorTexture = nullptr;
	DepthTexture = nullptr;
	UserTexture = nullptr;
}

void UNuitrackUtils::Update()
{
	if (!NuitrackManager_IsRunning()) return;
	if (!NuitrackManager_Update_impl())
	{
		UE_LOG(LogTemp, Error, TEXT("Nuitrack Update Fail: %s"), *FString(NuitrackManager_GetErrorMsg()));
		UE_LOG(LogTemp, Error, TEXT("%s"), *LicenseErrorMessage);

		FNotificationInfo Info(LOCTEXT("SpawnNotification_Notification", "To use Nuitrack, you need activate AI Subscription. Check Output Log for details"));
		Info.ExpireDuration = 30.0f;
		Info.Hyperlink = FSimpleDelegate::CreateLambda([]() {
			const FString PricingURL = TEXT("https://nuitrack.com/#pricing");
			FPlatformProcess::LaunchURL(*PricingURL, nullptr, nullptr);
			});
		Info.HyperlinkText = LOCTEXT("GoToPricing", "Go to Pricing...");
		FSlateNotificationManager::Get().AddNotification(Info);

		StopNuitrack();
	}
}

void UNuitrackUtils::StartNuitrack(bool AIMode, bool mirror, bool depthToColorReg, SensorRotationType sensorRotation, FString key)
{
	try
	{
		const char* bytes = TCHAR_TO_ANSI(*key);
		if (!NuitrackManager_Start_impl(AIMode, mirror, depthToColorReg, int(sensorRotation) * 90, bytes))
		{
			UE_LOG(LogTemp, Error, TEXT("Nuitrack Start Fail: %s"), *FString(NuitrackManager_GetErrorMsg()));
			if (NuitrackManager_GetDevicesCount_impl() == 0)
				UE_LOG(LogTemp, Error, TEXT("No connected sensors detected. Connect the sensor!"));
			UE_LOG(LogTemp, Error, TEXT("1. Is Nuitrack installed at all?"));
			UE_LOG(LogTemp, Error, TEXT("2. If Nuitrack is already installed, then try to install to the 0.38.4 version (https://github.com/3DiVi/nuitrack-sdk/releases/tag/v0.38.4)"));
			UE_LOG(LogTemp, Error, TEXT("3. %s"), *LicenseErrorMessage);
			UE_LOG(LogTemp, Error, TEXT("4. Sensor connected? Is the connection stable? Are the wires okay? If you use RealSense, then USB 3.0 is required for it to work. If the sensor is connected and you see this error, then check the sensor, the wire and the USB ports."));
			UE_LOG(LogTemp, Error, TEXT("5. Try restart PC"));
			UE_LOG(LogTemp, Error, TEXT("6. Check your Environment Variables in Windows settings."));
			UE_LOG(LogTemp, Error, TEXT("     There should be two variables \"NUITRACK_HOME\" with a path to \"Nuitrack\\nuitrack\\nutrack\" and a \"Path\" with a path to NUITRACK_HOME\\bin "));
			UE_LOG(LogTemp, Error, TEXT("     Maybe the installation probably did not complete correctly, in this case, look Troubleshooting Page. https://github.com/3DiVi/nuitrack-sdk/tree/master/doc"));
			UE_LOG(LogTemp, Error, TEXT("7. Check the read\\write permissions for the folder where Nuitrack Runtime is installed, as well as for all subfolders and files. Can you create text-file in NUITRACK_HOME\\middleware folder? Try allow Full controll permissions for Users."));

			FNotificationInfo Info(LOCTEXT("SpawnNotification_Notification", "Nuitrack Start Fail! Check Output Log for details"));
			Info.ExpireDuration = 30.0f;
			FSlateNotificationManager::Get().AddNotification(Info);
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Nuitrack Start OK"));
			UE_LOG(LogTemp, Display, TEXT("Connected devices: %d"), NuitrackManager_GetDevicesCount_impl());
		}
	}
	catch (const std::exception error) // @vadim: кажется избыточным, внутри NuitrackManager и так исключения обрабатываются
	{
		UE_LOG(LogTemp, Error, TEXT("Error:"));
		std::string errorText = error.what();

		if (errorText.find("license"))
			UE_LOGFMT(LogTemp, Error, "%s", *LicenseErrorMessage);
		else
			UE_LOGFMT(LogTemp, Error, "Nuitrack Error {0}", error.what());
	}
}

FNuitrackSkeleton UNuitrackUtils::GetSkeleton(int id)
{
	FNuitrackSkeleton skel;
	Skeleton skeleton;
	auto skeletonData = NuitrackManager_GetSkeleton_impl(id);
	if (skeletonData)
	{
		auto skels = skeletonData->getSkeletons();
		if (skels.size() > id && id >= 0)
		{
			skeleton = skels[id];
			skel.id = skeleton.id;
			skel.joints = skeleton.joints;
		}
	}
	return skel;
}

int UNuitrackUtils::GetSkeletonsCount() { return NuitrackManager_GetSkeletonsCount_impl(); }

void UNuitrackUtils::GetJoint(FNuitrackSkeleton skeleton, NuitrackJointType type, bool mirroredRotation, FTransform& transform3D, FVector& projectionPos, float& confidence)
{
	if (GetSkeletonsCount() > 0)
	{
		for (auto currentJoint : skeleton.joints)
		{
			if (int(currentJoint.type) == int(type))
			{
				transform3D.SetLocation(RealToLocation(currentJoint));
				if (mirroredRotation == false)
					transform3D.SetRotation(OrientToRotation(currentJoint));
				else
					transform3D.SetRotation(OrientToMirroredRotation(currentJoint));
				projectionPos = ProjToLocation(currentJoint);
				confidence = currentJoint.confidence;
			}
		}
	}
}

FVector UNuitrackUtils::RealToLocation(Joint joint)
{
	auto j = NuitrackManager_RealToLocation_impl(joint);
	return FVector(j.x, j.z, j.y);
}

FVector UNuitrackUtils::ProjToLocation(Joint joint)
{
	auto j = NuitrackManager_ProjToLocation_impl(joint);
	return FVector(j.x, j.z, j.y);
}

FQuat UNuitrackUtils::OrientToRotation(Joint joint)
{
	//auto jointRight = FVector(-joint.orient.matrix[0], joint.orient.matrix[6], joint.orient.matrix[3]);   //X(Right)
	auto jointForward = FVector(-joint.orient.matrix[1], joint.orient.matrix[7], joint.orient.matrix[4]);   //Y(Forward)
	auto jointUp = FVector(-joint.orient.matrix[2], joint.orient.matrix[8], joint.orient.matrix[5]);   //Z(Up)

	return FRotationMatrix::MakeFromYZ(jointUp, jointForward).ToQuat();
}

FQuat UNuitrackUtils::OrientToMirroredRotation(Joint joint)
{
	auto jointForward = FVector(joint.orient.matrix[1], -joint.orient.matrix[7], joint.orient.matrix[4]);   //Y(Forward)
	auto jointUp = FVector(-joint.orient.matrix[2], joint.orient.matrix[8], -joint.orient.matrix[5]);   //Z(Up)

	return FRotationMatrix::MakeFromYZ(jointUp, jointForward).ToQuat();
}

UTexture2D* UNuitrackUtils::GetColorTexture(bool& success)
{
	success = false;
	if (!NuitrackManager_GetColorSensor_impl()) return nullptr;

	auto ColorFramePtr = NuitrackManager_GetColorSensor_impl()->getColorFrame();
	if (!ColorFramePtr) return nullptr;

	if (ColorFramePtr->getTimestamp() == lastColorFrameTimestamp) return nullptr;
	lastColorFrameTimestamp = ColorFramePtr->getTimestamp();

	if (!successColorFrame) // @vadim: для чего?
	{
		UE_LOG(LogTemp, Display, TEXT("Color Frame test completed"));
		successColorFrame = true;
	}

	int width = ColorFramePtr->getCols(), height = ColorFramePtr->getRows();

	TArray<FColor> surfData;
	surfData.SetNum(width * height);

	auto colors = ColorFramePtr->getData();
	for (int i = 0; i < width * height; i++)
		surfData[i] = {colors[i].red, colors[i].green, colors[i].blue};

	if (surfData.Num() == 0) return nullptr;

	if (ColorTexture == nullptr)
		ColorTexture = UTexture2D::CreateTransient(width, height, PF_B8G8R8A8);

	UpdateTexture(surfData, ColorTexture);
	success = true;

	return ColorTexture;
}

UTexture2D* UNuitrackUtils::GetDepthTexture(bool& success)
{
	success = false;
	if (!NuitrackManager_GetDepthSensor_impl()) return nullptr;

	auto frame = NuitrackManager_GetDepthSensor_impl()->getDepthFrame();
	if (!frame) return nullptr;

	if (frame->getTimestamp() == lastDepthFrameTimestamp) return nullptr;
	lastDepthFrameTimestamp = frame->getTimestamp();

	int width = frame->getCols(), height = frame->getRows();

	TArray<FFloat16Color> surfData;
	surfData.SetNum(width * height);

	auto depthPtr = frame->getData();
	for (int i = 0; i < width * height; i++)
	{
		surfData[i].R = depthPtr[i];
		surfData[i].G = depthPtr[i];
		surfData[i].B = depthPtr[i];
		surfData[i].A = 255;
	}

	if (surfData.Num() == 0) return nullptr;

	if (DepthTexture == nullptr)
		DepthTexture = UTexture2D::CreateTransient(width, height, PF_FloatRGBA);

	UpdateTexture(surfData, DepthTexture);
	success = true;

	return DepthTexture;
}

TArray<int> UNuitrackUtils::GetDepthData(int& width, int& height, bool& success)
{
	success = false;

	TArray<int> data;
	if (!NuitrackManager_GetDepthSensor_impl()) return data;

	auto frame = NuitrackManager_GetDepthSensor_impl()->getDepthFrame();
	if (!frame) return data;

	auto depthData = frame->getData();
	width = frame->getCols(), height = frame->getRows();
	data.SetNum(width * height);

	// @vadim: уточнить необходимость именно TArray<int>, заменить на TArray<unsigned short> и инициализировать сырым буфером или скопировать с Memcpy
	for (int i = 0; i < width * height; i++)
		data[i] = depthData[i];

	success = true;
	return data;
}

void UNuitrackUtils::GetFloor(FVector& floor, FVector& floorNormal, float& sensorHeight, bool& success)
{
	success = false;

	auto UserFramePtr = NuitrackManager_GetUserFrame_impl();
	if (!UserFramePtr) return;
	if (UserFramePtr->getCols() == 0) return; // @vadim: без этого падения на сцене NiagaraLightDemo, требуется объяснение

	auto floorVec3 = UserFramePtr->getFloor();
	auto normalVec3 = UserFramePtr->getFloorNormal();

	// @vadim: почему здесь позиции не переведены в систему координат UE?
	floor = FVector{ floorVec3.x, floorVec3.y, floorVec3.z } * 0.1f;
	floorNormal = FVector{ normalVec3.x, normalVec3.y, normalVec3.z };

	auto dir = FVector{ 0, 0, 0 } - floor;
	success = floorNormal.Normalize(UE_KINDA_SMALL_NUMBER);
	if (success) sensorHeight = dir.Dot(floorNormal);
}

UTexture2D* UNuitrackUtils::GetUserTexture(TArray<FColor> userColors, bool& success)
{
	success = false; // @vadim: почему снаружи нельзя сделать проверку на nullptr, зачем ещё дополнительный статус нужен?

	auto UserFramePtr = NuitrackManager_GetUserFrame_impl();
	if (!UserFramePtr) return nullptr;
	if (UserFramePtr->getCols() == 0) return nullptr;
	if (UserFramePtr->getTimestamp() == lastUserFrameTimestamp) return nullptr;

	lastUserFrameTimestamp = UserFramePtr->getTimestamp();
	int width = UserFramePtr->getCols(), height = UserFramePtr->getRows();

	TArray<FColor> surfData;
	surfData.SetNum(width * height);

	// @vadim: спорное решение, почему userColors прилетает откуда-то извне, а не является глобальным?
	if (userColors.Num() < 7)
	{
		userColors.Add(FColor::Black);
		userColors.Add(FColor::Red);
		userColors.Add(FColor::Blue);
		userColors.Add(FColor::Green);
		userColors.Add(FColor::Cyan);
		userColors.Add(FColor::Emerald);
		userColors.Add(FColor::Magenta);		
	}

	for (int i = 0; i < width * height; i++)
	{
		auto colors = UserFramePtr->getData();
		surfData[i] = {userColors[colors[i]].R, userColors[colors[i]].G, userColors[colors[i]].B};
	}

	if (UserTexture == nullptr)
		UserTexture = UTexture2D::CreateTransient(width, height, PF_B8G8R8A8);

	UpdateTexture(surfData, UserTexture);
	success = true;

	return UserTexture;
}

TArray<FNuitrackUser> UNuitrackUtils::GetUsers()
{
	auto handData = NuitrackManager_GetHandTrackerData_impl();
	auto GestureDataPtr = NuitrackManager_GetGestureData_impl();

	TArray<FNuitrackUser> users;
	int usersCount = GetSkeletonsCount();

	for (int i = 0; i < usersCount; i++)
	{
		FNuitrackUser user;
		user.skeleton = GetSkeleton(i);
		user.userId = GetSkeleton(i).id;
		user.gestureType = NuitrackGestureType::NONE;

		if (GestureDataPtr)
		{
			for (auto gestures : GestureDataPtr->getGestures())
			{
				if (user.userId == gestures.userId)
					user.gestureType = (NuitrackGestureType)(int)gestures.type;
			}
		}

		if (handData)
		{
			for (auto userHands : handData->getUsersHands())
			{
				if (user.userId == userHands.userId)
				{
					auto lHand = userHands.leftHand;
					auto rHand = userHands.rightHand;

					user.leftHand.x = lHand->x;
					user.leftHand.y = lHand->y;
					user.leftHand.click = lHand->click;
					user.leftHand.pressure = lHand->pressure;
					user.leftHand.realLocation = FVector(lHand->xReal, lHand->yReal, lHand->zReal);

					user.rightHand.x = rHand->x;
					user.rightHand.y = rHand->y;
					user.rightHand.click = rHand->click;
					user.rightHand.pressure = rHand->pressure;
					user.rightHand.realLocation = FVector(rHand->xReal, rHand->yReal, rHand->zReal);
				}
			}
		}
		users.Add(user);
	}

	return users;
}

TArray<FNuitrackDevice> UNuitrackUtils::GetDevices()
{
	/* пустая/заглушечная реализация как и сам FNuitrackDevice */
	TArray<FNuitrackDevice> nuitrackDevices;
	auto devCount = NuitrackManager_GetDevicesCount_impl();
	for (int i = 0; i < devCount; i++) nuitrackDevices.Add({}); 
	return nuitrackDevices;
}

void UNuitrackUtils::UpdateTexture(TArray<FColor> surfData, UTexture2D* texturePtr)
{
#if WITH_EDITORONLY_DATA
	texturePtr->MipGenSettings = TMGS_NoMipmaps;
#endif

	void* TextureData = texturePtr->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	const int32 TextureDataSize = surfData.Num() * 4;
	FMemory::Memcpy(TextureData, surfData.GetData(), TextureDataSize);
	texturePtr->GetPlatformData()->Mips[0].BulkData.Unlock();
	texturePtr->UpdateResource();
}

void UNuitrackUtils::UpdateTexture(TArray<FFloat16Color> surfData, UTexture2D* texturePtr)
{
#if WITH_EDITORONLY_DATA
	texturePtr->MipGenSettings = TMGS_NoMipmaps;
#endif
	texturePtr->Filter = TF_Nearest;

	auto* TextureData = texturePtr->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);

	auto TextureDataSize = surfData.Num() * 8;
	FMemory::Memcpy(TextureData, surfData.GetData(), TextureDataSize);

	texturePtr->GetPlatformData()->Mips[0].BulkData.Unlock();
	texturePtr->UpdateResource();
}

FString UNuitrackUtils::GetInstancesJson() { return NuitrackManager_GetInstancesJson_impl(); }

float UNuitrackUtils::GetFOV(FOVType fovType)
{
	if (!NuitrackManager_GetDepthSensor_impl()) return 0;

	auto outputMode = NuitrackManager_GetDepthSensor_impl()->getOutputMode();
	if (fovType == FOVType::HORIZONTAL_FOV) return outputMode.hfov;
	return 2.0f * FGenericPlatformMath::Atan(outputMode.intrinsics.cy / outputMode.intrinsics.fy);
}