# kourobo2023
交流ロボコン2023に出場予定の「キサラズハコビガニ」のプログラムです。
App/Src/app.hを基本的に編集しています。

# 仕様

コントローラーはDUALSHOCK3を使用 

## 使用MD数
5枚
| MD番号 | つなぐモーター |
|------------|------|
| 0   | キャタピラ右 |
| 1   | キャラピラ左 |
| 2       | アーム(速いほう) |
| 3       | アーム(遅いほう) |
| 4  | トレー上下機構 |


## 関数
### suspensionSystem

引き継がれてきた2輪プログラムをキャタピラに使用

### armSystem

2つのモーターで2つのアームの開閉を制御

### upDownSystem

カム機構を用いて、トレーを上下させる動作を制御
## 操作
| コントローラーの操作 | ロボットの動作 |
|------------|---------|
| 右スティック上下   | キャタピラ右  |
| 左スティック上下   | キャラピラ左  |
| ○ボタン       | 両アーム開   |
| ×ボタン       | 両アーム閉    |
| △ボタン(単押し)  | 両アーム開(自動) |
| □ボタン(単押し)  | 両アーム閉(自動) |
| L1         | 前アーム開   |
| L2         | 前アーム閉   |
| R1         | 後アーム開   |
| R2         | 後アーム閉   |　
| 十字キー上      | トレー上    |
| 十字キー下      | トレー下    |
| 十字キー右(単押し)      | トレー上(自動) |
| 十字キー左(単押し)      | トレー下(自動) |


特に記載がないものは、押してる間だけ操作
