#include "CSVFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

bool UCSVFileManager::AppendStringToFile(FString FilePath, FString StringToAppend)
{
    // Load existing content if file exists
    FString ExistingContent;
    if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
    {
        FFileHelper::LoadFileToString(ExistingContent, *FilePath);
    }
    ExistingContent += StringToAppend + LINE_TERMINATOR;
    return FFileHelper::SaveStringToFile(ExistingContent, *FilePath);
}