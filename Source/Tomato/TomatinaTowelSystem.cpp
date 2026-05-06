// Fill out your copyright notice in the Description page of Project Settings.

#include "TomatinaTowelSystem.h"

#include "Components/AudioComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "IUltraleapTrackingPlugin.h"
#include "LeapComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"

#include "TomatoDirtManager.h"
#include "TomatinaPlayerPawn.h"
#include "TomatinaHUD.h"
#include "TomatinaFunctionLibrary.h"

ATomatinaTowelSystem::ATomatinaTowelSystem()
	: CachedDirtManager(nullptr)
	, CachedPlayerPawn(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
	LeapComponent = CreateDefaultSubobject<ULeapComponent>(TEXT("LeapComponent"));
}

void ATomatinaTowelSystem::BeginPlay()
{
	Super::BeginPlay();

	CurrentDurability = MaxDurability;
	ResetHandInputStateToCenter();

	SetLeapInputStatus(bReadLeapInputInCpp ? TEXT("Waiting for Leap frame") : TEXT("C++ Leap input disabled"));

	if (bDebugLeapInput)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[TowelDiag] BeginPlay LeapCppInput=%d Durability=%.1f MinSpeedToWipe=%.2f WipeRadius=%.3f InitialPos=(%.3f,%.3f)"),
			bReadLeapInputInCpp ? 1 : 0,
			CurrentDurability, MinSpeedToWipe, WipeRadius,
			HandScreenPosition.X, HandScreenPosition.Y);
	}
}

void ATomatinaTowelSystem::ResetHandInputStateToCenter()
{
	const FVector2D CenterPosition(0.5f, 0.5f);
	bHandDetected = false;
	bHasValidInput = false;
	bUsingGraceInput = false;
	RawHandScreenPosition = CenterPosition;
	SmoothedHandScreenPosition = CenterPosition;
	ClampedHandScreenPosition = CenterPosition;
	HandScreenPosition = CenterPosition;
	LastValidRawHandScreenPosition = CenterPosition;
	LastValidSmoothedHandScreenPosition = CenterPosition;
	LastValidClampedHandScreenPosition = CenterPosition;
	LastCppLeapScreenPosition = CenterPosition;
	PrevClampedHandScreenPosition = CenterPosition;
	LastWipePosition = CenterPosition;
	HandSpeed = 0.0f;
	ProcessedHandSpeed = 0.0f;
	LastValidInputTime = -1000.0f;
	LastValidHandSpeed = 0.0f;
	bHasEverValidInput = false;
	bHasSmoothedHandPosition = false;
	bHasLastCppLeapScreenPosition = false;
	bHasPrevWipePosition = false;
	bWasUsingGraceInput = false;
	bPendingStartupCenterCalibration = false;
	bHasStartupScreenOffset = false;
	StartupScreenOffset = FVector2D::ZeroVector;
	bLastGraceJustExited = false;
	bWipeAttemptedThisFrame = false;
	bWipeHadDirtManagerThisFrame = false;
	LastWipeAmount = 0.0f;
	LastAdjustedWipeRadius = 0.0f;
	LastWipeSegmentLength = 0.0f;
	LastWipeSampleCount = 0;
	LastWipeAmountPerSample = 0.0f;
	ResetHandInputFilter();
}

void ATomatinaTowelSystem::ForceTowelToCenterAfterCountdown(float HoldSeconds)
{
	ResetHandInputStateToCenter();
	ForceCenterRemainingTime = FMath::Max(0.0f, HoldSeconds);
	bPendingStartupCenterCalibration = true;
	bHasStartupScreenOffset = false;
	StartupScreenOffset = FVector2D::ZeroVector;
	bTowelVisible = true;
	bTowelInZoomView = false;
	UpdateWipeSound(false);

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	ATomatinaHUD* HUD = PC ? Cast<ATomatinaHUD>(PC->GetHUD()) : nullptr;
	if (HUD)
	{
		HUD->UpdateTowelPosition(HandScreenPosition);
		HUD->ShowTowel();
		bTowelShownOnHUD = true;
		UpdateTowelHUDStatus(HUD);
	}

	if (bDebugLeapInput)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[TowelDiag] Force center after countdown Pos=(%.3f,%.3f) Hold=%.2f"),
			HandScreenPosition.X, HandScreenPosition.Y, ForceCenterRemainingTime);
	}
}

