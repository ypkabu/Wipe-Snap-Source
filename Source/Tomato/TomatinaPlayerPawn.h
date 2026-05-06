#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "InputActionValue.h"
#include "TomatinaSoundCue.h"  // FTomatinaSoundCue
#include "TomatinaPlayerPawn.generated.h"

class UCameraComponent;
class USceneCaptureComponent2D;
class APlayerController;
class UInputMappingContext;
class UInputAction;
class ULeapComponent;
class ATomatinaHUD;

// 撮影操作とズーム用SceneCaptureを持つプレイヤーポーン。
UCLASS()
class TOMATO_API ATomatinaPlayerPawn : public ADefaultPawn
{
	GENERATED_BODY()

public:
	ATomatinaPlayerPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* PlayerCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
	USceneCaptureComponent2D* SceneCapture_Zoom;

	// 旧BPのLeapイベント参照を壊さないための互換用。タオル操作はTowelSystem側で読む。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LeapMotion|Deprecated")
	ULeapComponent* Leap = nullptr;

	// GameMode / HUD が BeginPlay で参照する画面サイズ。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Screen")
	float MainWidth = 2560.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Screen")
	float MainHeight = 1600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Screen")
	float PhoneWidth = 2024.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Screen")
	float PhoneHeight = 1152.f;

	// iPhoneなしのPiP確認用。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
	bool bTestMode = true;

	// 入力/ズーム/画面配置調査用。通常はOFF。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
	bool bDebugPlayerLog = false;

	// trueならスマホ側を独立SWindowへ出す。falseは旧スパンウィンドウ方式。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Screen")
	bool bUseSeparatePhoneWindow = true;

	UPROPERTY(BlueprintReadOnly, Category="Zoom")
	bool bIsZooming = false;

	UPROPERTY(BlueprintReadOnly, Category="Zoom")
	bool bZoomComplete = false;

	UPROPERTY(BlueprintReadOnly, Category="Zoom")
	bool bCursorCentered = false;

	UPROPERTY(BlueprintReadOnly, Category="Zoom")
	float ZoomAlpha = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom")
	float DefaultFOV = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom")
	float ZoomFOV = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom")
	float ZoomSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom")
	float MoveSpeed = 500.f;

	// ズーム中の視点移動感度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom")
	float ZoomLookSensitivity = 3.0f;

	// ズーム中のPitch最大角度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom")
	float ZoomPitchLimit = 89.f;

	// 0以下ならYaw制限なし。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom")
	float ZoomYawLimit = 0.f;

	// ズームカメラの衝突回避スイープ用。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom")
	float ZoomOffsetSweepRadius = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom")
	float ZoomOffsetSafetyMargin = 25.f;

	// 壁へ食い込みかけた時に手前の面を描画しない保険。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom")
	float ZoomNearClippingPlane = 20.f;

	// スマホ側RenderTargetを開始直後だけ温める。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom|Capture", meta=(ClampMin="0.0", ClampMax="2.0"))
	float InitialPhoneCaptureSeconds = 0.35f;

	// ズーム前後の短いキャプチャ猶予。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom|Capture", meta=(ClampMin="0.0", ClampMax="2.0"))
	float ZoomTransitionCaptureSeconds = 0.25f;

	// 空クリック時の仮想ヒット距離。空の鳥などにもズームできるようにする。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom")
	float SkyFallbackDistance = 3000.f;

	UPROPERTY(BlueprintReadOnly, Category="Zoom")
	FVector TargetOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom|Audio")
	FTomatinaSoundCue ZoomInSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zoom|Audio")
	FTomatinaSoundCue ZoomOutSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputMappingContext* DefaultMappingContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_RightMouse = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_LeftMouse = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_Look = nullptr;

protected:
	void OnRightMousePressed(const FInputActionValue& Value);
	void OnRightMouseReleased(const FInputActionValue& Value);
	void OnLeftMousePressed(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void CenterCursorOnMainScreen();

private:
	UPROPERTY()
	APlayerController* PC = nullptr;

	void EnsureDualScreenWindowLayout();

	bool bWindowLayoutVerified = false;
	float WindowLayoutRetryElapsed = 0.f;
	int32 WindowLayoutRetryCount = 0;

	// Tick で消費されるマウス入力（Triggered で蓄積）
	FVector2D CurrentLookInput = FVector2D::ZeroVector;

	float PhoneCaptureBurstRemaining = 0.f;

	void UpdateDualScreenLayoutRetry(float DeltaTime);
	void UpdateZoomInterpolation(float RealDelta);
	void CenterLegacySpanCursorWhenZoomReady();
	void EnableZoomLookWhenReady();
	void ApplyZoomLookInput(float RealDelta);
	void UpdateZoomHUDCursor(ATomatinaHUD* HUD);
	void ResetZoomViewWhenIdle();
	void ClampMouseCursorToMainScreen();
	void EndZoomInteraction();
	void ClampZoomTargetOffsetAgainstWorld();
	void ApplyZoomNearClipGuard();
	void RequestPhoneCaptureBurst(float Duration);
	void UpdatePhoneSceneCapture(float DeltaTime);
	void CapturePhoneSceneNow();
};
