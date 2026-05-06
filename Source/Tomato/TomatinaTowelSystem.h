// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TomatinaSoundCue.h"
#include "TomatinaTowelSystem.generated.h"

class ATomatoDirtManager;
class ATomatinaHUD;
class ATomatinaPlayerPawn;
class USoundBase;
class UAudioComponent;
class ULeapComponent;

UENUM(BlueprintType)
enum class EHandSmoothingMode : uint8
{
	None    UMETA(DisplayName="None"),
	EMA     UMETA(DisplayName="EMA"),
	OneEuro UMETA(DisplayName="One Euro"),
};

UENUM(BlueprintType)
enum class ELeapTowelHandSelection : uint8
{
	Any   UMETA(DisplayName="Any"),
	Left  UMETA(DisplayName="Left"),
	Right UMETA(DisplayName="Right"),
};

UENUM(BlueprintType)
enum class ELeapTowelAxis : uint8
{
	X         UMETA(DisplayName="X"),
	Y         UMETA(DisplayName="Y"),
	Z         UMETA(DisplayName="Z"),
	NegativeX UMETA(DisplayName="-X"),
	NegativeY UMETA(DisplayName="-Y"),
	NegativeZ UMETA(DisplayName="-Z"),
};

// Leap入力、タオル表示、拭き取り判定をまとめるActor。
UCLASS(Blueprintable)
class TOMATO_API ATomatinaTowelSystem : public AActor
{
	GENERATED_BODY()

public:
	ATomatinaTowelSystem();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Details調整だけでLeap入力を使えるようにする。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|C++ Input")
	ULeapComponent* LeapComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	bool bHandDetected = false;

	// 既存BP参照のため名前は維持。中身はスムージング/Clamp後の座標。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	FVector2D HandScreenPosition = FVector2D(0.5f, 0.5f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	FVector2D RawHandScreenPosition = FVector2D(0.5f, 0.5f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	FVector2D SmoothedHandScreenPosition = FVector2D(0.5f, 0.5f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	FVector2D ClampedHandScreenPosition = FVector2D(0.5f, 0.5f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	float HandSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	float ProcessedHandSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	bool bHasValidInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	bool bUsingGraceInput = false;

	// BP互換入口。bReadLeapInputInCpp=trueならTick冒頭のC++入力で上書きする。
	UFUNCTION(BlueprintCallable, Category="Towel")
	void UpdateHandData(bool bDetected, FVector2D ScreenPosition, float Speed);