void ATomatinaTowelSystem::UpdateHandData(bool bDetected, FVector2D ScreenPosition, float Speed)
{
	FVector2D AdjustedScreenPosition = ScreenPosition;
	if (bDetected)
	{
		const FVector2D ClampedInputPosition = ClampNormalizedHandPosition(ScreenPosition);
		if (bPendingStartupCenterCalibration)
		{
			StartupScreenOffset = FVector2D(0.5f, 0.5f) - ClampedInputPosition;
			bHasStartupScreenOffset = true;
			bPendingStartupCenterCalibration = false;
		}

		if (bHasStartupScreenOffset)
		{
			AdjustedScreenPosition = ScreenPosition + StartupScreenOffset;
		}
	}

	bHandDetected         = bDetected;
	RawHandScreenPosition = AdjustedScreenPosition;
	ClampedHandScreenPosition = ClampNormalizedHandPosition(AdjustedScreenPosition);
	HandScreenPosition = ClampedHandScreenPosition;
	HandSpeed             = FMath::Max(0.0f, Speed);

	if (bDebugLeapInput && (!bHasLoggedBlueprintInputState || bDetected != bLastLoggedBlueprintInputDetected))
	{
		bHasLoggedBlueprintInputState = true;
		bLastLoggedBlueprintInputDetected = bDetected;
		UE_LOG(LogTemp, Warning,
			TEXT("[TowelDiag] UpdateHandData Detected=%d Raw=(%.3f,%.3f) Speed=%.2f"),
			bDetected ? 1 : 0,
			AdjustedScreenPosition.X, AdjustedScreenPosition.Y,
			Speed);
	}
}

float ATomatinaTowelSystem::GetDurabilityPercent() const
{
	return MaxDurability > KINDA_SMALL_NUMBER ? CurrentDurability / MaxDurability : 0.0f;
}

void ATomatinaTowelSystem::UpdateTowelHUDStatus(ATomatinaHUD* HUD) const
{
	if (HUD)
	{
		HUD->UpdateTowelStatus(GetDurabilityPercent(), bIsSwapping);
	}
}

void ATomatinaTowelSystem::HideTowelVisual(ATomatinaHUD* HUD)
{
	if (HUD && bTowelShownOnHUD)
	{
		HUD->HideTowel();
		bTowelShownOnHUD = false;
	}
}

bool ATomatinaTowelSystem::TickTowelSwap(float DeltaTime, ATomatinaHUD* HUD)
{
	if (!bIsSwapping)
	{
		return false;
	}

	bTowelVisible = false;
	bTowelInZoomView = false;
	UpdateWipeSound(false);
	HideTowelVisual(HUD);

	if (!bHasValidInput)
	{
		ProcessedHandSpeed = 0.0f;
		ResetHandInputFilter();
	}

	SwapTimer -= DeltaTime;
	if (SwapTimer <= 0.f)
	{
		bIsSwapping       = false;
		CurrentDurability = MaxDurability;
		UE_LOG(LogTemp, Warning,
			TEXT("ATomatinaTowelSystem: タオル交換完了 Durability=%.1f"), CurrentDurability);

		UTomatinaFunctionLibrary::PlayTomatinaCue2D(this, TowelReadySound);
	}

	UpdateTowelHUDStatus(HUD);
	return true;
}

void ATomatinaTowelSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PollLeapInputFromCpp(DeltaTime);

	APlayerController* PC  = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	ATomatinaHUD*       HUD = PC ? Cast<ATomatinaHUD>(PC->GetHUD()) : nullptr;
	if (HUD)
	{
		HUD->UpdateLeapDistanceWarning(bEnableLeapTooCloseWarning && bLeapTooCloseToDevice);
	}

	if (ForceCenterRemainingTime > 0.0f)
	{
		const float CenterDeltaTime = FMath::Max(FApp::GetDeltaTime(), DeltaTime);
		ForceCenterRemainingTime = FMath::Max(0.0f, ForceCenterRemainingTime - CenterDeltaTime);
		const FVector2D CenterPosition(0.5f, 0.5f);
		bTowelVisible = true;
		bTowelInZoomView = false;
		UpdateWipeSound(false);
		if (HUD)
		{
			HUD->UpdateTowelPosition(CenterPosition);
			HUD->ShowTowel();
			bTowelShownOnHUD = true;
			UpdateTowelHUDStatus(HUD);
		}
		LogLeapInputFrame(false);
		return;
	}

	const float NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	bHasValidInput = false;
	bUsingGraceInput = false;
	ProcessedHandSpeed = 0.0f;
	bWipeAttemptedThisFrame = false;
	bWipeHadDirtManagerThisFrame = false;
	LastWipeAmount = 0.0f;
	LastAdjustedWipeRadius = 0.0f;
	LastWipeSegmentLength = 0.0f;
	LastWipeSampleCount = 0;
	LastWipeAmountPerSample = 0.0f;

	if (bHandDetected)
	{
		bHasValidInput = true;
		SmoothedHandScreenPosition = ApplyHandSmoothing(RawHandScreenPosition, DeltaTime);
		ClampedHandScreenPosition = ClampNormalizedHandPosition(SmoothedHandScreenPosition);
		HandScreenPosition = ClampedHandScreenPosition;

		// 速度は Clamp 後の座標差分ではなく BP 由来の生速度を使う。
		// 画面端で座標が 0/1 に張り付いても、拭き取り/SE の速度判定を不自然に落とさないため。
		ProcessedHandSpeed = HandSpeed;

		bHasEverValidInput = true;
		LastValidInputTime = NowSeconds;
		LastValidHandSpeed = ProcessedHandSpeed;
		LastValidRawHandScreenPosition = RawHandScreenPosition;
		LastValidSmoothedHandScreenPosition = SmoothedHandScreenPosition;
		LastValidClampedHandScreenPosition = ClampedHandScreenPosition;
	}
	else if (bHasEverValidInput && InputGraceTime > 0.0f)
	{
		const float LostTime = NowSeconds - LastValidInputTime;
		if (LostTime <= InputGraceTime)
		{
			bHasValidInput = true;
			bUsingGraceInput = true;

			// 短時間ロスト中は最後のスムージング済み座標を保持し、速度だけ線形に落とす。
			// これにより端で一瞬外れても拭き取り中心は途切れにくく、長く拭き続けることはない。
			const float DecayStartTime = InputGraceTime * FMath::Clamp(SpeedDecayStartRatio, 0.0f, 1.0f);
			SmoothedHandScreenPosition = LastValidSmoothedHandScreenPosition;
			ClampedHandScreenPosition = LastValidClampedHandScreenPosition;
			HandScreenPosition = ClampedHandScreenPosition;
			if (LostTime <= DecayStartTime)
			{
				ProcessedHandSpeed = LastValidHandSpeed;
			}
			else
			{
				const float DecayDuration = FMath::Max(InputGraceTime - DecayStartTime, KINDA_SMALL_NUMBER);
				const float DecayAlpha = 1.0f - FMath::Clamp((LostTime - DecayStartTime) / DecayDuration, 0.0f, 1.0f);
				ProcessedHandSpeed = LastValidHandSpeed * DecayAlpha;
			}
		}
	}

	bLastGraceJustExited = bWasUsingGraceInput && !bUsingGraceInput;

	// タオル交換は入力が無い間も進める。先に no-input return すると、
	// 手を離した瞬間に交換タイマーと「交換中」UIが止まってしまう。
	if (TickTowelSwap(DeltaTime, HUD))
	{
		if (bHasValidInput)
		{
			bHasPrevWipePosition = true;
			PrevClampedHandScreenPosition = ClampedHandScreenPosition;
		}
		else
		{
			bHasPrevWipePosition = false;
			PrevClampedHandScreenPosition = FVector2D(0.5f, 0.5f);
		}
		bWasUsingGraceInput = bUsingGraceInput;
		LogLeapInputFrame(false);
		return;
	}

	if (!bHasValidInput)
	{
		bTowelVisible    = false;
		bTowelInZoomView = false;
		ProcessedHandSpeed = 0.0f;
		ResetHandInputFilter();
		bHasPrevWipePosition = false;
		PrevClampedHandScreenPosition = FVector2D(0.5f, 0.5f);
		if (!bHasEverValidInput)
		{
			const FVector2D CenterPosition(0.5f, 0.5f);
			RawHandScreenPosition = CenterPosition;
			SmoothedHandScreenPosition = CenterPosition;
			ClampedHandScreenPosition = CenterPosition;
			HandScreenPosition = CenterPosition;
			LastValidRawHandScreenPosition = CenterPosition;
			LastValidSmoothedHandScreenPosition = CenterPosition;
			LastValidClampedHandScreenPosition = CenterPosition;
			if (HUD)
			{
				HUD->UpdateTowelPosition(CenterPosition);
				HUD->ShowTowel();
				bTowelShownOnHUD = true;
			}
		}

		UpdateWipeSound(false);

		if (bHasEverValidInput)
		{
			HideTowelVisual(HUD);
		}
		UpdateTowelHUDStatus(HUD);
		bWasUsingGraceInput = bUsingGraceInput;
		LogLeapInputFrame(false);
		return;
	}

	bTowelVisible = true;

	if (HUD)
	{
		// Leapタオルはスマホ用カーソルではなく、メイン画面DirtOverlay上のIMG_Towelを動かす。
		HUD->UpdateTowelPosition(ClampedHandScreenPosition);
		if (!bTowelShownOnHUD)
		{
			HUD->ShowTowel();
			bTowelShownOnHUD = true;
		}
	}

	const float ContinueSpeedFactor = FMath::Clamp(WipeContinueSpeedFactor, 0.1f, 1.0f);
	const float CurrentMinSpeed = bWasWiping
		? MinSpeedToWipe * ContinueSpeedFactor
		: MinSpeedToWipe;
	const bool bIsWiping = (ProcessedHandSpeed >= CurrentMinSpeed);
	UpdateWipeSound(bIsWiping);

	{
		static bool bDiagWasWiping = false;
		if (bDebugLeapInput && bIsWiping != bDiagWasWiping)
		{
			bDiagWasWiping = bIsWiping;
			UE_LOG(LogTemp, Warning,
				TEXT("[TowelDiag] WipingState=%s ProcessedSpeed=%.2f RawSpeed=%.2f Min=%.2f"),
				bIsWiping ? TEXT("ON") : TEXT("OFF"),
				ProcessedHandSpeed, HandSpeed, CurrentMinSpeed);
		}
	}

	if (bIsWiping)
	{
		CurrentDurability -= DurabilityDrainRate * DeltaTime;

		const float Amount = WipeEfficiency * ProcessedHandSpeed * SpeedMultiplier * DeltaTime;
		const FVector2D CurrentPos = ClampedHandScreenPosition;
		const bool bUseLineSegment = bHasPrevWipePosition && !bLastGraceJustExited;
		const float SegmentLength = bHasPrevWipePosition
			? (CurrentPos - PrevClampedHandScreenPosition).Size()
			: 0.0f;
		const float SampleSpacing = WipeRadius * SampleSpacingFactor;
		const int32 RawSampleCount = bUseLineSegment
			? FMath::CeilToInt(SegmentLength / FMath::Max(SampleSpacing, KINDA_SMALL_NUMBER))
			: 1;
		const int32 SampleCount = FMath::Clamp(RawSampleCount, 1, FMath::Clamp(MaxWipeSamples, 1, 32));
		const float AmountPerSample = Amount / FMath::Max(SampleCount, 1);
		bWipeAttemptedThisFrame = true;
		LastWipePosition = CurrentPos;
		LastWipeAmount = Amount;
		LastWipeSegmentLength = SegmentLength;
		LastWipeSampleCount = SampleCount;
		LastWipeAmountPerSample = AmountPerSample;
		if (ATomatoDirtManager* DirtMgr = GetDirtManager())
		{
			bWipeHadDirtManagerThisFrame = true;
			for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
			{
				const float Alpha = (SampleCount == 1)
					? 1.0f
					: static_cast<float>(SampleIndex) / static_cast<float>(SampleCount - 1);
				const FVector2D SamplePos = bUseLineSegment
					? FMath::Lerp(PrevClampedHandScreenPosition, CurrentPos, Alpha)
					: CurrentPos;
				const float SampleRadius = CalculateEdgeAdjustedWipeRadius(SamplePos);
				LastAdjustedWipeRadius = SampleRadius;
				DirtMgr->WipeDirtAt(SamplePos, SampleRadius, AmountPerSample);
			}
		}
		else
		{
			LastAdjustedWipeRadius = CalculateEdgeAdjustedWipeRadius(CurrentPos);
			if (bDebugLeapInput && !bWarnedMissingDirtManager)
			{
				bWarnedMissingDirtManager = true;
				UE_LOG(LogTemp, Warning,
					TEXT("[TowelDiag] Wiping but DirtManager=NULL（レベルに ATomatoDirtManager 配置されてない可能性）"));
			}
		}

		if (CurrentDurability <= 0.f)
		{
			CurrentDurability = 0.f;
			bIsSwapping       = true;
			SwapTimer         = SwapDuration;
			UE_LOG(LogTemp, Warning,
				TEXT("ATomatinaTowelSystem: 耐久値切れ → タオル交換開始 (%.1f 秒)"), SwapDuration);

			UTomatinaFunctionLibrary::PlayTomatinaCue2D(this, TowelBreakSound);
		}
	}

	UpdateTowelHUDStatus(HUD);

	bTowelInZoomView = CheckTowelInView(ClampedHandScreenPosition);
	bHasPrevWipePosition = true;
	PrevClampedHandScreenPosition = ClampedHandScreenPosition;
	bWasUsingGraceInput = bUsingGraceInput;
	LogLeapInputFrame(bIsWiping);
}

