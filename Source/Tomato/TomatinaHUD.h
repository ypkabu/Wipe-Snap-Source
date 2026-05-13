// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TomatoDirtManager.h"
#include "TomatinaFunctionLibrary.h"
#include "TomatinaHUD.generated.h"

class UUserWidget;
class UTexture2D;
class UMaterialInterface;
class UTextBlock;
class UWidget;
class SWindow;

// Widget生成と表示切り替えをまとめるHUD。
UCLASS()
class TOMATO_API ATomatinaHUD : public AHUD
{
	GENERATED_BODY()

public:
	ATomatinaHUD();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Widgets")
	TSubclassOf<UUserWidget> ViewFinderWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Widgets")
	TSubclassOf<UUserWidget> CursorWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Widgets")
	TSubclassOf<UUserWidget> DirtOverlayWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Widgets")
	TSubclassOf<UUserWidget> PhotoResultWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Widgets")
	TSubclassOf<UUserWidget> MissionDisplayWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Widgets")
	TSubclassOf<UUserWidget> MissionResultWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Widgets")
	TSubclassOf<UUserWidget> FinalResultWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Widgets")
	TSubclassOf<UUserWidget> ShutterFlashWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Widgets")
	TSubclassOf<UUserWidget> CountdownWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Widgets")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Widgets")
	TSubclassOf<UUserWidget> TestPipWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Debug")
	bool bDebugHUDLog = false;

	// スマホ側SWindowに載せるWidget。IMG_ZoomView / PhoneSplatContainer / IMG_PhoneCursorを想定。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Widgets")
	TSubclassOf<UUserWidget> PhoneViewWidgetClass;

	UPROPERTY(EditAnywhere, Category="HUD|Material")
	UMaterialInterface* ZoomDisplayMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category="HUD|Material")
	UMaterialInterface* PhotoDisplayMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category="HUD|Material")
	UTexture2D* DirtTexture = nullptr;

	// FDirtSplat::TextureIndexで参照。空ならDirtTextureへフォールバック。
	UPROPERTY(EditAnywhere, Category="HUD|Material")
	TArray<UTexture2D*> DirtTextures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Photo")
	float PhotoDisplayWidth = 1024.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Photo")
	float PhotoDisplayHeight = 768.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Dirt")
	float DirtInnerMargin = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Dirt", meta=(ClampMin="0.1", ClampMax="2.0"))
	float DirtSizeScalePhone = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Dirt", meta=(ClampMin="0.1", ClampMax="2.0"))
	float DirtSizeScalePhoto = 0.5f;

	// メイン画面だけ中央寄せで狭める。スマホ/写真側には使わない。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Dirt", meta=(ClampMin="0.1", ClampMax="1.0"))
	FVector2D MainDirtAreaRatio = FVector2D(0.75f, 0.675f);

	UPROPERTY(BlueprintReadOnly, Category="HUD|Screen")
	float MainWidth = 2560.f;

	UPROPERTY(BlueprintReadOnly, Category="HUD|Screen")
	float MainHeight = 1600.f;

	UPROPERTY(BlueprintReadOnly, Category="HUD|Screen")
	float PhoneWidth = 2024.f;

	UPROPERTY(BlueprintReadOnly, Category="HUD|Screen")
	float PhoneHeight = 1152.f;

	UPROPERTY(BlueprintReadOnly, Category="HUD|Screen")
	bool bTestMode = true;

	UPROPERTY(BlueprintReadOnly, Category="HUD|Screen")
	bool bUseSeparatePhoneWindow = true;

	// Windowsのディスプレイ配置と合わない時だけ手動指定する。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Screen")
	bool bOverridePhoneWindowPosition = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Screen", meta=(EditCondition="bOverridePhoneWindowPosition"))
	FVector2D PhoneWindowPositionOverride = FVector2D(2560.f, 0.f);

	UFUNCTION(BlueprintCallable, Category="HUD|Cursor")
	void UpdateCursorPosition(FVector2D Pos);

	UFUNCTION(BlueprintCallable, Category="HUD|Cursor")
	void ShowCursor();

	UFUNCTION(BlueprintCallable, Category="HUD|Cursor")
	void HideCursor();

	// WBP_DirtOverlayにIMG_Towelを置く。
	UFUNCTION(BlueprintCallable, Category="HUD|Towel")
	void UpdateTowelPosition(FVector2D Pos);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Towel")
	bool bApplyTowelVisualCenterOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Towel", meta=(EditCondition="bApplyTowelVisualCenterOffset"))
	FVector2D TowelVisualCenterOffset = FVector2D::ZeroVector;

	UFUNCTION(BlueprintCallable, Category="HUD|Towel")
	void ShowTowel();

	UFUNCTION(BlueprintCallable, Category="HUD|Towel")
	void HideTowel();

	UFUNCTION(BlueprintCallable, Category="HUD|Result")
	void ShowResult(int32 Score, const FString& Comment, const TArray<FDirtSplat>& Dirts);

	UFUNCTION(BlueprintCallable, Category="HUD|Result")
	void HideResult();

	UFUNCTION(BlueprintCallable, Category="HUD|Result")
	void ShowMissionResult(int32 Score, const FString& Comment);

	UFUNCTION(BlueprintCallable, Category="HUD|Result")
	void HideMissionResult();

	UFUNCTION(BlueprintCallable, Category="HUD|Result")
	void ShowFinalResult(int32 InTotalScore, int32 MissionCount,
		const FString& AverageStylishRank = TEXT("C"), float SyncRate01 = 0.f);

	UFUNCTION(BlueprintCallable, Category="HUD|Mission")
	void ShowMissionDisplay(const FText& MissionText, UTexture2D* TargetImage);

	UFUNCTION(BlueprintCallable, Category="HUD|Mission")
	void HideMissionDisplay();

	UFUNCTION(BlueprintCallable, Category="HUD|Mission")
	void UpdateTimer(float RemainingSeconds);

	UFUNCTION(BlueprintCallable, Category="HUD|Mission")
	void UpdateGameTimer(float RemainingSeconds, float TotalSeconds);

	UFUNCTION(BlueprintCallable, Category="HUD|Mission")
	void UpdateTotalScore(int32 TotalScore);

	UFUNCTION(BlueprintCallable, Category="HUD|Stylish")
	void UpdateStylishDisplay(const FString& RankText, float GaugePercent, int32 ComboCount, bool bDanger);

	UFUNCTION(BlueprintCallable, Category="HUD|Countdown")
	void ShowCountdown(int32 Seconds);

	UFUNCTION(BlueprintCallable, Category="HUD|Countdown")
	void HideCountdown();

	UFUNCTION(BlueprintCallable, Category="HUD|Loading")
	void ShowLoading();

	UFUNCTION(BlueprintCallable, Category="HUD|Loading")
	void HideLoading();

	UFUNCTION(BlueprintCallable, Category="HUD|Flash")
	void PlayShutterFlash();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Flash", meta=(ClampMin="0.01", ClampMax="2.0"))
	float ShutterFlashDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Flash")
	int32 ShutterFlashZOrder = 9999;

	UFUNCTION(BlueprintCallable, Category="HUD|Dirt")
	void UpdateDirtDisplay(const TArray<FDirtSplat>& Dirts);

	UFUNCTION(BlueprintCallable, Category="HUD|Towel")
	void UpdateTowelStatus(float DurabilityPercent, bool bSwapping);

	UFUNCTION(BlueprintCallable, Category="HUD|Leap")
	void UpdateLeapDistanceWarning(bool bVisible);

	UFUNCTION(BlueprintCallable, Category="HUD|Phone")
	void SetFramingPreviewResult(const FPhotoFramingPreviewResult& Result);

	UFUNCTION(BlueprintCallable, Category="HUD|Phone")
	void SetFramingPreviewVisible(bool bVisible);

