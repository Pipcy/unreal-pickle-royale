// Copyright 3DiVi 2024, Inc. All Rights Reserved.

#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "Engine/Texture2D.h"
#include "RHITypes.h" // FUpdateTextureRegion2D

#include "PointCloud.generated.h"

UCLASS()
class APointCloud : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APointCloud();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nuitrack")
	UNiagaraSystem* pointCloudRenderer = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nuitrack")
	UTexture2D* positionTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nuitrack")
	UTexture2D* colorTexture = nullptr;

private:
	UNiagaraComponent* rendererInst = nullptr;
	uint32_t pointCount = 0;
	std::vector<FFloat16> positions;
	std::vector<uint8_t> colors;
	FUpdateTextureRegion2D region;
	uint64_t lastDepthFrameTimestamp = 0;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void InitPointCloud(int width, int height);

	UFUNCTION(BlueprintCallable, Category = "Nuitrack")
	void UpdatePointCloud(bool userMask, FVector position, FRotator rotation);
};