FVector2D ATomatinaTowelSystem::ClampNormalizedHandPosition(FVector2D Position) const
{
	return FVector2D(
		FMath::Clamp(Position.X, 0.0f, 1.0f),
		FMath::Clamp(Position.Y, 0.0f, 1.0f));
}

void ATomatinaTowelSystem::PollLeapInputFromCpp(float DeltaTime)
{
	if (!bReadLeapInputInCpp)
	{
		SetLeapInputStatus(TEXT("C++ Leap input disabled"));
		bLastLeapInputGated = false;
		LastRawLeapHandSpeed = 0.0f;
		UpdateLeapTooCloseState(FVector::ZeroVector, false, DeltaTime);
		return;
	}

	if (!LeapComponent)
	{
		SetLeapInputStatus(TEXT("LeapComponent is null"));
		UpdateHandData(false, RawHandScreenPosition, 0.0f);
		bLastLeapInputGated = false;
		LastRawLeapHandSpeed = 0.0f;
		UpdateLeapTooCloseState(FVector::ZeroVector, false, DeltaTime);
		bHasLastCppLeapScreenPosition = false;
		bLastCppLeapHadSelectedHand = false;
		return;
	}

	FLeapFrameData Frame;
	LeapComponent->GetLatestFrameData(Frame, bApplyLeapDeviceOrigin);
	bLeapComponentHasDevice = LeapComponent->IsActiveDevicePluggedIn();
	LeapComponent->AreHandsVisible(bLastLeapLeftVisible, bLastLeapRightVisible);

	bool bUsedPluginFallback = false;
	if (Frame.Hands.Num() == 0 && bUseUltraleapPluginFallback && IUltraleapTrackingPlugin::IsAvailable())
	{
		FLeapFrameData FallbackFrame;
		IUltraleapTrackingPlugin::Get().GetLatestFrameData(FallbackFrame, LeapDeviceSerial);
		if (FallbackFrame.Hands.Num() > 0 || FallbackFrame.NumberOfHandsVisible > 0)
		{
			Frame = FallbackFrame;
			bUsedPluginFallback = true;
		}
	}

	LastLeapFrameHandCount = FMath::Max(Frame.Hands.Num(), Frame.NumberOfHandsVisible);
	LastLeapFrameId = Frame.FrameId;
	LastLeapFrameTimeStamp = Frame.TimeStamp;
	bLastLeapLeftVisible = Frame.LeftHandVisible;
	bLastLeapRightVisible = Frame.RightHandVisible;

	FLeapHandData SelectedHand;
	if (!TryGetSelectedLeapHand(Frame, SelectedHand))
	{
		if (Frame.Hands.Num() > 0)
		{
			SetLeapInputStatus(TEXT("Leap frame received, but selected hand was not found"));
		}
		else if (!bLeapComponentHasDevice && !bUsedPluginFallback)
		{
			SetLeapInputStatus(TEXT("Leap device is not active on LeapComponent"));
		}
		else
		{
			SetLeapInputStatus(TEXT("Leap frame has no hands"));
		}
		UpdateHandData(false, RawHandScreenPosition, 0.0f);
		bLastLeapInputGated = false;
		LastLeapConfidence = 0.0f;
		LastLeapVisibleTime = 0.0f;
		LastLeapHandId = 0;
		LastRawLeapHandSpeed = 0.0f;
		UpdateLeapTooCloseState(FVector::ZeroVector, false, DeltaTime);
		bHasLastCppLeapScreenPosition = false;
		bLastCppLeapHadSelectedHand = false;
		return;
	}

	LastSelectedLeapPalmRawPosition = SelectedHand.Palm.Position;
	LastSelectedLeapPalmStabilizedPosition = SelectedHand.Palm.StabilizedPosition;
	LastLeapConfidence = SelectedHand.Confidence;
	LastLeapVisibleTime = SelectedHand.VisibleTime;
	LastLeapHandId = SelectedHand.Id;

	const bool bHasStabilizedPosition = !SelectedHand.Palm.StabilizedPosition.IsNearlyZero();
	const FVector PalmPosition = (bUseStabilizedPalmPosition && bHasStabilizedPosition)
		? SelectedHand.Palm.StabilizedPosition
		: SelectedHand.Palm.Position;
	LastSelectedLeapPalmPosition = PalmPosition;
	const FVector2D ScreenPosition = ConvertLeapPositionToScreen(PalmPosition);

	float Speed = 0.0f;
	const float SafeDeltaTime = FMath::Max(DeltaTime, KINDA_SMALL_NUMBER);
	if (bHasLastCppLeapScreenPosition)
	{
		// C++入力ではBP由来の速度がない。端で鈍らないようRaw座標差分を使う。
		Speed = FVector2D::Distance(ScreenPosition, LastCppLeapScreenPosition) / SafeDeltaTime * LeapInputSpeedScale;
	}
	LastRawLeapHandSpeed = Speed;

	const float EffectiveMinConfidence = Speed > HighSpeedThreshold
		? MinConfidence * SpeedConfidenceRelaxFactor
		: MinConfidence;
	const bool bGatedByConfidence = SelectedHand.Confidence < EffectiveMinConfidence;
	const bool bGatedByVisibleTime = SelectedHand.VisibleTime < MinVisibleTime;
	if (bGatedByConfidence || bGatedByVisibleTime)
	{
		UpdateHandData(false, RawHandScreenPosition, 0.0f);
		bLastLeapInputGated = true;
		UpdateLeapTooCloseState(FVector::ZeroVector, false, DeltaTime);
		bHasLastCppLeapScreenPosition = false;
		bLastCppLeapHadSelectedHand = false;
		SetLeapInputStatus(TEXT("Leap hand gated by confidence/visible time"));

		if (bDebugLeapInput)
		{
			UE_LOG(LogTemp, Verbose,
				TEXT("[TowelDiag] Leap gated HandId=%d Confidence=%.3f MinConfidence=%.3f VisibleTime=%.3f MinVisibleTime=%.3f Speed=%.2f"),
				SelectedHand.Id,
				SelectedHand.Confidence,
				EffectiveMinConfidence,
				SelectedHand.VisibleTime,
				MinVisibleTime,
				Speed);
		}
		return;
	}

	bLastLeapInputGated = false;
	UpdateLeapTooCloseState(PalmPosition, true, DeltaTime);
	LastCppLeapScreenPosition = ScreenPosition;
	bHasLastCppLeapScreenPosition = true;
	UpdateHandData(true, ScreenPosition, Speed);
	SetLeapInputStatus(bUsedPluginFallback ? TEXT("Tracking hand via plugin fallback") : TEXT("Tracking hand via LeapComponent"));

	if (bDebugLeapInput && !bLastCppLeapHadSelectedHand)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[TowelDiag] CppLeap selected Hand=%d Palm=(%.2f,%.2f,%.2f) ScreenRaw=(%.3f,%.3f) Fallback=%d"),
			static_cast<int32>(SelectedHand.HandType.GetValue()),
			PalmPosition.X, PalmPosition.Y, PalmPosition.Z,
			ScreenPosition.X, ScreenPosition.Y,
			bUsedPluginFallback ? 1 : 0);
	}
	bLastCppLeapHadSelectedHand = true;
}

