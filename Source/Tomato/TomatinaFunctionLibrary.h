// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TomatinaSoundCue.h"     // FTomatinaSoundCue
#include "TomatinaTargetBase.h"
#include "TomatinaFunctionLibrary.generated.h"

class APlayerController;
class USceneCaptureComponent2D;
class UCameraComponent;
class UTextureRenderTarget2D;
class USoundBase;

UENUM(BlueprintType)
enum class EPhotoFramingType : uint8
{
	None      UMETA(DisplayName="None"),
	FullBody  UMETA(DisplayName="Full Body"),
	UpperBody UMETA(DisplayName="Upper Body"),
	LowerBody UMETA(DisplayName="Lower Body"),
	Invalid   UMETA(DisplayName="Invalid"),
};

USTRUCT(BlueprintType)
struct FPhotoFramingPreviewResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bHasTarget = false;

	UPROPERTY(BlueprintReadOnly)
	EPhotoFramingType FramingType = EPhotoFramingType::None;

	UPROPERTY(BlueprintReadOnly)
	float ScoreMultiplier = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 Score = 0;

	UPROPERTY(BlueprintReadOnly)
	ATomatinaTargetBase* BestTarget = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FString Comment;
};

USTRUCT(BlueprintType)
struct FPhotoResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Score = 0;

	UPROPERTY(BlueprintReadOnly)
	ATomatinaTargetBase* BestTarget = nullptr;

	UPROPERTY(BlueprintReadOnly)
	EPhotoFramingType FramingType = EPhotoFramingType::None;

	UPROPERTY(BlueprintReadOnly)
	float ScoreMultiplier = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	FString Comment;
};

UCLASS()
class TOMATO_API UTomatinaFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Tomatina|Visibility")
	static bool CheckVisibility(
		USceneCaptureComponent2D* ZoomCamera,
		AActor* Target,
		FVector CheckLoc,
		float ScreenWidth,
		float ScreenHeight);

	UFUNCTION(BlueprintCallable, Category="Tomatina|Photo")
	static FPhotoResult CalculatePhotoScore(
		USceneCaptureComponent2D* ZoomCamera,
		const TArray<ATomatinaTargetBase*>& Targets,
		FName CurrentMission,
		float ScreenWidth,
		float ScreenHeight);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Tomatina|Photo")
	static FPhotoFramingPreviewResult EvaluatePhotoFraming(
		USceneCaptureComponent2D* ZoomCamera,
		const TArray<ATomatinaTargetBase*>& Targets,
		FName CurrentMission,
		float ScreenWidth,
		float ScreenHeight);

	UFUNCTION(BlueprintCallable, Category="Tomatina|Camera")
	static FVector CalculateZoomOffset(
		APlayerController* PC,
		const FHitResult& HitResult,
		UCameraComponent* Camera,
		float CameraFOV = 90.f);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Tomatina|Camera")
	static FVector2D GetZoomScreenCenter(
		float MainWidth,
		float PhoneWidth,
		float PhoneHeight);

	UFUNCTION(BlueprintCallable, Category="Tomatina|Camera")
	static FVector2D ProjectZoomToMainScreen(
		APlayerController* PC,
		USceneCaptureComponent2D* SceneCapture,
		float MainWidth,
		float MainHeight);

	UFUNCTION(BlueprintCallable, Category="Tomatina|Photo")
	static void CopyZoomToPhoto(
		USceneCaptureComponent2D* ZoomCamera,
		UTextureRenderTarget2D* PhotoTarget);

	UFUNCTION(BlueprintCallable, Category="Tomatina|Debug")
	static void SetUtilityDebugLogEnabled(bool bEnabled);

	/** FTomatinaSoundCue を 2D 再生（遅延・音量・ピッチ対応） */
	UFUNCTION(BlueprintCallable, Category="Tomatina|Audio", meta=(WorldContext="WorldContextObject"))
	static void PlayTomatinaCue2D(
		const UObject* WorldContextObject,
		const FTomatinaSoundCue& Cue);

	/** FTomatinaSoundCue を指定ワールド座標で 3D 再生（遅延・音量・ピッチ対応） */
	UFUNCTION(BlueprintCallable, Category="Tomatina|Audio", meta=(WorldContext="WorldContextObject"))
	static void PlayTomatinaCueAtLocation(
		const UObject* WorldContextObject,
		const FTomatinaSoundCue& Cue,
		FVector Location);

private:
	static bool bUtilityDebugLogEnabled;
};