protected:
	UPROPERTY() UUserWidget* ViewFinderWidget     = nullptr;
	UPROPERTY() UUserWidget* CursorWidget         = nullptr;
	UPROPERTY() UUserWidget* DirtOverlayWidget    = nullptr;
	UPROPERTY() UUserWidget* MissionDisplayWidget = nullptr;
	UPROPERTY() UUserWidget* TestPipWidget        = nullptr;
	UPROPERTY() UUserWidget* PhoneViewWidget      = nullptr;

	UPROPERTY() class UMaterialInstanceDynamic* PhoneZoomDMI = nullptr;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY() UUserWidget* PhotoResultWidget    = nullptr;
	UPROPERTY() UUserWidget* MissionResultWidget  = nullptr;
	UPROPERTY() UUserWidget* FinalResultWidget    = nullptr;
	UPROPERTY() UUserWidget* CountdownWidget      = nullptr;
	UPROPERTY() UUserWidget* ShutterFlashWidget   = nullptr;
	UPROPERTY() UUserWidget* LoadingWidget        = nullptr;
	UPROPERTY() UTextBlock* RuntimeLeapDistanceWarningText = nullptr;
	UPROPERTY() UTextBlock* RuntimeFramingPreviewText = nullptr;

private:
	// TimeDilation=0でもフラッシュを消すため実時間で見る。
	float FlashElapsed = 0.f;
	bool  bFlashActive = false;

	// OriginはCanvas内の左上。メイン/スマホ/写真で同じ正規化座標を使い回す。
	void AddDirtSplatsToCanvas(
		UUserWidget* OwnerWidget,
		class UCanvasPanel* Container,
		const TArray<FDirtSplat>& Dirts,
		float AreaWidth,
		float AreaHeight,
		float OriginX,
		float OriginY,
		float SizeScale = 1.0f);

	void ValidateMissionStylishWidgets();

	bool BindZoomMaterialToWidget(UUserWidget* Widget, FName PreferredImageName, const TCHAR* WidgetLabel);

	void LayoutPhoneZoomImage(UUserWidget* Widget, FName PreferredImageName, const TCHAR* WidgetLabel);

	class UImage* FindOrCreateZoomImage(UUserWidget* Widget, FName PreferredImageName, const TCHAR* WidgetLabel);

	bool ConfigureZoomImageContent(class UImage* ImageWidget, const TCHAR* WidgetLabel);

	bool ForceWidgetToFillParentCanvas(UWidget* Widget, const TCHAR* WidgetLabel);

	UTextBlock* FindOrCreateFramingPreviewText();

	FString BuildFramingPreviewText(const FPhotoFramingPreviewResult& Result) const;

	void CreatePhoneWindow();

	void DestroyPhoneWindow();

	TSharedPtr<SWindow> PhoneWindow;
};