void ATomatinaTowelSystem::SetLeapInputStatus(const FString& NewStatus)
{
	if (LastLeapInputStatus == NewStatus)
	{
		return;
	}

	LastLeapInputStatus = NewStatus;
	if (bDebugLeapInput)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TowelDiag] %s"), *LastLeapInputStatus);
	}
}

bool ATomatinaTowelSystem::TryGetSelectedLeapHand(const FLeapFrameData& Frame, FLeapHandData& OutHand) const
{
	for (const FLeapHandData& Hand : Frame.Hands)
	{
		const EHandType HandType = Hand.HandType.GetValue();
		const bool bMatchesSelection =
			LeapHandSelection == ELeapTowelHandSelection::Any ||
			(LeapHandSelection == ELeapTowelHandSelection::Left && HandType == LEAP_HAND_LEFT) ||
			(LeapHandSelection == ELeapTowelHandSelection::Right && HandType == LEAP_HAND_RIGHT);

		if (bMatchesSelection)
		{
			OutHand = Hand;
			return true;
		}
	}

	return false;
}

FVector2D ATomatinaTowelSystem::ConvertLeapPositionToScreen(FVector LeapPosition) const
{
	const float Horizontal = ReadLeapAxis(LeapPosition, LeapHorizontalAxis);
	const float Vertical = ReadLeapAxis(LeapPosition, LeapVerticalAxis);
	const FVector2D SafeHalfRange(
		FMath::Max(LeapInputHalfRange.X, 1.0f),
		FMath::Max(LeapInputHalfRange.Y, 1.0f));

	return FVector2D(
		0.5f + (Horizontal - LeapInputCenter.X) / (SafeHalfRange.X * 2.0f),
		0.5f - (Vertical - LeapInputCenter.Y) / (SafeHalfRange.Y * 2.0f));
}

