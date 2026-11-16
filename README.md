# Wipe&Snap

Unreal Engine 5 / C++ で作った、2人協力の撮影アクションゲームです。
1P は Joy-Con を付けたカメラ型コントローラーとスマートフォンのファインダーで被写体を撮り、
2P は Leap Motion を使って、レンズについたトマトの汚れを手で拭き取ります。

このリポジトリには、チームの開発リポジトリから、私の担当部分を中心に C++ のソースだけを抜き出して置いています。

## 動画・記事

- [プレイ動画（YouTube）](https://youtu.be/YlQxtFcei3Y)
- [技術記事（Zenn）](https://zenn.dev/ippon/articles/32ad6608d7eb30)

Leap Motion などの展示用の機材がないと遊べないため、ビルドは公開していません。

## 開発情報

| 項目 | 内容 |
| --- | --- |
| ジャンル | 2人協力の撮影アクション |
| 制作期間 | 2026年3月〜5月（約2か月） |
| チーム | 4人（プログラマー2人・デザイナー1人・プランナー1人） |
| 使用技術 | Unreal Engine 5 / C++ / Leap Motion Controller |
| 私の役割 | リーダー、プログラマー |

## 担当したこと

私の担当部分はほぼすべて C++ で、Blueprint の部分はもう1人のプログラマーが作っています。

- スマートフォン用ファインダーの別ウィンドウ表示（Slate の `SWindow`）
- ファインダー映像の描画（`SceneCapture2D` / `RenderTarget`）
- Leap Motion でのタオル操作、汚れの拭き取り判定、タオルの耐久値
- 構図のプレビュー表示と撮影判定
- ズームカメラの動きと、カメラが壁にめり込まないようにする処理
- カメラ型コントローラーの入力（Joy-Con のボタンは Enhanced Input で割り当て、ジャイロは Steam Input でマウスカーソルの動きに変えて使用）
- ファインダー表示とトマトの演出まわりの描画負荷の確認

ほかのメンバーは、3Dアセット、アニメーションの設定、レベルの配置、イラスト、ルールの検討、カメラ型コントローラーの外側（筐体）の制作などを担当しました。

## 主なファイル

| 機能 | ファイル | 内容 |
| --- | --- | --- |
| スマホのファインダー | [TomatinaHUD.cpp](Source/Tomato/TomatinaHUD.cpp) | 別ウィンドウの作成、マテリアル経由での映像表示 |
| ファインダー映像の更新 | [TomatinaPlayerPawn.cpp](Source/Tomato/TomatinaPlayerPawn.cpp) | 必要なときだけ映像を描く処理、構図判定の間隔 |
| Leap Motion のタオル入力 | [TomatinaTowelSystem.cpp](Source/Tomato/TomatinaTowelSystem.cpp) | 手の位置の変換、平滑化、手を見失ったときの処理、拭き取る位置の計算 |
| 汚れの管理 | [TomatoDirtManager.cpp](Source/Tomato/TomatoDirtManager.cpp) | 汚れの位置と濃さの管理 |
| 撮影判定・構図プレビュー | [TomatinaFunctionLibrary.cpp](Source/Tomato/TomatinaFunctionLibrary.cpp)、[TomatinaGameMode.cpp](Source/Tomato/TomatinaGameMode.cpp) | 撮影とプレビューで共通の `EvaluatePhotoFraming` |
| ターゲット・トマト | [TomatinaTargetBase.cpp](Source/Tomato/TomatinaTargetBase.cpp)、[TomatinaProjectile.cpp](Source/Tomato/TomatinaProjectile.cpp)、[TomatinaProjectileSpawner.cpp](Source/Tomato/TomatinaProjectileSpawner.cpp) | 撮影するターゲットと、投げられるトマトの処理 |

## 技術的な工夫

### スマートフォンを別ウィンドウにした

最初は1つのウィンドウを、メインモニターとスマートフォンの2画面にまたがせていました。
`SWindow` でスマートフォン用のウィンドウを分けてからは、展示前に合わせるのはそのウィンドウの位置と大きさだけになりました。
長いときは約1時間かかっていた画面合わせが、数分で終わるようになっています。

### ファインダー映像の描画

ズームしたときの映像は、`SceneCapture2D` で `RenderTarget` に描いています。
別ウィンドウでは `RenderTarget` を画像に直接貼ると映像が更新されない問題があるため、マテリアル（Dynamic Material Instance）を通して毎フレーム映像を渡しています。

`SceneCapture2D` は毎フレーム自動では描かず、ズーム中など必要なときだけ `CaptureScene()` を呼んでいます。

### Leap Motion のタオル操作

Leap Motion で取った手の位置を画面上の位置に変えて、汚れの拭き取りに使っています。
Ultraleap Visualizer で確かめると、速く動かしたときに手を見失うのはデバイス側でも起きていました。
そこで、ゲーム側では起きていたことを4つに分けて、それぞれ次の処理を入れています。

| 起きていたこと | 入れている処理 |
| --- | --- |
| 手を一瞬見失う | 少しの間（コードの既定値で0.2秒）は直前の位置を保ち、速度を落としながら拭き取りを続ける |
| 位置が細かく揺れる | 位置を平滑化する |
| 拭き取りのオン・オフがばたつく | 拭き始めと拭き続けで、速度のしきい値を分ける |
| 速く動かすと汚れを飛び越える | 前のフレームの位置との間を線でつなぎ、途中の点でも拭き取る（1フレームで最大8点） |

手を Leap Motion に近づけすぎたときは、画面に「手を離してね」と出します。

### 入力と汚れの管理を分けた

タオル側は「画面のどこをどれだけ拭くか」を計算して `ATomatoDirtManager` に渡すだけにして、汚れの状態は `ATomatoDirtManager` が持っています。

### 構図のプレビュー

スマートフォンのファインダーに、今の構図が「全身」「上半身」「下半身」のどれになるかと、そのときの倍率を出します。構図が合っていないときは「構図を調整」と出ます。
プレビューと実際の撮影で判定がずれないように、どちらも同じ関数（`EvaluatePhotoFraming`）で判定しています。
プレビューの判定は0.1秒ごと（既定値）です。

## このリポジトリについて

このリポジトリだけではビルドできません。
元のプロジェクトのうち、第三者のアセットやプラグインなど再配布できないものと、ビルドで生成されるファイルは入れていません。

- `Content/`（`.uasset` / `.umap`）
- `Plugins/UltraleapTracking_ue5_4-5.0.1/`
- `Binaries/`、`Intermediate/`、`Saved/`、`DerivedDataCache/`
- 公開しない設定ファイル、エディタのローカル設定

ソースの一部は Ultraleap Tracking プラグイン（`UltraleapTracking` モジュール、`ULeapComponent`）を使っています。
ビルドするには、このプラグインと、公開していないアセット・設定が必要です。

## コミット履歴について

コミット履歴には、私と hato（實光駿斗）さんの変更が入っています。
hato さんの担当は、3Dアセットの調整、アニメーションの設定、観客・投げる人・ターゲットなどの配置、効果音の導入と設定です。

## ライセンス

ライセンスは付けていません。コードを読んでもらうための公開です。
