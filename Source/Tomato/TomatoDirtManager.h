#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TomatoDirtManager.generated.h"

UENUM(BlueprintType)
enum class EDirtType : uint8
{
	NormalRed        UMETA(DisplayName="Normal Red"),
	StickyYellowDash UMETA(DisplayName="Sticky Yellow Dash x4"),
};

USTRUCT(BlueprintType)
struct FDirtSplat
{
	GENERATED_BODY()

	// 画面上の正規化座標。
	UPROPERTY(BlueprintReadWrite, Category="Dirt")
	FVector2D NormalizedPosition = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category="Dirt")
	float Opacity = 1.0f;

	// 正規化スケール。1.0で領域幅と同じサイズ。
	UPROPERTY(BlueprintReadWrite, Category="Dirt")
	float Size = 0.2f;

	UPROPERTY(BlueprintReadWrite, Category="Dirt")
	float FadeSpeed = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category="Dirt")
	bool bActive = true;

	// HUDのDirtTextures配列に対応。範囲外なら単体DirtTextureへフォールバック。
	UPROPERTY(BlueprintReadWrite, Category="Dirt")
	int32 TextureIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category="Dirt")
	EDirtType DirtType = EDirtType::NormalRed;

	// StickyYellowDash用。
	UPROPERTY(BlueprintReadWrite, Category="Dirt")
	int32 RequiredDashCount = 0;

	UPROPERTY(BlueprintReadWrite, Category="Dirt")
	int32 CurrentDashCount = 0;

	UPROPERTY(BlueprintReadWrite, Category="Dirt")
	float LastDashTime = -1000.0f;

	UPROPERTY(BlueprintReadWrite, Category="Dirt")
	FVector2D LastDashPos = FVector2D::ZeroVector;
};

UCLASS()
class TOMATO_API ATomatoDirtManager : public AActor
{
	GENERATED_BODY()

public:
	ATomatoDirtManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// トマト命中で湧かせる場合はfalse。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Config")
	bool bEnableAutoSpawn = false;

	// 汚れ生成/消去/HUD通知の調査用。通常はOFF。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Debug")
	bool bDebugDirtLog = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Config")
	float SpawnInterval = 3.0f;

	// 超過時は新規追加を無視する。既存の汚れは消さない。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Config")
	int32 MaxDirts = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Config", meta=(ClampMin="0.01", ClampMax="1.0"))
	float SpawnSizeMin = 0.16f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Config", meta=(ClampMin="0.01", ClampMax="1.0"))
	float SpawnSizeMax = 0.30f;

	// AddDirtのSize暴走防止。0以下で無効。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Config")
	float MaxDirtSize = 0.6f;

	// ここで制限するとスマホ/写真側も狭まる。メイン表示だけならHUD側で制限する。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Config", meta=(ClampMin="0.0", ClampMax="1.0"))
	FVector2D SpawnRangeMin = FVector2D(0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Config", meta=(ClampMin="0.0", ClampMax="1.0"))
	FVector2D SpawnRangeMax = FVector2D(1.0f, 1.0f);

	// HUDのDirtTextures配列サイズに合わせる。1以下なら単体DirtTexture。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Config", meta=(ClampMin="1"))
	int32 NumDirtVariants = 1;

	// HUDのDirtTexturesで黄色画像を割り当てる。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Special")
	int32 StickyTextureIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Special", meta=(ClampMin="1"))
	int32 StickyRequiredDashCount = 4;

	// WipeDirtAtのAmountをダッシュ判定に使う。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Special", meta=(ClampMin="0.01"))
	float StickyDashMinAmount = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Special", meta=(ClampMin="0.001"))
	float StickyDashMinMoveDistance = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Special", meta=(ClampMin="0.01"))
	float StickyDashMaxInterval = 0.28f;

	// 連続ダッシュ途中の進捗を見せるための下限。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dirt|Special", meta=(ClampMin="0.0", ClampMax="1.0"))
	float StickyProgressMinOpacity = 0.35f;

	UFUNCTION(BlueprintCallable, Category="Dirt")
	void SpawnDirt();

	UFUNCTION(BlueprintCallable, Category="Dirt")
	void AddDirt(FVector2D NormPos, float Size);

	// TextureIndexを-1にするとタイプから自動選択。
	UFUNCTION(BlueprintCallable, Category="Dirt")
	void AddDirtWithType(FVector2D NormPos, float Size, EDirtType DirtType, int32 InRequiredDashCount = 0, int32 InTextureIndex = -1);

	UFUNCTION(BlueprintCallable, Category="Dirt")
	void ClearDirtAt(FVector2D NormPos, float Radius);

	// ClearDirtAtは即時消去。こちらはタオル用の漸進的な拭き取り。
	UFUNCTION(BlueprintCallable, Category="Dirt")
	void WipeDirtAt(FVector2D NormPos, float Radius, float Amount);

	UFUNCTION(BlueprintCallable, Category="Dirt")
	TArray<FDirtSplat> GetActiveDirts() const;

	UPROPERTY(BlueprintReadOnly, Category="Dirt")
	TArray<FDirtSplat> DirtSplats;

	// 2P活動判定用。
	UPROPERTY(BlueprintReadOnly, Category="Dirt")
	float LastWipeRealTime = -1000.f;

	UFUNCTION(BlueprintCallable, Category="Dirt")
	float GetSecondsSinceLastWipe() const;

	UFUNCTION(BlueprintCallable, Category="Dirt")
	void ClearAllDirts();

private:
	float SpawnTimer = 0.0f;

	float ClampDirtSize(float Size) const;
	FVector2D ClampDirtSpawnPosition(FVector2D NormPos) const;
	int32 ResolveDirtTextureIndex(EDirtType DirtType, int32 InTextureIndex) const;
	bool WipeNormalDirt(FDirtSplat& Dirt, float Distance, float EffectRange, float Amount) const;
	bool WipeStickyDirt(FDirtSplat& Dirt, FVector2D NormPos, float Amount, float NowRealTime) const;
	bool ResetStickyDashIfExpired(FDirtSplat& Dirt, float NowRealTime) const;

	void NotifyHUD();
};