float ATomatinaTowelSystem::ReadLeapAxis(FVector LeapPosition, ELeapTowelAxis Axis) const
{
	switch (Axis)
	{
	case ELeapTowelAxis::X:
		return LeapPosition.X;
	case ELeapTowelAxis::Y:
		return LeapPosition.Y;
	case ELeapTowelAxis::Z:
		return LeapPosition.Z;
	case ELeapTowelAxis::NegativeX:
		return -LeapPosition.X;
	case ELeapTowelAxis::NegativeY:
		return -LeapPosition.Y;
	case ELeapTowelAxis::NegativeZ:
		return -LeapPosition.Z;
	default:
		return LeapPosition.Y;
	}
}

void ATomatinaTowelSystem::UpdateLeapTooCloseState(FVector LeapPosition, bool bHasSelectedHand, float DeltaTime)
{
	if (!bEnableLeapTooCloseWarning || !bHasSelectedHand)
	{
		bLeapTooCloseToDevice = false;
		LastLeapDistanceValue = 0.0f;
		SmoothedPalmHeight = 0.0f;
		bHasSmoothedPalmHeight = false;
		CloseEnterTimer = 0.0f;
		CloseExitTimer = 0.0f;
		return;
	}

	LastLeapDistanceValue = ReadLeapAxis(LeapPosition, LeapTooCloseAxis);
	// Ultraleap plugin converts hand positions from mm to UE cm; thresholds are exposed as mm.
	const float RawPalmHeightMm = LastLeapDistanceValue * 10.0f;
	const float Alpha = FMath::Clamp(HeightEMAAlpha, 0.0f, 1.0f);
	if (!bHasSmoothedPalmHeight)
	{
		bHasSmoothedPalmHeight = true;
		SmoothedPalmHeight = RawPalmHeightMm;
	}
	else
	{
		SmoothedPalmHeight = FMath::Lerp(SmoothedPalmHeight, RawPalmHeightMm, Alpha);
	}

	const float EnterThreshold = CloseEnterThresholdMm;
	const float ExitThreshold = FMath::Max(CloseExitThresholdMm, EnterThreshold);
	if (!bLeapTooCloseToDevice && SmoothedPalmHeight < EnterThreshold)
	{
		CloseEnterTimer += DeltaTime;
		CloseExitTimer = 0.0f;
		if (CloseEnterTimer >= CloseEnterDelay)
		{
			bLeapTooCloseToDevice = true;
			CloseEnterTimer = 0.0f;
		}
	}
	else if (bLeapTooCloseToDevice && SmoothedPalmHeight > ExitThreshold)
	{
		CloseExitTimer += DeltaTime;
		CloseEnterTimer = 0.0f;
		if (CloseExitTimer >= CloseExitDelay)
		{
			bLeapTooCloseToDevice = false;
			CloseExitTimer = 0.0f;
		}
	}
	else
	{
		CloseEnterTimer = 0.0f;
		CloseExitTimer = 0.0f;
	}
}

