// Copyright 3DiVi 2024, Inc. All Rights Reserved.

#include "PointCloud.h"
#include "NuitrackManager.h"
#include "nuitrack/Nuitrack.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterFaceTexture.h"

using namespace tdv::nuitrack;

APointCloud::APointCloud()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void SetNiagaraVariableTexture(class UNiagaraComponent* niagara, FString variableName, UTexture* texture)
{
	if (!niagara || !texture) return;

	FNiagaraUserRedirectionParameterStore& overrideParameters = niagara->GetOverrideParameters();
	auto niagaraVar = FNiagaraVariable(FNiagaraTypeDefinition(UNiagaraDataInterfaceTexture::StaticClass()), *variableName);

	auto dataInterface = (UNiagaraDataInterfaceTexture*)overrideParameters.GetDataInterface(niagaraVar);
	dataInterface->SetTexture(texture);
}

void APointCloud::BeginPlay() { Super::BeginPlay(); }

void APointCloud::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void APointCloud::InitPointCloud(int width, int height)
{
	if (!pointCloudRenderer) return;

	pointCount = width * height;

	positions.resize(pointCount * 4);
	colors.resize(pointCount * 4);

	UE_LOG(LogTemp, Display, TEXT("Init Point Cloud Start"));
	UE_LOG(LogTemp, Display, TEXT("Width %i"), width);
	UE_LOG(LogTemp, Display, TEXT("Height %i"), height);
	UE_LOG(LogTemp, Display, TEXT("Total Points %i"), pointCount);

	region = FUpdateTextureRegion2D(0, 0, 0, 0, width, height);

	rendererInst = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, pointCloudRenderer, FVector(0, 0, 0), FRotator(0), FVector(1), true, true, ENCPoolMethod::AutoRelease, true);
	positionTexture = UTexture2D::CreateTransient(width, height, PF_FloatRGBA, "PositionData");

#if WITH_EDITORONLY_DATA
	positionTexture->MipGenSettings = TMGS_NoMipmaps;
#endif
	positionTexture->Filter = TF_Nearest;
	positionTexture->UpdateResource();

	colorTexture = UTexture2D::CreateTransient(width, height, PF_B8G8R8A8, "ColorTexture");
	colorTexture->Filter = TF_Nearest;
	colorTexture->UpdateResource();

	SetNiagaraVariableTexture(rendererInst, "User.PositionTexture", positionTexture);
	SetNiagaraVariableTexture(rendererInst, "User.ColorTexture", colorTexture);

	rendererInst->SetVariableInt("User.TextureWidth", width);
	rendererInst->SetVariableInt("User.TextureHeight", height);
	rendererInst->SetVariableInt("User.ParticlesCount", pointCount);

	UE_LOG(LogTemp, Display, TEXT("Init Point Cloud Success"));
}

void APointCloud::UpdatePointCloud(bool userMask, FVector position, FRotator rotation)
{
	if (!NuitrackManager_GetDepthSensor_impl()) return;

	auto outputMode = NuitrackManager_GetDepthSensor_impl()->getOutputMode();
	auto frame = NuitrackManager_GetDepthSensor_impl()->getDepthFrame();
	if (!frame)
	{
		UE_LOG(LogTemp, Display, TEXT("No Depth Frame"));
		return;
	}

	if (frame->getTimestamp() == lastDepthFrameTimestamp) return;
	lastDepthFrameTimestamp = frame->getTimestamp();

	auto depthPtr = frame->getData();
	int frameWidth = frame->getCols(), frameHeight = frame->getRows();

	if (frameWidth == 0 || frameHeight == 0)
	{
		UE_LOG(LogTemp, Display, TEXT("Zero Depth Frame"));
		return;
	}
	if (!depthPtr)
	{
		UE_LOG(LogTemp, Display, TEXT("No Depth Data"));
		return;
	}

	const uint16_t* UserPtr = nullptr;
	if (userMask)
	{
		auto userFramePtr = NuitrackManager_GetUserFrame_impl();
		if (!userFramePtr) return;
		UserPtr = userFramePtr->getData();
	}
	
	if (!rendererInst)
	{
		InitPointCloud(frameWidth, frameHeight);
		return;
	}

	for (int y = 0, texturePixelId = 0; y < frameHeight; ++y)
	{
		for (int x = 0; x < frameWidth; ++x, ++texturePixelId)
		{
			auto pixelValue = depthPtr[texturePixelId] * 0.1f;
			int currentPixelId = texturePixelId * 4;

			positions[currentPixelId + 0] = pixelValue * (x - outputMode.intrinsics.cx) / float(-outputMode.intrinsics.fx); //x
			positions[currentPixelId + 1] = pixelValue; //y
			positions[currentPixelId + 2] = -pixelValue * (y - outputMode.intrinsics.cy) / float(outputMode.intrinsics.fy); //z
			positions[currentPixelId + 3] = 1;

			if (userMask)
			{
				colors[currentPixelId + 0] = UserPtr[texturePixelId];
				colors[currentPixelId + 1] = UserPtr[texturePixelId];
				colors[currentPixelId + 2] = UserPtr[texturePixelId];
				colors[currentPixelId + 3] = 255;
			}
			else
			{
				colors[currentPixelId + 0] = 0;
				colors[currentPixelId + 1] = uint8_t(float(y) / frameHeight * 255);
				colors[currentPixelId + 2] = uint8_t(float(x) / frameWidth * 255);
				colors[currentPixelId + 3] = 255;
			}
		}
	}

	positionTexture->UpdateTextureRegions(0, 1, &region, frameWidth * 8, 8, (uint8*)positions.data());
	colorTexture->UpdateTextureRegions(0, 1, &region, frameWidth * 4, 4, (uint8*)colors.data());

	rendererInst->SetWorldRotation(rotation);
	rendererInst->SetWorldLocation(position);
}