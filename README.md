# crane_x7_samples
研究・教材用アームロボットのCRANE-X7のサンプルプログラム集

## crane_x7用 teaching play back
crane_x7のteaching play backのサンプルを実行
### 参考資料
 * [ROBOTIS-GIT/Dynamixel SDK](https://github.com/ROBOTIS-GIT/DynamixelSDK)
 * [Dynamixel XM430-W350 マニュアル](http://www.besttechnology.co.jp/modules/knowledge/?Dynamixel%20XM430-W350)

### 動作環境
 * OS : Linux ubuntu16.04 64bit
 * コンパイラ : gcc version 5.4.0
 
### コンパイル手順 
   ```
   $ git clone https://github.com/rt-net/crane_x7_samples.git 
   $ cd crane_x7_samples  
   ```
   本プログラムは[Dynamixel SDK](https://github.com/ROBOTIS-GIT/DynamixelSDK)を使用している  
   * SDKのclone,makeを行う  
   ``` 
   $ git clone https://github.com/ROBOTIS-GIT/DynamixelSDK.git 
   $ cd DynamixelSDK/c++/build/linux64
   $ make
   $ cd ../../../../
   ```
   make
   * buildフォルダ内にある[Makefile](./teaching_play_back/build/Makefile)を使用する
   ```
   $ cd teaching_play_back  
   $ cd build 
   $ make
   ```

### 使用方法
   * デバイスが認識しているか確認をする
   ```
   $ ls /dev/ttyUSB*  
   ttyUSB0
   ```
   `ttyUSB0`と認識されていれることがわかる。また、[プログラム](./teaching_play_back/example_CRANE-X7.cpp)内のCOMポート(DEVICENAME)を同名で設定する。
   * 接続されたデバイスに書き込み読み込みの権限を渡す
   ```
   $ sudo chmod a+rw /dev/ttyUSB0 
   ```
   プログラムの実行
   ```
   $ ./example_CRANE-X7 
   ```
   COM port, baudrateの設定が完了すると動作できるようになる
   キーボード入力を行うと、それぞれ対応する動きを行う

   | キー | 名称 | 概要 |
   |:---:|:---:|:---|
   |q| exit | プログラム終了 |
   |o| SERVO ON | トルクON |
   |i| SERVO OFF | トルクOFF |
   |p| MOVE POSITION | 指定の角度に移動 |
   |s| Offset Position | 初期姿勢に移動 |
   |t| Teaching | play back用の角度を保存 |
   |l| Play back data | 保存した角度を再生 |
   
   #### MOVE POSITION   
   * [プログラム](./teaching_play_back/example_CRANE-X7.cpp)内のgoal_positionに設定された角度(deg)に移動(初期設定は初期姿勢になっている)
    
   #### Offset Position  
   * 最大動作角の中心を初期姿勢として、全サーボを初期姿勢にする
    
   #### Teaching   
   * 全サーボがトルクOFFの状態になり、角度をテキストデータに保存   
   * `q`を押すと終了する   
   * それ以外のキーが押された時の角度をテキストに保存する　
   * もう一度、teachingを行うまで、テキストデータは書き換わりません
    
   #### play back data(point to pointの再生)   
   * 全サーボがトルクONになり、初期姿勢に移動する   
   * テキストのデータを上から順番に再生を行う   
   * 再生の時間を取るためにsleepをプログラムに追加   
   * 全部のデータを再生終了すると、初期姿勢に戻る
    
## License

This repository is licensed under the Apache License Version 2.0, see [LICENSE](./LICENSE).

このリポジトリはApache License Version 2.0で公開されています。詳細は[LICENSE](./LICENSE)を確認してください。
