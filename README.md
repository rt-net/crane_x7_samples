# crane_x7
研究・教材用アームロボットのCRANE-X7のサンプルプログラム集

## crane_x7用 teaching play back
crane_x7のteaching play backのサンプルを実行
### 動作環境
 * OS : Linux 16.04 64bit
 * コンパイラ : gcc version 5.4.0
 
### コンパイル手順 
   ```
   $ git clone https://github.com/rt-net/crane_x7.git 
   $ cd crane_x7  
   ```
   本プログラムは[Dynamixel　SDK](https://github.com/ROBOTIS-GIT/DynamixelSDK)を使用している  
   * SDKのcloneを行う  
   ``` 
   $ git clone https://github.com/ROBOTIS-GIT/DynamixelSDK.git `  
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
