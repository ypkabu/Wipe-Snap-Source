#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TomatinaSoundCue.h"         // FTomatinaSoundCue
#include "TomatinaTargetBase.h"
#include "TomatinaGameMode.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UTexture2D;
class USoundBase;
class UAudioComponent;
class ATomatinaTargetSpawner;
class ATomatinaHUD;
class ATomatoDirtManager;
class ATomatinaTowelSystem;

USTRUCT(BlueprintType)
struct FMissionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName TargetType;

	UPROPERTY(EditAnywhere)
	FText DisplayText;

	// 0以下なら無制限。
	UPROPERTY(EditAnywhere)
	float TimeLimit = 15.0f;

	// TargetClassVariantsが空のときのフォールバック。
	UPROPERTY(EditAnywhere)
	TSubclassOf<ATomatinaTargetBase> TargetClass;

	// 空でなければ、SpawnCount体それぞれにランダム候補を使う。
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<ATomatinaTargetBase>> TargetClassVariants;

	UPROPERTY(EditAnywhere)
	int32 SpawnCount = 1;

	UPROPERTY(EditAnywhere)
	FName SpawnProfileName;

	UPROPERTY(EditAnywhere)
	UTexture2D* TargetImage = nullptr;
};

USTRUCT(BlueprintType)
struct FFanfareTier
{
	GENERATED_BODY()

	// 条件を満たす中で一番高いMinScoreを使う。
	UPROPERTY(EditAnywhere)
	int32 MinScore = 0;

	UPROPERTY(EditAnywhere)
	USoundBase* Sound = nullptr;

	UPROPERTY(EditAnywhere, meta=(ClampMin="0.0", ClampMax="4.0"))
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, meta=(ClampMin="0.1", ClampMax="4.0"))
	float PitchMultiplier = 1.0f;
};

UENUM(BlueprintType)
enum class EStylishRank : uint8
{
	C   UMETA(DisplayName="C"),
	B   UMETA(DisplayName="B"),
	A   UMETA(DisplayName="A"),
	S   UMETA(DisplayName="S"),
	SSS UMETA(DisplayName="SSS"),
};

UCLASS()
class TOMATO_API ATomatinaGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATomatinaGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category="Tomatina|Mission")
	TArray<FMissionData> Missions;

	UPROPERTY(EditAnywhere, Category="Tomatina|Mission")
	bool bRandomOrder = true;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|Mission")
	int32 CurrentMissionIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|Mission")
	FName CurrentMission;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|Score")
	int32 CurrentScore = 0;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|Score")
	int32 TotalScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Photo")
	UTextureRenderTarget2D* RT_Photo = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Audio")
	USoundBase* ShutterSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Audio")
	USoundBase* BGM = nullptr;

	// ゲームプレイ中の観客ループ。最終リザルト前に停止する。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Audio")
	USoundBase* CrowdAmbient = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Audio", meta=(ClampMin="0.0", ClampMax="4.0"))
	float CrowdAmbientVolume = 1.0f;

	// 写真スコア別のファンファーレ。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Audio")
	TArray<FFanfareTier> FanfareTiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Audio|Countdown")
	FTomatinaSoundCue CountdownTickSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Audio|Countdown")
	FTomatinaSoundCue CountdownGoSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Audio|Mission")
	FTomatinaSoundCue MissionStartSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Audio|Mission")
	FTomatinaSoundCue TimeUpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Audio|Result")
	FTomatinaSoundCue FinalBuildupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Audio|Result")
	FTomatinaSoundCue FinalResultRevealSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Result")
	float ResultDisplayTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Result")
	float MissionResultDisplayTime = 2.0f;

	// 最終リザルト前の溜め。BGM以外の音停止と汚れ全削除を行う。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Result", meta=(ClampMin="0.0", ClampMax="10.0"))
	float FinalResultBuildupTime = 1.5f;

	// 0以下ならMissions[].TimeLimitの合計を使う。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|GameTime")
	float GameTimeOverride = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|GameTime")
	float GameTimeRemaining = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|GameTime")
	float GameTimeTotal = 0.f;

	// 汚れ1個あたりの減点。負の値を想定。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Score")
	int32 DirtPenaltyPerSplat = -5;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|Stylish")
	EStylishRank StylishRank = EStylishRank::C;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|Stylish")
	float StylishGauge = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|Stylish")
	int32 StylishComboCount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|Stylish")
	float DirtCoverage01 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	float StylishGaugeMax = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	float StylishBaseDecayPerSecond = 3.0f;

	// 汚れが増えるほどランク維持が難しくなる。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish", meta=(ClampMin="0.0", ClampMax="1.0"))
	float StylishDirtDangerThreshold = 0.72f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish", meta=(ClampMin="0.0", ClampMax="1.0"))
	float StylishDirtCriticalThreshold = 0.92f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	float StylishDirtDangerDecayPerSecond = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	float StylishDirtCriticalDecayPerSecond = 32.0f;

	// この点数以上をテンポコンボ対象にする。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	int32 StylishGoodShotThreshold = 60;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	float StylishTempoWindow = 4.0f;

	// 写真スコアからゲージ加算量への係数。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	float StylishScoreToGaugeScale = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	float StylishTempoBonusGauge = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	float StylishMissPenalty = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	bool bResetComboOnMiss = true;

	// Cは0。以降はゲージ値でランクを決める。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	float StylishThresholdB = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	float StylishThresholdA = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	float StylishThresholdS = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	float StylishThresholdSSS = 90.0f;

	// このランク以上でターゲット側の追加演出を解放。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stylish")
	EStylishRank HiddenActionMinRank = EStylishRank::S;

	UFUNCTION(BlueprintImplementableEvent, Category="Tomatina|Stylish")
	void OnStylishRankChanged(EStylishRank NewRank, EStylishRank OldRank, bool bRankUp);

	UFUNCTION(BlueprintImplementableEvent, Category="Tomatina|Stylish")
	void OnHighStylishShot(ATomatinaTargetBase* Target, EStylishRank Rank, int32 ShotScore);

	// Spawnerなどの準備完了に加えて、この時間だけロード表示を保つ。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Loading", meta=(ClampMin="0.0", ClampMax="10.0"))
	float LoadingHoldSeconds = 1.2f;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|State")
	bool bIsLoading = false;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|State")
	bool bInCountdown = false;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|State")
	bool bIsShowingResult = false;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|State")
	bool bIsShowingMissionResult = false;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|State")
	bool bIsBuildingUpFinalResult = false;

	UPROPERTY(BlueprintReadOnly, Category="Tomatina|State")
	float RemainingTime = -1.f;

	UFUNCTION(BlueprintCallable, Category="Tomatina|Photo")
	void TakePhoto(USceneCaptureComponent2D* ZoomCamera);

	UFUNCTION(BlueprintCallable, Category="Tomatina|Mission")
	void StartMission(int32 Index);