FVector2D ATomatinaTowelSystem::ApplyHandSmoothing(FVector2D RawPosition, float DeltaTime)
{
	if (!bHasSmoothedHandPosition)
	{
		bHasSmoothedHandPosition = true;
		OneEuroX = FOneEuroAxisState();
		OneEuroY = FOneEuroAxisState();
		return RawPosition;
	}

	switch (SmoothingMode)
	{
	case EHandSmoothingMode::EMA:
	{
		const float Alpha = FMath::Clamp(EMAAlpha, 0.0f, 1.0f);
		return FMath::Lerp(SmoothedHandScreenPosition, RawPosition, Alpha);
	}
	case EHandSmoothingMode::OneEuro:
		return FVector2D(
			ApplyOneEuroAxis(RawPosition.X, DeltaTime, OneEuroX),
			ApplyOneEuroAxis(RawPosition.Y, DeltaTime, OneEuroY));
	case EHandSmoothingMode::None:
	default:
		return RawPosition;
	}
}

float ATomatinaTowelSystem::ApplyOneEuroAxis(float Value, float DeltaTime, FOneEuroAxisState& AxisState) const
{
	const float SafeDeltaTime = FMath::Max(DeltaTime, KINDA_SMALL_NUMBER);
	if (!AxisState.bInitialized)
	{
		AxisState.bInitialized = true;
		AxisState.PreviousRaw = Value;
		AxisState.PreviousFiltered = Value;
		AxisState.PreviousDerivative = 0.0f;
		return Value;
	}

	const float RawDerivative = (Value - AxisState.PreviousRaw) / SafeDeltaTime;
	const float DerivativeAlpha = CalculateOneEuroAlpha(OneEuroDCutoff, SafeDeltaTime);
	const float FilteredDerivative = FMath::Lerp(AxisState.PreviousDerivative, RawDerivative, DerivativeAlpha);
	const float DynamicCutoff = FMath::Max(0.001f, OneEuroMinCutoff + OneEuroBeta * FMath::Abs(FilteredDerivative));
	const float PositionAlpha = CalculateOneEuroAlpha(DynamicCutoff, SafeDeltaTime);
	const float FilteredValue = FMath::Lerp(AxisState.PreviousFiltered, Value, PositionAlpha);

	AxisState.PreviousRaw = Value;
	AxisState.PreviousFiltered = FilteredValue;
	AxisState.PreviousDerivative = FilteredDerivative;
	return FilteredValue;
}

float ATomatinaTowelSystem::CalculateOneEuroAlpha(float Cutoff, float DeltaTime) const
{
	const float SafeCutoff = FMath::Max(Cutoff, 0.001f);
	const float Tau = 1.0f / (2.0f * PI * SafeCutoff);
	return 1.0f / (1.0f + Tau / FMath::Max(DeltaTime, KINDA_SMALL_NUMBER));
}

float ATomatinaTowelSystem::CalculateEdgeAdjustedWipeRadius(FVector2D ClampedPosition) const
{
	const float BaseRadius = FMath::Max(0.0f, WipeRadius);
	if (BaseRadius <= 0.0f || EdgeThreshold <= 0.0f || EdgeRadiusBoost <= 0.0f)
	{
		return BaseRadius;
	}

	const float DistanceToEdge = FMath::Min(
		FMath::Min(ClampedPosition.X, 1.0f - ClampedPosition.X),
		FMath::Min(ClampedPosition.Y, 1.0f - ClampedPosition.Y));
	const float EdgeAlpha = 1.0f - FMath::Clamp(DistanceToEdge / EdgeThreshold, 0.0f, 1.0f);
	const float BoostedRadius = BaseRadius + EdgeRadiusBoost * EdgeAlpha;
	const float MaxRadius = BaseRadius * FMath::Max(1.0f, MaxEdgeRadiusMultiplier);
	return FMath::Clamp(BoostedRadius, BaseRadius, MaxRadius);
}

void ATomatinaTowelSystem::ResetHandInputFilter()
{
	bHasSmoothedHandPosition = false;
	OneEuroX = FOneEuroAxisState();
	OneEuroY = FOneEuroAxisState();
}