	// カウントダウン中に拾った入力やWidget初期値を捨て、開始直後は中央に戻す。
	void ForceTowelToCenterAfterCountdown(float HoldSeconds = 1.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|C++ Input")
	bool bReadLeapInputInCpp = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|C++ Input")
	ELeapTowelHandSelection LeapHandSelection = ELeapTowelHandSelection::Any;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|C++ Input")
	bool bUseStabilizedPalmPosition = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|C++ Input")
	bool bApplyLeapDeviceOrigin = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|C++ Input")
	bool bUseUltraleapPluginFallback = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|C++ Input")
	FString LeapDeviceSerial;

	// センサーの置き方で軸が変わるのでDetailsで調整する。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Mapping")
	ELeapTowelAxis LeapHorizontalAxis = ELeapTowelAxis::Y;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Mapping")
	ELeapTowelAxis LeapVerticalAxis = ELeapTowelAxis::Z;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Mapping")
	FVector2D LeapInputCenter = FVector2D(0.0f, 25.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Mapping", meta=(ClampMin="1.0"))
	FVector2D LeapInputHalfRange = FVector2D(35.0f, 25.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Mapping", meta=(ClampMin="0.0", ClampMax="1000.0"))
	float LeapInputSpeedScale = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Gating", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MinConfidence = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Gating", meta=(ClampMin="0.0", ClampMax="2.0"))
	float MinVisibleTime = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Gating", meta=(ClampMin="0.0"))
	float HighSpeedThreshold = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Gating", meta=(ClampMin="0.1", ClampMax="1.0"))
	float SpeedConfidenceRelaxFactor = 0.7f;

	// 手を近づけすぎた時の警告。実機を見ながら軸としきい値を合わせる。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Too Close Warning")
	bool bEnableLeapTooCloseWarning = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Too Close Warning")
	ELeapTowelAxis LeapTooCloseAxis = ELeapTowelAxis::Z;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Too Close Warning")
	float LeapTooCloseThreshold = -5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Too Close Warning")
	float LeapTooCloseClearThreshold = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|TooClose")
	float CloseEnterThresholdMm = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|TooClose")
	float CloseExitThresholdMm = 130.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|TooClose", meta=(ClampMin="0.0"))
	float CloseEnterDelay = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|TooClose", meta=(ClampMin="0.0"))
	float CloseExitDelay = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|TooClose", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HeightEMAAlpha = 0.3f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	float LastLeapDistanceValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	float SmoothedPalmHeight = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	bool bLeapTooCloseToDevice = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Stability")
	bool bDebugLeapInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	FString LastLeapInputStatus = TEXT("Not started");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	bool bLeapComponentHasDevice = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	int32 LastLeapFrameHandCount = 0;

	// FrameId/TimeStampが止まるならLeap側の更新が止まっている。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	int32 LastLeapFrameId = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	int64 LastLeapFrameTimeStamp = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	bool bLastLeapLeftVisible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	bool bLastLeapRightVisible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	FVector LastSelectedLeapPalmRawPosition = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	FVector LastSelectedLeapPalmStabilizedPosition = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Debug")
	FVector LastSelectedLeapPalmPosition = FVector::ZeroVector;

	// 一瞬のロストだけ拾う。長く外れたら拭き取り停止。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Stability", meta=(ClampMin="0.0", ClampMax="0.5"))
	float InputGraceTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Grace", meta=(ClampMin="0.0", ClampMax="1.0"))
	float SpeedDecayStartRatio = 0.6f;

	// スムージングは拭き取り中心用。速度判定はRaw由来を使う。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Smoothing")
	EHandSmoothingMode SmoothingMode = EHandSmoothingMode::EMA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Smoothing", meta=(ClampMin="0.0", ClampMax="1.0"))
	float EMAAlpha = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Smoothing", meta=(ClampMin="0.001", ClampMax="30.0"))
	float OneEuroMinCutoff = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Smoothing", meta=(ClampMin="0.0", ClampMax="10.0"))
	float OneEuroBeta = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Smoothing", meta=(ClampMin="0.001", ClampMax="30.0"))
	float OneEuroDCutoff = 1.0f;

	// 画面端で中心が張り付いても端まで拭けるようにする。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wipe|Edge", meta=(ClampMin="0.0", ClampMax="0.5"))
	float EdgeThreshold = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wipe|Edge", meta=(ClampMin="0.0", ClampMax="0.5"))
	float EdgeRadiusBoost = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wipe|Edge", meta=(ClampMin="1.0", ClampMax="3.0"))
	float MaxEdgeRadiusMultiplier = 1.35f;

	UPROPERTY(EditAnywhere, Category="Towel")
	float MaxDurability = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category="Towel")
	float CurrentDurability = 100.0f;

	UPROPERTY(EditAnywhere, Category="Towel")
	float DurabilityDrainRate = 10.0f;

	UPROPERTY(EditAnywhere, Category="Towel")
	float SwapDuration = 3.0f;

	UPROPERTY(BlueprintReadOnly, Category="Towel")
	bool bIsSwapping = false;

	UPROPERTY(BlueprintReadOnly, Category="Towel")
	float SwapTimer = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Towel")
	bool bTowelVisible = false;

	UPROPERTY(EditAnywhere, Category="Wipe")
	float WipeRadius = 0.1f;

	UPROPERTY(EditAnywhere, Category="Wipe")
	float WipeEfficiency = 1.0f;

	UPROPERTY(EditAnywhere, Category="Wipe")
	float MinSpeedToWipe = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LeapMotion|Wiping", meta=(ClampMin="0.1", ClampMax="1.0"))
	float WipeContinueSpeedFactor = 0.5f;

	UPROPERTY(EditAnywhere, Category="Wipe")
	float SpeedMultiplier = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wiping|LineSegment", meta=(ClampMin="0.1", ClampMax="2.0"))
	float SampleSpacingFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wiping|LineSegment", meta=(ClampMin="1", ClampMax="32"))
	int32 MaxWipeSamples = 8;

