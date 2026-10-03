# Wipe&Snap

## 概要

『Wipe&Snap』は、Unreal Engine 5 / C++で制作した2人協力型の撮影アクションゲームです。
1Pはカメラ型コントローラーとスマートフォンのファインダー画面を使って被写体を撮影し、
2PはLeap Motionによる手入力でレンズに付着したトマト汚れを拭き取ります。

本リポジトリは、元のチーム開発リポジトリから公開可能な担当ソースコードをまとめたものです。

## 開発情報

| 項目 | 内容 |
| --- | --- |
| ジャンル | 2人協力型撮影アクションゲーム |
| 開発体制 | 4人チーム / 約2か月 |
| 使用技術 | Unreal Engine 5 / C++ / Leap Motion Controller |
| 自分の担当 | クライアント実装・外部デバイス連携・操作体験に関わる機能 |
| 公開範囲 | 自身の担当範囲に関するソースコード |

## デモ

この作品はLeap Motion等の展示機材を使用するため、公開Buildではなくプレイ動画と担当ソースを公開しています。

- [プレイ動画 - Wipe&Snap](https://youtu.be/YlQxtFcei3Y)
- [技術記事 - Zenn](https://zenn.dev/ippon/articles/32ad6608d7eb30)

## 自分の担当

本作は4人チームで制作しました。私は主に、プレイヤーの操作体験と
外部デバイス連携に関わるC++実装を担当しました。

- Slate `SWindow` を用いたスマートフォン用ファインダー表示
- `SceneCapture2D` / `RenderTarget` を用いたファインダー映像描画
- Leap Motionによるタオル操作入力
- 汚れ拭き取り判定と耐久値処理
- スマートフォン画面上のフレーミングプレビュー表示
- ズームカメラの挙動と壁抜け対策
- ファインダー表示およびトマト演出周辺の描画負荷確認

他メンバーは、3Dアセット、アニメーション設定、レベル配置、イラスト、
ルール検討、物理カメラ型コントローラー制作などを担当しています。
本リポジトリでは、それらを私個人の成果として扱いません。

## 主な実装

| 機能 | 主なファイル | 確認できる内容 |
| --- | --- | --- |
| スマホファインダー / SWindow | [TomatinaHUD.cpp](Source/Tomato/TomatinaHUD.cpp) | 独立Window生成、Dynamic Material経由のRenderTarget表示 |
| SceneCapture2D / 更新タイミング | [TomatinaPlayerPawn.cpp](Source/Tomato/TomatinaPlayerPawn.cpp) | 自動Capture無効化、明示的なCapture要求、フレーミング評価の間隔制御 |
| Leap Motionタオル入力 | [TomatinaTowelSystem.cpp](Source/Tomato/TomatinaTowelSystem.cpp) | 入力変換、平滑化、ゲート処理、拭き取り要求生成 |
| 汚れ拭き取り管理 | [TomatoDirtManager.cpp](Source/Tomato/TomatoDirtManager.cpp) | 入力処理と汚れ状態管理の責務分離 |
| 撮影判定 / フレーミングプレビュー | [TomatinaFunctionLibrary.cpp](Source/Tomato/TomatinaFunctionLibrary.cpp)、[TomatinaGameMode.cpp](Source/Tomato/TomatinaGameMode.cpp) | 撮影とプレビューから呼ぶ共通の `EvaluatePhotoFraming` |
| ターゲット / 投射物処理 | [TomatinaTargetBase.cpp](Source/Tomato/TomatinaTargetBase.cpp)、[TomatinaProjectile.cpp](Source/Tomato/TomatinaProjectile.cpp)、[TomatinaProjectileSpawner.cpp](Source/Tomato/TomatinaProjectileSpawner.cpp) | ゲーム内インタラクションと生成処理 |

## 技術的な工夫

### スマートフォン用ファインダー表示

メイン画面とは別にスマートフォン側の表示を管理するため、
Slate `SWindow` とスマートフォン表示用のウィジェットを使用しました。
これにより、展示環境でメインモニターとスマートフォン表示を独立して扱える構成にしました。

### SceneCapture2D / RenderTargetによる映像描画

ズームファインダーには `SceneCapture2D` と `RenderTarget` を使用しました。
スマートフォン側の表示を安定させるため、RenderTargetをウィジェットへ直接割り当てる方法ではなく、
Dynamic Material Instanceを経由して更新する構成にしました。

### Leap Motionによるタオル操作

Leap Motionから取得した手の動きを、汚れ拭き取りに使用する正規化画面座標へ変換しています。
意図しない反応を減らすため、入力ゲート、平滑化、短時間の入力猶予、
画面端処理、手がデバイスに近すぎる場合の警告表示を実装しました。

### 汚れ拭き取り処理の責務分離

タオル入力側では汚れテクスチャの状態を直接管理せず、
正規化した拭き取り要求を `ATomatoDirtManager` へ送る構成にしました。
入力処理と汚れ状態管理を分離し、調整・拡張しやすくしています。

### フレーミングプレビュー

スマートフォンのファインダー上で、現在の構図が全身・上半身・下半身・無効の
どれに該当するかを事前表示します。
撮影スコア判定と同じ副作用のない評価処理を用いることで、
プレビュー表示と実際の撮影結果がずれないようにしました。

### 更新処理の制御

`TomatinaPlayerPawn`ではSceneCaptureの自動更新を無効にし、明示的な要求でCaptureします。
フレーミングプレビューの評価は設定可能な間隔（既定0.1秒）で行います。
コードから更新方式は確認できますが、本公開ソースだけで負荷の改善率や実機での安定性を再計測したものではありません。

## 本リポジトリの位置付け

本リポジトリは、担当したコードの閲覧を目的とした公開リポジトリであり、
このリポジトリ単体でゲーム全体をビルド・実行することを目的としていません。

元プロジェクトには、Unreal Engineのバイナリアセット、Blueprintアセット、
マップ、第三者アセット、Ultraleap Trackingプラグインが含まれます。
これらは権利・再配布範囲がソースコードとは異なるため、本リポジトリには含めていません。

含めていない主なもの：

- `Content/`
- `.uasset` / `.umap`
- `Plugins/UltraleapTracking_ue5_4-5.0.1/`
- `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`
- 非公開設定およびローカルエディタ設定

一部のソースコードは `UltraleapTracking` モジュール、`ULeapComponent`、
`IUltraleapTrackingPlugin` を参照しています。
完全なローカル環境でのビルドには、対応するUltraleap Trackingプラグインと、
非公開のプロジェクトアセット・設定の復元が必要です。

2026-10-03の確認はREADMEの参照先と公開C++実装の照合です。
完全なBuild、PIE、Leap Motion・スマートフォン実機動作は今回未検証であり、コンパイル成功を保証しません。

## チーム制作・公開範囲について

### 制作メンバー

- 嶋田一歩：クライアント実装、外部デバイス連携、操作体験に関わる機能
- hato（實光駿斗）：3Dアセット調整、アニメーション設定、観客・投擲者・ターゲット等のオブジェクト配置、SE導入・設定

『Wipe&Snap』はチーム制作作品です。
本リポジトリは、再配布できないアセット、非公開設定、個人情報を除外するため、
元の開発リポジトリから公開用に履歴を抽出しています。

履歴には、共同開発者である hato（實光駿斗）の変更も含まれています。
公開履歴では、個人メールアドレスを公開しないため、GitHubが同氏に提供する
noreplyメールアドレスを使用しています。
これらの変更を私個人の成果として主張するものではありません。

本リポジトリには、第三者アセット、Unreal Engineのバイナリアセット、
マップ、プラグイン本体を再配布していません。

本公開リポジトリに対して、オープンソースライセンスは付与していません。
コード閲覧を目的として公開しています。
