# AviUtl プラグイン - エディットボックス最適化

* version 7.0.0 by 蛇色 - 2022/03/07 エディットボックスのフォントを変更する機能を追加
* version 6.1.0 by 蛇色 - 2022/02/28 スクリプト制御の高さも変えられるように修正
* version 6.0.0 by 蛇色 - 2022/02/28 エディットボックスの高さを変更する機能を追加
* version 5.0.0 by 蛇色 - 2022/02/12 選択領域外の色などを調整する機能を追加
* version 4.0.0 by 蛇色 - 2022/02/11 枠線描画の調整機能を追加
* version 3.0.0 by 蛇色 - 2022/02/08 グラデーションを綺麗にする機能などを追加
* version 2.1.1 by 蛇色 - 2022/01/08 スピンボタンによる編集が無効になる問題を修正

拡張編集の冗長なフレーム更新処理を抑制します。

![綺麗にグラデーション](https://user-images.githubusercontent.com/96464759/152974130-bcda58c8-fdab-43fa-96fa-bfbe091975f2.png)

## (1) テキストオブジェクトがやたら重いと感じている人向け
現在の拡張編集ではタイムライン上でテキストオブジェクトを左クリックダウンした時点でフレームの更新処理が最低でも2回発生します。これは本来必要のない処理です。さらに左クリックアップ時に1回フレームの更新が行われます。  
  
その他のオブジェクトではフレームの更新が行われるのは左クリックアップ時の1回だけです。つまりテキストオブジェクトは左クリックで選択するのに他のオブジェクトの3倍時間がかかってしまいます。  
  
これは拡張編集がエディットボックスから送られてくる通知メッセージ(EN_CHANGE)に反応して冗長なフレームの更新処理を行っているのが原因です。  
  
よって、このプラグインは左クリック状態のときはエディットボックスの通知メッセージを無視するようにして、テキストオブジェクト選択時の処理を他のオブジェクトと同じくらいの軽さにします。  

## (2) エディットボックスで1文字入力するたびに再計算するのは冗長だと感じている人向け
このプラグインはエディットボックス編集時の再計算を遅延させることでこの問題を解決します。  
設定ファイルで指定した遅延時間内に次の文字を入力すると再計算が省略されて遅延時間が延長されます。  
最後の入力が終わってから遅延時間が経過した時点でフレームの再計算が始まります。  

## (3) IME で UNICODE 文字を入力したい人向け
```usesUnicodeInput``` を ON にすると、IME の日本語入力で UNICODE 文字を入力できるようになります。ただし、サロゲートペアには未対応です。

## (4) オブジェクト設定ダイアログの切り替え
```usesSetRedraw``` を ON にすると、オブジェクト設定ダイアログの切替時にコントロールの更新処理を最適化します。※eclipse_fast.auf と似たような機能です。

## (5) タイムラインの描画がおかしいと感じる人向け
高 DPI の設定をしているとタイムラインのグラデーション描画が特におかしくなります。
```usesGradientFill``` を ON にすると、OS の API でグラデーションを描画するようになります。※eclipse_fast.auf と似たような機能です。

## 導入方法

以下のファイルを AviUtl の Plugins フォルダに入れてください。
* OptimizeEditBox.auf
* OptimizeEditBox.ini

## 設定方法

OptimizeEditBox.ini をテキストエディタで編集してから AviUtl を起動します。

```ini
[Settings]
optimizeTimeLine=1
; 0 以外を指定すると、タイムラインでテキストオブジェクトを左クリックして選択したときの動作を最適化します。
editBoxDelay=0
; エディットボックスの再計算遅延時間をミリ秒で指定します。
; 例えば 1 秒遅延させたい場合は 1000 を指定します。
; 遅延させたくない場合は 0 を指定します。
usesUnicodeInput=0 ; テキストオブジェクトで UNICODE 文字を入力したい場合は 1 を指定します。
usesCtrlA=0 ; エディットボックスで Ctrl+A を有効にしたい場合は 1 を指定します。ただし、usesUnicodeInput が 1 のときのみ有効になります。
usesSetRedraw=0 ; オブジェクト設定ダイアログの最適化を行う場合は 1 を指定します。
usesGradientFill=0 ; グラデーション描画を変更する場合は 1 を指定します。
innerColorR=0xff ; 内側の枠の色。
innerColorG=0xff ; 内側の枠の色。
innerColorB=0xff ; 内側の枠の色。
innerEdgeWidth=1 ; 内側の枠の横幅。0以下なら枠の左右は描画しない。
innerEdgeHeight=1 ; 内側の枠の縦幅。0以下なら枠の上下は描画しない。
outerColorR=0x00 ; 外側の枠の色。指定の仕方は内側の枠と同じ。
outerColorG=0x00
outerColorB=0x00
outerEdgeWidth=1
outerEdgeHeight=1
selectionColor=-1 ; 選択領域の色。色は 0x00bbggrr の形式で指定する。-1 の場合は指定なし。
selectionEdgeColor=-1 ; 選択領域端の色。
selectionBkColor=-1 ; 選択領域外の色。
layerBorderLeftColor=-1 ; レイヤー間ボーダーの左側の色。
layerBorderRightColor=-1 ; レイヤー間ボーダーの右側の色。
layerBorderTopColor=-1 ; レイヤー間ボーダーの上側の色。
layerBorderBottomColor=-1 ; レイヤー間ボーダーの下側の色。
addTextEditBoxHeight=0 ; テキストオブジェクトのエディットボックスの高さに加算する値を指定します。例えば、200 を指定するとエディットボックスの高さが通常より 200 ピクセル高くなります。
addScriptEditBoxHeight=0 ; スクリプト制御のエディットボックスの高さに加算する値を指定します。
fontName=Segoe UI ; エディットボックスで使用するフォントのフォント名を指定します。
fontSize=14 ; フォントのサイズを指定します。
fontPitch=1 ; 固定幅を指定する場合は 1 を指定します。
```

## 動作確認

* (必須) AviUtl 1.10 & 拡張編集 0.92 http://spring-fragrance.mints.ne.jp/aviutl/
* (共存確認) patch.aul r14 https://www.nicovideo.jp/watch/sm40087155
* (一部競合) eclipse_fast 1.00 https://www.nicovideo.jp/watch/sm39756003
