﻿# AviUtl プラグイン - エディットボックス最適化 (sigma-axis 改造版)

蛇色様の [エディットボックス最適化プラグイン](https://github.com/sigma-axis/AviUtl-Plugin-OptimizeEditBox) のバージョン 8.0.0 を元に機能追加，バグ修正，自分好みのアレンジを加えたものです．

## 改造のきっかけ

蛇色様のオリジナル版が 8.0.0 の更新で余計な機能を大幅に省いたのですが，その中に自分が重要視している文字入力の遅延機能が含まれていました．
それ以前にもいくらか不具合が確認されていたのを自前で修正したものを自分用に使用していたのですが，これをきっかけに元のコードと似ても似つかないくらいに手を入れることにしました．
コードこそ大幅に変わっているものの，手法そのものは同じです．

## 導入時の注意点

蛇色様のオリジナル版の設定ファイル `OptimizeEditBox.ini` とは互換性がありません．[こちらの手順](#移行の手順) に従って修正してください．

## オリジナル版からの変更点

### 削除された機能の復活．
	
エディットボックスに文字を入力した際，1文字入力するごとに画面の更新がされていたのを遅延させ，ある程度まとまった文字が入力されてから更新する機能．
	
個人的には最も重要視している機能の 1 つで，私の場合 `editBoxDelay=10` を指定して IME 日本語入力確定時に確定文字がまとめて入力されるようにしています．これがないと確定時に1文字ずつ入力され，重いシーンでのテキスト編集が面倒になります．

これに付随して以下の不具合も修正．

-	[SplitWindow](https://github.com/hebiiro/AviUtl-Plugin-SplitWindow) と併用時，別テキストオブジェクトをマウスで選択と同時にドラッグすると，ドラッグ操作がキャンセルされる不具合がありました．
	正確な不具合の仕組みは不明ですが，遅延効果の発動条件を厳しくすることで修正しました．

また，エディットボックスからフォーカスを外したタイミングで，遅延量の指定にかかわらず即座に変更を反映させるような仕様に変更しています．

### 新しい機能の追加．

エディットボックスの TAB 文字の横幅を指定できるようにしました．
拡張編集のテキストオブジェクトとしては実質ゼロ幅空白として無視される文字ですが `<p,>`
制御文字やスクリプト制御で利用してテキストを見やすくすることができます．
既定値だと大きすぎて使いづらかったので実装しました．

### バグ修正・細かな変更

1.	エディットボックスのフォント変更が，エディットボックスの高さ変更 (`addTextEditBoxHeight` や `addScriptEditBoxHeight` の指定)
	を設定しなければ反映していなかったのを修正．

1.	Unicode 文字入力 (`usesUnicodeInput`) を有効化しているとエディットボックスにフォーカスがあるとき
	ESC キーでダイアログが閉じてしまうのを修正．
		
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
usesGradientFill=0 ; グラデーション描画を変更する場合は 1 を指定します。ただし、patch.aul のグラデーション描画を無効にしている場合のみ有効になります。
innerColor=0xffffff ; 内側の枠の色。
innerEdgeWidth=1 ; 内側の枠の横幅。0以下なら枠の左右は描画しない。
innerEdgeHeight=1 ; 内側の枠の縦幅。0以下なら枠の上下は描画しない。
outerColor=0x000000 ; 外側の枠の色。指定の仕方は内側の枠と同じ。
outerEdgeWidth=1
outerEdgeHeight=1
selectionColor=-1 ; 選択領域の色。色は 0xRRGGBB の形式で指定する。-1 の場合は指定なし。
selectionEdgeColor=-1 ; 選択領域端の色。
selectionBkColor=-1 ; 選択領域外の色。
layerBorderLeftColor=-1 ; レイヤー間ボーダーの左側の色。
layerBorderRightColor=-1 ; レイヤー間ボーダーの右側の色。
layerBorderTopColor=-1 ; レイヤー間ボーダーの上側の色。
layerBorderBottomColor=-1 ; レイヤー間ボーダーの下側の色。
layerSeparatorColor=-1 ; レイヤーボタンとレイヤーの間の境界線の色。
addTextEditBoxHeight=0 ; テキストオブジェクトのエディットボックスの高さに加算する値を指定します。例えば、200 を指定するとエディットボックスの高さが通常より 200 ピクセル高くなります。
addScriptEditBoxHeight=0 ; スクリプト制御のエディットボックスの高さに加算する値を指定します。
fontName=Segoe UI ; エディットボックスで使用するフォントのフォント名を指定します。
fontSize=14 ; フォントのサイズを指定します。
fontPitch=1 ; 固定幅を指定する場合は 1 を指定します。
tabstopTextEditBox=0 ; テキストオブジェクトのエディットボックス内の TAB 文字の横幅．0 で変更なしで既定値の 32 と同等.
tabstopScriptEditBox=0 ; スクリプト制御のエディットボックス内の TAB 文字の横幅．
```

## 更新履歴

* 改造版 8.0.0 - 2023/07/02 改造版公開．文字入力遅延機能の復活，TAB 文字の横幅指定機能の追加，バグ修正等．
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
* (共存確認) patch.aul r43_ss28 https://github.com/nazonoSAUNA/patch.aul

## クレジット

* Microsoft Research Detours Package https://github.com/microsoft/Detours
* aviutl_exedit_sdk https://github.com/ePi5131/aviutl_exedit_sdk
* Common Library https://github.com/hebiiro/Common-Library

## 作成者情報
 
* 作成者 - 蛇色 (へびいろ)
* GitHub - https://github.com/hebiiro
* Twitter - https://twitter.com/io_hebiiro

*改造版についての不具合報告等はオリジナル版の作成者ではなく，私 sigma-axis にまでご連絡ください．* [連絡先](#連絡・バグ報告)

## 免責事項

この作成物および同梱物を使用したことによって生じたすべての障害・損害・不具合等に関しては、私と私の関係者および私の所属するいかなる団体・組織とも、一切の責任を負いません。各自の責任においてご使用ください。


# 移行の手順

蛇色様のオリジナル版から移行するための手順です．

1.	`OptimizeEditBox.auf` を改造版に上書きコピーします．

1.	`OptimizeEditBox.ini` を次の手順に従って編集します．(設定を始めからやり直したい場合は同梱のものを上書きコピーでも構いません．)

	1.	`innerColorR`, `innerColorG`, `innerColorB` をまとめて `innerColor` に書き換えます．色指定は `0xRRGGBB` の形式です．
		
		例：

		```ini
		innerColorR=0x80
		innerColorG=0xc0
		innerColorB=0xff
		```
		
		↓
		
		```ini
		innerColor=0x80c0ff
		```
	
	1.	`outerColorR`, `outerColorG`, `outerColorB` も同様に `outerColor` まとめます．

	1.	その他色指定の項目で， `-1` でないものがあれば，それらを `0xBBGGRR` から `0xRRGGBB` の形式に書き換えます．

		例:
		
		> `0xffc080` → `0x80c0ff`

	1.	(任意) 必要なら以下の2行を末尾に追加します．

		```ini
		tabstopTextEditBox=0 ; テキストオブジェクトのエディットボックス内の TAB 文字の横幅．0 で変更なしで既定値の 32 と同等.
		tabstopScriptEditBox=0 ; スクリプト制御のエディットボックス内の TAB 文字の横幅．
		```

	1.	最後に UTF8 形式で保存します．(エディットボックスのフォント指定に日本語文字が含まれる場合必須．)


# 謝辞

このような素晴らしいプラグインを制作，公開してくださった蛇色様に感謝申し上げます．
AviUtl を使うたびに必ずお世話になっているプラグインです．

加えてエディットボックスの Unicode 化の手法や各種関数のアドレスを探して上書きする手法は
少なくとも私個人では見つけられませんでした．ソースコードを公開してくださったことにも重ねて感謝申し上げます．

# 連絡・バグ報告

*改造版についての不具合報告等はオリジナル版の作成者ではなく，私 sigma-axis にまでご連絡ください．*

-	sigma-axis
-	GitHub: https://github.com/sigma-axis
-	Twitter: https://twitter.com/sigma_axis
-	nicovideo: https://www.nicovideo.jp/user/51492481
