```
略語
RC=>リモコン
MD=>モータドライバ
AB=>電磁弁
SV=>サーボ
```

# 制御フロー
入力したデータをマスターで処理し、外部に出力する。
```
[外部入力] => [Master] => [外部出力]
```
入力はRC、リミットスイッチを用いて行う。
出力はMD,AB,SVなどが挙げられるが、その他にもテープLEDにも出力されることがある。
処理の間隔は10msほどでPCへのデバック情報はn回に一度のみ送信される。

# 外部入出力
`Drivers/DevDriver`にて定義される。
実体は`Drivers/DevDriver/Gene`にて定義される。
それぞれ階層内にdocが付属している。詳しくはそちらを参照のこと。

## 外部入力
### RC
`Drivers/DevDriver/RC/`で定義される
リモコンから届くデータはボタンの押下状態(L1R1L2R2,O X △ □,↑↓→←)とアナログスティックの量子化されたディジタル値(LX,LY,RX,RY)それぞれ(0~127)に入る。
例えばリモコンの○が押された時は
```c
__RC_ISPRESSED_CIRCLE(g_rc_data)
```
が真となる
アナログのデータは
```c
DD_RCGetRY(g_rc_data)
```
で取得可能で右のY成分(コンピュータ的座標系)の値が-64〜63ほどまで使用可能となる。

## 外部出力
### MD
モータドライバには**方向と量子化されたディジタル値(0-9999)**で操作される。
モータのデューティは`g_md_h`に格納する。

### AB
電磁弁にはそれぞれの**ビット列で対応するビットにて対応する電磁弁がOn-Offする。**
データは`g_ab_h`に格納する。

### SV
サーボについてはch 0~ch `DD_NUM_OF_SV`まで送信され、それぞれの更新は**`DD_NUM_OF_SV`回に1回**のみ送信される。
データは`g_sv_h`に格納する。

# ファイル構成
- App
応用層でプログラムはここに記述する。
- Drivers
ドライバでユーザーは変更してはいけない。ここにはデータを受信し、Appで処理されたデータを送信する内容が含まれる。
- bin
生成したバイナリデータである。

# 詳細
## ハードウェア側
I2Cでスレーブとマスタを通信
UARTでPCとのメッセージ、リモコンのデータ取得

## ソフトウェア側
10[ms]ごとのリアルタイム処理
- スレーブに対し自動データ送信
- PCへの自動メッセージ生成〜送信
- RCからのデータ処理
- フリーズ検知

全体のマスターとスレーブの間は`I2C`で通信される。マスターからは、一定のデータ列でひとつずつスレーブにデータが送信され、全てのスレーブにデータが送られる。I2Cとは、Inter integrated circuitの略で、近距離通信ができる通信規格。スレーブはアドレスを持ち、そのアドレスに向けて送信要求（受信）、データの送信が可能である。通信フォーマットは別に定める。

PCへのデータ送信は`50[ms]`ほどの間隔で行われる。変わる可能性もある。（変更可能）この間隔で、リモコンのデータや、現在の出力をPCにダンプできる。これはプログラム開発時にスレーブ無しでプログラムをかけるようにするための仕組みである。

RCからの信号は、適切に処理される。最初の8個程度は値が破棄される。以後のデータについて、チェックサム等のエラー時はその旨のエラー内容を出し、動作は続行させる。ボーレートは`9600bps`である。

フリーズ検知として、WatchDogTimerを用いている。一定時間、リアルタイム性が崩れると、異常事態とみなし、リセットをかける。なのでユーザーアプリケーションでは処理フローに乱れをかけるようなことはしてはならない。（重い計算や、waitなど）