bool ATomatinaTowelSystem::CheckTowelInView(FVector2D TowelNormPos)
{
	ATomatinaPlayerPawn* Pawn = GetPlayerPawn();
	if (!Pawn || !Pawn->bIsZooming) { return false; }
	if (!Pawn->SceneCapture_Zoom) { return false; }

	APlayerController* PC = Pawn->GetController<APlayerController>();
	if (!PC) { return false; }

	const FVector2D ZoomScreenCenter = UTomatinaFunctionLibrary::ProjectZoomToMainScreen(
		PC, Pawn->SceneCapture_Zoom, Pawn->MainWidth, Pawn->MainHeight);

	const FVector2D NormZoomCenter(
		ZoomScreenCenter.X / FMath::Max(Pawn->MainWidth,  1.f),
		ZoomScreenCenter.Y / FMath::Max(Pawn->MainHeight, 1.f));

	const float ZoomRatio    = Pawn->DefaultFOV
	                           / FMath::Max<float>(Pawn->SceneCapture_Zoom->FOVAngle, 1.0f);
	const float ViewHalfSize = 0.5f / ZoomRatio;

	const bool bInView =
		FMath::Abs(TowelNormPos.X - NormZoomCenter.X) < ViewHalfSize &&
		FMath::Abs(TowelNormPos.Y - NormZoomCenter.Y) < ViewHalfSize;

	return bInView;
}

ATomatoDirtManager* ATomatinaTowelSystem::GetDirtManager()
{
	if (IsValid(DirtManagerOverride))
	{
		CachedDirtManager = DirtManagerOverride;
		return CachedDirtManager;
	}

	if (!IsValid(CachedDirtManager))
	{
		AActor* Found = UGameplayStatics::GetActorOfClass(
			GetWorld(), ATomatoDirtManager::StaticClass());
		CachedDirtManager = Cast<ATomatoDirtManager>(Found);

		if (!CachedDirtManager && bDebugLeapInput && !bWarnedMissingDirtManager)
		{
			bWarnedMissingDirtManager = true;
			UE_LOG(LogTemp, Warning,
				TEXT("ATomatinaTowelSystem::GetDirtManager: "
				     "ATomatoDirtManager がレベル上に見つかりません"));
		}
	}
	return CachedDirtManager;
}

ATomatinaPlayerPawn* ATomatinaTowelSystem::GetPlayerPawn()
{
	if (!CachedPlayerPawn)
	{
		if (APlayerController* PC =
			GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
		{
			CachedPlayerPawn = Cast<ATomatinaPlayerPawn>(PC->GetPawn());
		}

		if (!CachedPlayerPawn && bDebugLeapInput && !bWarnedMissingPlayerPawn)
		{
			bWarnedMissingPlayerPawn = true;
			UE_LOG(LogTemp, Warning,
				TEXT("ATomatinaTowelSystem::GetPlayerPawn: "
				     "ATomatinaPlayerPawn の取得に失敗"));
		}
	}
	return CachedPlayerPawn;
}

void ATomatinaTowelSystem::LogLeapInputFrame(bool bIsWiping) const
{
	if (!bDebugLeapInput)
	{
		return;
	}

	UE_LOG(LogTemp, Verbose,
		TEXT("[TowelDiag] RawPalmPos=(%.2f,%.2f,%.2f) SmoothedPalmHeight=%.2f Confidence=%.3f VisibleTime=%.3f HandId=%d bGated=%d bUsingGraceInput=%d RawHandSpeed=%.2f ProcessedHandSpeed=%.2f bIsWiping=%d bIsTooClose=%d SegmentLength=%.4f SampleCount=%d AmountPerSample=%.4f bGraceJustExited=%d"),
		LastSelectedLeapPalmRawPosition.X,
		LastSelectedLeapPalmRawPosition.Y,
		LastSelectedLeapPalmRawPosition.Z,
		SmoothedPalmHeight,
		LastLeapConfidence,
		LastLeapVisibleTime,
		LastLeapHandId,
		bLastLeapInputGated ? 1 : 0,
		bUsingGraceInput ? 1 : 0,
		LastRawLeapHandSpeed,
		ProcessedHandSpeed,
		bIsWiping ? 1 : 0,
		bLeapTooCloseToDevice ? 1 : 0,
		LastWipeSegmentLength,
		LastWipeSampleCount,
		LastWipeAmountPerSample,
		bLastGraceJustExited ? 1 : 0);
}

void ATomatinaTowelSystem::UpdateWipeSound(bool bIsWiping)
{
	if (bIsWiping == bWasWiping)
	{
		return;
	}
	bWasWiping = bIsWiping;

	if (bIsWiping)
	{
		if (!WipeLoopSound) { return; }
		if (WipeAudioComp && WipeAudioComp->IsPlaying())
		{
			return;
		}
		WipeAudioComp = UGameplayStatics::SpawnSound2D(
			this, WipeLoopSound, WipeLoopVolume, WipeLoopPitch);
	}
	else
	{
		if (WipeAudioComp && WipeAudioComp->IsPlaying())
		{
			WipeAudioComp->Stop();
		}
		WipeAudioComp = nullptr;
	}
}
