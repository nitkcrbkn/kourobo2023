# kourobo2023
交流ロボコン2023に出場予定の「キサラズハコビガニ」のプログラムです。

# 仕様

## 使用MD数
現在実装済5枚

## 関数
### suspensionSystem

引き継がれてきた2輪プログラムを、キャタピラ用に調整

### armSystem

2つのモーターで2つのアームの開閉を制御

### upDownSystem

カム機構を用いて、トレーを上下させる動作

| コントローラーの操作 | ロボットの動作  |
|------------|----------|
| 右スティック上下   | キャタピラ右   |
| 左スティック上下   | キャラピラ左   |
| ○ボタン       | アーム開(手動) |
| ×ボタン       | アーム閉(手動) |
| 十字キー上      | トレー上昇    |
| 十字キー下      | トレー下降    |