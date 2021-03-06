//refered http://www.lucidarme.me/?p=5057
//http://garchiving.com/gyro-drift-correction/ and peripherals

#include<SoftwareSerial.h>
SoftwareSerial JOY_receive(14, 15);
#include<TimerOne.h>
#include<Wire.h>

int A = 0;
int B = 0;
int timer = 0;
int mode = 0; //ロボットの動作モードを表す。0：休止、1：遠隔操縦、2：自律走行
byte RMotor1 = 0, RMotor2 = 0, LMotor1 = 0, LMotor2 = 0, RS = 1, LS = 1;
byte discard;
unsigned long t;

volatile float data[6] = {0, 0, 0, 0, 0, 0}; //加速度3軸＋角速度3軸
volatile float Calib[6] = {0, 0, 0, 0, 0, 0}; //同上。キャリブレーション用。
volatile float ACCroll = 0, ACCpitch = 0, ACCyaw = 0; //加速度から計算した姿勢。
volatile float roll = 0, pitch = 0, yaw = 0; //姿勢角
float k = 0.05; //姿勢角計算時の、加速度から姿勢角を計算する成分の重み係数。重力以外の加速度が大きくかかっているときは加速度から算出される姿勢角の信用度を落とす。最大0.05（5%）
volatile int flag = 0;


void setup() {
  //シリアル通信開始。
  JOY_receive.begin(9600);
  Serial.begin(115200);
  Wire.begin();

  //IMUの初期化、キャリブレーション
  MPU9250_Initialization();
  MPU9250_Calibration();

  //タイマー割込み設定
  Timer1.initialize(46000);
  Timer1.attachInterrupt(FLAG);

  //ピンモードの設定
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(13, OUTPUT);
}

void loop() {

  if (A == 0) {
    JOY_receive.write('S');
    A++;
  }

  if (JOY_receive.available() > 6) {  //受信バッファにデータが6バイトより多くある時（ヘッダバイト＋ジョイスティック信号６バイト
    if (JOY_receive.read() == 'H') {
      RMotor1 = JOY_receive.read();
      RMotor2 = JOY_receive.read();
      LMotor1 = JOY_receive.read();
      LMotor2 = JOY_receive.read();
      RS = JOY_receive.read();
      LS = JOY_receive.read();

      ChangeMode(); //モード変更指令が入った場合に動作モードを変更する関数

      if (mode == 0) {
        //Serial.println("mode : stop");
      }
      else if (mode == 1) {
        //Serial.print("mode : remote  ");
        //mode_R(RMotor1, RMotor2, LMotor1, LMotor2); //指令値に基づきモータ駆動する関数
      }
      else if (mode == 2) {
        //Serial.println("mode : auto");
      }
    }
    else {  //受信バッファにデータは6バイトより多くあるものの、先頭がヘッダバイトではないとき（通信に欠落があったとき
      while (JOY_receive.available() > 0) {
        mode = 0;
        discard = JOY_receive.read(); //受信バッファが空になるまでデータを読み出して、リセット。
      }
    }
    JOY_receive.write(mode);//現在の動作モードをコントローラにフィードバックする
    JOY_receive.write('S');//一通り処理が終わったので、次の指令値をリモコン側にリクエスト
  }

  if(flag = 1){
    MPU9250_PostureAngle();
    flag = 0;
  }
  
  Serial.print(pitch);
  Serial.print(",");
  Serial.print(roll);
  Serial.print(",");
  Serial.println(yaw);
}


