#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CSVFileManager.generated.h"

UCLASS()
class PICKLE3RDPERSON_API UCSVFileManager : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "CSV")
    static bool AppendStringToFile(FString FilePath, FString StringToAppend);
};