private:
	// PlayerPawnから取得してキャッシュする画面サイズ。
	float MainWidth   = 2560.f;
	float MainHeight  = 1600.f;
	float PhoneWidth  = 2024.f;
	float PhoneHeight = 1152.f;

	float CountdownRemaining = 0.f;
	int32 LastCountdownSecond = -1;

	float LoadingElapsed = 0.f;

	bool IsLoadingComplete() const;

	void BeginCountdownAfterLoading();
	bool TickLoading(float RealDelta);
	bool TickCountdown(float RealDelta);
	bool TickGameTime(float DeltaSeconds);
	bool TickPhotoResult(float RealDelta);
	bool TickMissionResult(float RealDelta);
	bool TickFinalResultBuildup(float RealDelta);
	void TickMissionTimer(float DeltaSeconds);

	// FApp::GetDeltaTimeで進めるリザルト用タイマー。
	float ResultElapsed = 0.f;
	float MissionResultElapsed = 0.f;
	float FinalBuildupElapsed = 0.f;

	// falseなら同じミッションを続ける。
	bool bLastPhotoSucceeded = false;

	UPROPERTY()
	ATomatinaTargetSpawner* TargetSpawner = nullptr;

	// StopAllSoundsExceptBGMで残すために保持。
	UPROPERTY()
	UAudioComponent* BGMAudioComp = nullptr;

	UPROPERTY()
	UAudioComponent* CrowdAudioComp = nullptr;

	void ShuffleMissions();
	void BeginFinalResultBuildup();
	void ShowFinalResult();
	void StopAllSoundsExceptBGM();
	void PlayFanfareForShotScore(int32 ShotScore);
	ATomatinaHUD* GetTomatinaHUD() const;
	void CachePlayerPawnSizes();
	ATomatoDirtManager* GetDirtManager();
	float CalculateDirtCoverage01();
	void UpdateStylishGauge(float RealDelta);
	void AddStylishGaugeFromShot(int32 ShotScore);
	void ApplyStylishRankFromGauge();
	FName GetStylishRankName(EStylishRank Rank) const;
	void PushStylishStateToHUD();
	void ForceTowelToCenterAfterCountdown();

	UPROPERTY()
	ATomatoDirtManager* CachedDirtManager = nullptr;

	float LastHighScoreShotTime = -1000.f;

	// 最終結果画面用の平均ランク・1P2Pシンクロ率。
	float StylishRankSum = 0.f;

	int32 StylishRankSampleCount = 0;

	int32 TotalPhotoAttempts = 0;

	int32 SyncPhotoCount = 0;

public:
	// 撮影直前に2Pが拭いていれば同期扱いにする猶予。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tomatina|Stats")
	float SyncWindowSeconds = 2.0f;

private:
	// レベルにタオルシステムがない場合の保険。
	UPROPERTY(EditAnywhere, Category="Tomatina|Towel")
	bool bEnsureTowelSystemExists = true;

	// BP派生を使う場合はGameMode Detailsで設定する。
	UPROPERTY(EditAnywhere, Category="Tomatina|Towel", meta=(EditCondition="bEnsureTowelSystemExists"))
	TSubclassOf<ATomatinaTowelSystem> TowelSystemClass;

	UPROPERTY()
	ATomatinaTowelSystem* CachedTowelSystem = nullptr;

	void EnsureTowelSystemExists();
};
