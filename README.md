# AviUtl プラグイン - エディットボックス最適化 (sigma-axis 改造版)

蛇色様の [エディットボックス最適化プラグイン](https://github.com/hebiiro/AviUtl-Plugin-OptimizeEditBox) のバージョン 8.0.0 を元に機能追加，バグ修正，自分好みのアレンジを加えたものです．

## 改造のきっかけ

蛇色様のオリジナル版が 8.0.0 の更新で余計な機能を大幅に省いたのですが，その中に自分が重要視している文字入力の遅延機能が含まれていました．
それ以前にもいくらか不具合が確認されていたのを自前で修正したものを自分用に使用していたのですが，これをきっかけに元のコードと似ても似つかないくらいに手を入れることにしました．
コードこそ大幅に変わっているものの，手法そのものは同じです．

## 導入時の注意点

蛇色様のオリジナル版の設定ファイル `OptimizeEditBox.ini` とは互換性がありません．[こちらの手順](#移行の手順) に従って修正してください．

## オリジナル版 8.0.0 からの変更点

### 削除された機能の復活
	
エディットボックスに文字を入力した際，1文字入力するごとに画面の更新がされていたのを遅延させ，ある程度まとまった文字が入力されてから更新する機能．
	
個人的には最も重要視している機能の 1 つで，私の場合 `editBoxDelay=10` を指定して IME 日本語入力確定時に確定文字がまとめて入力されるようにしています．これがないと確定時に1文字ずつ入力され，重いシーンでのテキスト編集が面倒になります．

これに付随して以下の不具合も修正．

-	[SplitWindow](https://github.com/hebiiro/AviUtl-Plugin-SplitWindow) と併用時，別テキストオブジェクトをマウスで選択と同時にドラッグすると，ドラッグ操作がキャンセルされる不具合がありました．
	正確な不具合の仕組みは不明ですが，遅延効果の発動条件を厳しくすることで修正しました．

また，エディットボックスからフォーカスを外したタイミングで，遅延量の指定にかかわらず即座に変更を反映させるような仕様に変更しています．

### 新しい機能の追加

1.	エディットボックスの TAB 文字の横幅を指定できるようにした．

	拡張編集のテキストオブジェクトとしては実質ゼロ幅空白として無視される文字ですが `<p,>`
	制御文字やスクリプト制御で利用してテキストを見やすくすることができます．
	既定値だと大きすぎて使いづらかったので実装しました．

1.	タイムライン上のオブジェクトの枠を，選択中かそうでないかで別指定できるようにした．

	特に高 DPI 環境だと選択中オブジェクトを囲むある点線が細く薄く表示されてしまうため見辛かったのを解消できます．

	![選択中の枠線が見やすい](https://github.com/sigma-axis/AviUtl-Plugin-OptimizeEditBox/assets/132639613/f773427a-d591-4bb2-aae8-2523c622a56a)

	それに加えて以下のように仕様を微修正：
	1. 枠線の色に `-1` を指定すると描画しないように変更．これで内側の枠線のマージンを設定できるように．
	1. 枠線の太さを上下左右，4つの数値で別々に指定できるように．
	1. それに伴って**設定ファイルの枠線指定に互換性がなくなりました．**

1.	オブジェクトのグラデーション描画のオプションを増やした．

	オリジナル版の「大雑把」 (ソースのコメントから抜粋) なグラデーションに加えて，デフォルトの処理に近いグラデーションと
	patch.aul と同様の階段状の描画も指定できるように．
	
	`.ini` ファイルの `gradientSteps` の指定で選べます．

	-	`-2` を指定: 単色表示．

		![各区間を単色で描画](https://github.com/sigma-axis/AviUtl-Plugin-OptimizeEditBox/assets/132639613/a00ab7f6-a114-403b-a377-1eaac7bad1f9)


	-	`-1` を指定: オリジナル版の簡易グラデーション．

		![オリジナル版のグラデーション](https://github.com/sigma-axis/AviUtl-Plugin-OptimizeEditBox/assets/132639613/21ae9041-7eaa-461e-93d9-35db11eda3b7)﻿
		-	デフォルトに比べてオブジェクト端をドラッグ中の見え方が違います．

	-	`0` を指定: AviUtl デフォルトのグラデーション．

		![デフォルトのグラデーション](https://github.com/sigma-axis/AviUtl-Plugin-OptimizeEditBox/assets/132639613/0ec24349-e18a-4dd6-9db6-ae8af4d45109)

	-	`1` 以上を指定: patch.aul と同様の階段状グラデーション．

		![階段状のグラデーション](https://github.com/sigma-axis/AviUtl-Plugin-OptimizeEditBox/assets/132639613/f6e37eea-3cd6-4e7d-b959-07577431bb92)

		段数が `4` の場合の見え方．

		-	`1` と `-2` の違いは，オブジェクト端をドラッグ中の見え方で確認できます．

			-	`-2` (単色表示) の場合．

				![端をドラッグ中も単色](https://github.com/sigma-axis/AviUtl-Plugin-OptimizeEditBox/assets/132639613/a00ab7f6-a114-403b-a377-1eaac7bad1f9)

			-	`1` (1段の階段グラデーション) の場合．

				![端をドラッグ中は一部だけ色が違う](https://github.com/sigma-axis/AviUtl-Plugin-OptimizeEditBox/assets/132639613/e4260cfe-6375-4faf-82a4-4f6aec92e30c)

		-	厳密には patch.aul の見え方とは異なります．

1.	選択中オブジェクトを囲む点線を非表示にする機能を追加．

*patch.aul 導入下で (2), (3) の機能を利用するためには，patch.aul の設定で `fast_exeditwindow` 内の `step` に
`-1` を指定して patch.aul のグラデーション描画機能を無効化してください．*

### バグ修正・細かな変更

1.	エディットボックスのフォント変更が，エディットボックスの高さ変更 (`addTextEditBoxHeight` や `addScriptEditBoxHeight` の指定)
	を設定しなければ反映されなかったのを修正．

1.	Unicode 文字入力 (`usesUnicodeInput`) を有効化しているとエディットボックスにフォーカスがあるとき
	ESC キーでダイアログが閉じてしまうのを修正．

1.	同じく Unicode 文字入力を有効化しているとエディットボックスに
	TAB キーで TAB 文字が入力できなかったのを修正．
		
1.	設定ファイルの色指定が R, G, B が別々に分かれていたり，馴染みの薄い `0xBBGGRR` 形式の指定だったりしていたのを
	`0xRRGGBB` 形式に統一．**これによって設定ファイルに互換性がなくなりました．**

1.	フォント指定を UTF8 として読み込むように変更．例は確認できてませんが，
	Shift-JIS で32バイトを超える名前のフォントも使用可能になるはず．
	現状最大は UTF16 で32文字以下 (終端 null を含む) となります．

1.	拡張編集 0.92 が確認できなかった場合，警告のメッセージボックスを表示するように．
	(以前はバージョンが 0.92 であることのチェックをしていなかった．)


# 使い方

***※ 以下は蛇色様のオリジナル版の README を改造版に合わせて加筆修正したものになります．***

エディットボックスのサイズやフォントを変更したり、ユニーコード文字列を入力できるようにしたりします。拡張編集の冗長なフレーム更新処理を抑制もします。<br>
[最新バージョンをダウンロード](../../releases/latest/)

![綺麗にグラデーション](https://user-images.githubusercontent.com/96464759/152974130-bcda58c8-fdab-43fa-96fa-bfbe091975f2.png)

## エディットボックスで1文字入力するたびに再計算するのは冗長だと感じている人向け

このプラグインはエディットボックス編集時の再計算を遅延させることでこの問題を解決します。  
設定ファイルで指定した遅延時間内に次の文字を入力すると再計算が省略されて遅延時間が延長されます。  
最後の入力が終わってから遅延時間が経過した時点でフレームの再計算が始まります。  

## IME で UNICODE 文字を入力したい人向け
```usesUnicodeInput``` を ON にすると、IME の日本語入力で UNICODE 文字を入力できるようになります。ただし、サロゲートペアには未対応です。

## タイムラインの描画がおかしいと感じる人向け
高 DPI の設定をしているとタイムラインのグラデーション描画が特におかしくなります。
```usesGradientFill``` を ON にすると、OS の API でグラデーションを描画するようになります。※eclipse_fast.auf と似たような機能です。

## 導入方法

以下のファイルを AviUtl の Plugins フォルダに入れてください。
* OptimizeEditBox.auf
* OptimizeEditBox.ini

## 設定方法

OptimizeEditBox.ini をテキストエディタで編集してから AviUtl を起動します。*エンコード形式は UTF8 で保存してください．*

```ini
[Settings]
editBoxDelay=0
; エディットボックスの再計算遅延時間をミリ秒で指定します。
; 例えば 1 秒遅延させたい場合は 1000 を指定します。
; 遅延させたくない場合は 0 を指定します。
usesUnicodeInput=0 ; テキストオブジェクトで UNICODE 文字を入力したい場合は 1 を指定します。
usesCtrlA=0 ; エディットボックスで Ctrl+A を有効にしたい場合は 1 を指定します。ただし、usesUnicodeInput が 1 のときのみ有効になります。
            ; 追記: aviutl_dark.exe 経由で起動すると (DarkenWindow.aul がなくても) Ctrl+A が自動で有効になるため不要です．

addTextEditBoxHeight=0 ; テキストオブジェクトのエディットボックスの高さに加算する値を指定します。
                       ; 例えば、200 を指定するとエディットボックスの高さが通常より 200 ピクセル高くなります。
addScriptEditBoxHeight=0 ; スクリプト制御のエディットボックスの高さに加算する値を指定します。
fontName=Segoe UI ; エディットボックスで使用するフォントのフォント名を指定します。
fontSize=14 ; フォントのサイズを指定します。
fontPitch=1 ; 固定幅を指定する場合は 1 を指定します。
tabstopTextEditBox=0 ; テキストオブジェクトのエディットボックス内の TAB 文字の横幅．0 で変更なしで既定値の 32 と同等.
tabstopScriptEditBox=0 ; スクリプト制御のエディットボックス内の TAB 文字の横幅．

usesGradientFill=0 ; グラデーション描画を変更する場合は 1 を指定します。
                   ; ただし、patch.aul のグラデーション描画を無効 ("fast_exeditwindow.step" を -1) にしている場合のみ有効になります。
gradientSteps=-1 ; グラデーションの種類．0 でデフォルトに近いもの，-1 でオリジナル版の簡易グラデーション，-2 で単色表示．
                 ; 1 以上で patch.aul の機能と同様の階段状のグラデーションで，段数を指定．
hideDotOutline=0 ; 選択中オブジェクトを囲むデフォルトの点線を非表示にするには 1 を指定します．

selectionColor=-1 ; フレーム範囲選択領域の内部の色。色は 0xRRGGBB の形式で指定する。-1 の場合は指定なし。
selectionEdgeColor=-1 ; フレーム範囲選択領域の端の色。
selectionBkColor=-1 ; フレーム範囲選択領域の外側の色。

layerBorderLeftColor=-1 ; レイヤー間ボーダーの左側の色。-1 で指定なし．
layerBorderRightColor=-1 ; レイヤー間ボーダーの右側の色。
layerBorderTopColor=-1 ; レイヤー間ボーダーの上側の色。
layerBorderBottomColor=-1 ; レイヤー間ボーダーの下側の色。
layerSeparatorColor=-1 ; レイヤーボタンとレイヤーの間の境界線の色。

; タイムラインの未選択オブジェクトの枠線の設定．usesGradientFill が 1 のときのみ有効．
[ObjectFrame]
outerColor=0x000000 ; 外側の枠の色．-1 で描画なし (枠の太さ分だけ内側の枠のマージンが残る).
outerLeft=1 ; 外側の枠の左の太さ． 0 から 255 まで．
outerRight=1 ; 外側の枠の右の太さ．
outerTop=1 ; 外側の枠の上の太さ．
outerBottom=1 ; 外側の枠の下の太さ．
innerColor=0xffffff ; 内側の枠の色．-1 で描画なし．
innerLeft=1 ; 内側の枠の左の太さ．以下同様．
innerRight=1
innerTop=1
innerBottom=1

; タイムラインの選択中オブジェクトの枠線の設定．usesGradientFill が 1 のときのみ有効．
; このセクションをコメントアウトまたは削除すると未選択オブジェクトの設定を引き継ぎます．
[SelectedObjectFrame]
outerColor=0x000000
outerLeft=1
outerRight=1
outerTop=1
outerBottom=1
innerColor=0xffffff
innerLeft=1
innerRight=1
innerTop=1
innerBottom=1

```

## 更新履歴

###	改造版

-	8.0.0_mod3.1

	2023/07/24

	- 階段グラデーションの段数が `1` の場合の仕様変更，旧仕様は `-2` で指定するように．
 	- タイムライン描画や遅延タイマー周りのコード整理．
	- ビルドオプションや自動ビルド周りの微修正．

-	8.0.0_mod3

	2023/07/14

	- オブジェクト枠線の指定を，選択中かそうでないかで別指定できるように．
	- デフォルトの選択中枠線の点線を非表示にできるように．
	- オブジェクトのグラデーション描画の種類を増やした．
	- `layerBorderTopColor`, `layerBorderBottomColor` の色指定をした場合，枠線の座標がおかしかったのを修正 (8.0.0_mod からのバグ).
	- プラグイン情報に改造版のバージョン番号を追加．
	- 使用ライブラリのライセンス文を同梱し忘れていたのを追加．
	- コード整理．
	
	**設定ファイルの形式が変わったため [移行の手順](#移行の手順) や同梱の設定ファイルを参考に編集をお願いします．**

-	8.0.0_mod2

	2023/07/08

	- Unicode 文字入力有効時に TAB キー挿下で TAB 文字が入力できなかったなどの不具合修正．
	- コード整理．

-	8.0.0_mod	

	2023/07/02 - 改造版公開．

	- 文字入力遅延機能の復活．
	- TAB 文字の横幅指定機能の追加．
	- バグ修正．

###	オリジナル版

* 8.0.0 - 2023/05/26 余計な機能を削除
* 7.1.1 - 2022/04/10 設定ダイアログのフックを安全な方法に変更
* 7.1.0 - 2022/03/17 レイヤー左側のセパレータの色を変更する機能を追加
* 7.0.1 - 2022/03/15 レイヤーの枠をデフォルト色に戻せない問題を修正
* 7.0.0 - 2022/03/07 エディットボックスのフォントを変更する機能を追加
* 6.1.0 - 2022/02/28 スクリプト制御の高さも変えられるように修正
* 6.0.0 - 2022/02/28 エディットボックスの高さを変更する機能を追加
* 5.0.0 - 2022/02/12 選択領域外の色などを調整する機能を追加
* 4.0.0 - 2022/02/11 枠線描画の調整機能を追加
* 3.0.0 - 2022/02/08 グラデーションを綺麗にする機能などを追加
* 2.1.1 - 2022/01/08 スピンボタンによる編集が無効になる問題を修正

## 動作確認

* (必須) AviUtl 1.10 & 拡張編集 0.92 http://spring-fragrance.mints.ne.jp/aviutl/
* (共存確認) patch.aul r43_ss29 https://github.com/nazonoSAUNA/patch.aul

## クレジット

* Microsoft Research Detours Package https://github.com/microsoft/Detours
* aviutl_exedit_sdk https://github.com/ePi5131/aviutl_exedit_sdk
* Common Library https://github.com/hebiiro/Common-Library

## 作成者情報
 
* 作成者 - 蛇色 (へびいろ)
* GitHub - https://github.com/hebiiro
* Twitter - https://twitter.com/io_hebiiro

*改造版についての不具合報告等はオリジナル版の作成者ではなく，私 sigma-axis にまでご連絡ください．* [連絡先](#連絡バグ報告)

## 免責事項

この作成物および同梱物を使用したことによって生じたすべての障害・損害・不具合等に関しては、私と私の関係者および私の所属するいかなる団体・組織とも、一切の責任を負いません。各自の責任においてご使用ください。


## 移行の手順

蛇色様のオリジナル版から移行するための手順です．

1.	`OptimizeEditBox.auf` を改造版に上書きコピーします．
1.	`OptimizeEditBox.ini` を次の手順に従って編集します．(設定を始めからやり直したい場合は同梱のものを上書きコピーでも構いません．)
	1.	ファイルの末尾に `[ObjectFrame]` と書いた行を追加します．

	1.	`outerColorR`, `outerColorG`, `outerColorB` をまとめて `outerColor` に書き換え,
		`[ObjectFrame]` の次の行に配置します．色指定は `0xRRGGBB` の形式です．

		例：

		```ini
		[Settings]
		...
		outerColorR=0x80
		outerColorG=0xc0
		outerColorB=0xff
		...
		```
		
		↓
		
		```ini
		[ObjectFrame]
		outerColor=0x80c0ff
		```

	1.	`outerWidth` を `outerLeft`, `outerRight` の2項目に書き換えて, `[ObjectFrame]` 以下の行に移動します．
		`outerHeight` も同様に `outerTop`, `outerBottom` として移動します．

		例：

		```ini
		[Settings]
		...
		outerWidth=2
		outerHeight=1
		...
		```
		
		↓
		
		```ini
		[ObjectFrame]
		...
		outerLeft=2
		outerRight=2
		outerTop=1
		outerBottom=1
		```
	
	1.	`innerColorR`, `innerColorG`, `innerColorB` も同様に `innerColor` まとめて `[ObjectFrame]` 以下に移動します．

		`innerWidth`, `innerHeight` も同様に `innerLeft`, `innerRight`, `innerTop`, `innerBottom` として移動します．

	1.	その他色指定の項目で， `-1` でないものがあれば，それらを `0xBBGGRR` から `0xRRGGBB` の形式に書き換えます．

		例:
		
		`0xffc080` → `0x80c0ff`

	1.	(任意) 必要なら `[SelectedObjectFrame]` セクションも末尾に追加して，
		`[ObjectFrame]` と同様に選択中オブジェクトの枠線設定も指定します．

	1.	(任意) 必要なら以下の4行を `[Settings]` セクションの末尾に追加します．

		```ini
		tabstopTextEditBox=0 ; テキストオブジェクトのエディットボックス内の TAB 文字の横幅．0 で変更なしで既定値の 32 と同等.
		tabstopScriptEditBox=0 ; スクリプト制御のエディットボックス内の TAB 文字の横幅．
		gradientSteps=0 ; グラデーションの種類．0 でデフォルトに近いもの，-1 でオリジナル版の簡易グラデーション，
		                ; 1 以上で patch.aul の機能と同様の階段状のグラデーション．
		hideDotOutline=0 ; 選択中オブジェクトを囲むデフォルトの点線を非表示にするには 1 を指定します．
		```

	1.	最後に UTF8 形式で保存します．(エディットボックスのフォント指定に日本語文字が含まれる場合必須．)


## 謝辞

このような素晴らしいプラグインを制作，公開してくださった蛇色様に感謝申し上げます．
AviUtl を使うたびに必ずお世話になっているプラグインです．

加えてエディットボックスの Unicode 化の手法や各種関数のアドレスを探して上書きする手法は
少なくとも私個人では見つけられませんでした．ソースコードを公開してくださったことにも重ねて感謝申し上げます．

## 連絡・バグ報告

*改造版についての不具合報告等はオリジナル版の作成者ではなく，私 sigma-axis にまでご連絡ください．*

**sigma-axis**
-	GitHub: https://github.com/sigma-axis
-	Twitter: https://twitter.com/sigma_axis
-	nicovideo: https://www.nicovideo.jp/user/51492481