	// 自動検索が外す場合はレベル上のDirtManagerを直接指定する。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wipe|References")
	ATomatoDirtManager* DirtManagerOverride = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Wipe|Debug")
	bool bWipeAttemptedThisFrame = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Wipe|Debug")
	bool bWipeHadDirtManagerThisFrame = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Wipe|Debug")
	FVector2D LastWipePosition = FVector2D(0.5f, 0.5f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Wipe|Debug")
	float LastAdjustedWipeRadius = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Wipe|Debug")
	float LastWipeAmount = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Wipe|Debug")
	float LastWipeSegmentLength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Wipe|Debug")
	int32 LastWipeSampleCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Wipe|Debug")
	float LastWipeAmountPerSample = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Wipe|Debug")
	bool bLastGraceJustExited = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wipe|Audio")
	USoundBase* WipeLoopSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wipe|Audio", meta=(ClampMin="0.0", ClampMax="4.0"))
	float WipeLoopVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wipe|Audio", meta=(ClampMin="0.1", ClampMax="4.0"))
	float WipeLoopPitch = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Towel|Audio")
	FTomatinaSoundCue TowelBreakSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Towel|Audio")
	FTomatinaSoundCue TowelReadySound;

	UPROPERTY(BlueprintReadOnly, Category="Towel")
	bool bTowelInZoomView = false;

	UPROPERTY(EditAnywhere, Category="Towel")
	int32 TowelPenalty = -20;

	// TakePhoto直前にGameModeから呼ばれる。
	UFUNCTION(BlueprintCallable, Category="Towel")
	bool CheckTowelInView(FVector2D TowelNormPos);

private:
	ATomatoDirtManager* GetDirtManager();
	ATomatinaPlayerPawn* GetPlayerPawn();

	UPROPERTY()
	ATomatoDirtManager* CachedDirtManager;

	UPROPERTY()
	ATomatinaPlayerPawn* CachedPlayerPawn;

	UPROPERTY()
	UAudioComponent* WipeAudioComp = nullptr;

	bool bWasWiping = false;
	bool bTowelShownOnHUD = false;

	bool bHasEverValidInput = false;
	bool bHasSmoothedHandPosition = false;
	bool bHasLastCppLeapScreenPosition = false;
	bool bHasLoggedBlueprintInputState = false;
	bool bLastLoggedBlueprintInputDetected = false;
	bool bLastCppLeapHadSelectedHand = false;
	bool bHasSmoothedPalmHeight = false;
	bool bLastLeapInputGated = false;
	bool bHasPrevWipePosition = false;
	bool bWasUsingGraceInput = false;
	bool bWarnedMissingDirtManager = false;
	bool bWarnedMissingPlayerPawn = false;
	bool bPendingStartupCenterCalibration = false;
	bool bHasStartupScreenOffset = false;
	float ForceCenterRemainingTime = 0.0f;
	float LastValidInputTime = -1000.0f;
	float LastValidHandSpeed = 0.0f;
	float CloseEnterTimer = 0.0f;
	float CloseExitTimer = 0.0f;
	float LastLeapConfidence = 0.0f;
	float LastLeapVisibleTime = 0.0f;
	float LastRawLeapHandSpeed = 0.0f;
	int32 LastLeapHandId = 0;
	FVector2D PrevClampedHandScreenPosition = FVector2D(0.5f, 0.5f);
	FVector2D LastValidRawHandScreenPosition = FVector2D(0.5f, 0.5f);
	FVector2D LastValidSmoothedHandScreenPosition = FVector2D(0.5f, 0.5f);
	FVector2D LastValidClampedHandScreenPosition = FVector2D(0.5f, 0.5f);
	FVector2D LastCppLeapScreenPosition = FVector2D(0.5f, 0.5f);
	FVector2D StartupScreenOffset = FVector2D::ZeroVector;

	struct FOneEuroAxisState
	{
		bool bInitialized = false;
		float PreviousRaw = 0.0f;
		float PreviousFiltered = 0.0f;
		float PreviousDerivative = 0.0f;
	};

	FOneEuroAxisState OneEuroX;
	FOneEuroAxisState OneEuroY;

	FVector2D ClampNormalizedHandPosition(FVector2D Position) const;
	void PollLeapInputFromCpp(float DeltaTime);
	void SetLeapInputStatus(const FString& NewStatus);
	bool TryGetSelectedLeapHand(const struct FLeapFrameData& Frame, struct FLeapHandData& OutHand) const;
	FVector2D ConvertLeapPositionToScreen(FVector LeapPosition) const;
	float ReadLeapAxis(FVector LeapPosition, ELeapTowelAxis Axis) const;
	void UpdateLeapTooCloseState(FVector LeapPosition, bool bHasSelectedHand, float DeltaTime);
	void ResetHandInputStateToCenter();
	float GetDurabilityPercent() const;
	void UpdateTowelHUDStatus(ATomatinaHUD* HUD) const;
	void HideTowelVisual(ATomatinaHUD* HUD);
	bool TickTowelSwap(float DeltaTime, ATomatinaHUD* HUD);
	FVector2D ApplyHandSmoothing(FVector2D RawPosition, float DeltaTime);
	float ApplyOneEuroAxis(float Value, float DeltaTime, FOneEuroAxisState& AxisState) const;
	float CalculateOneEuroAlpha(float Cutoff, float DeltaTime) const;
	float CalculateEdgeAdjustedWipeRadius(FVector2D ClampedPosition) const;
	void ResetHandInputFilter();
	void LogLeapInputFrame(bool bIsWiping) const;
	void UpdateWipeSound(bool bIsWiping);
};
