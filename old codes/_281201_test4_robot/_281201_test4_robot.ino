#include<SoftwareSerial.h>
SoftwareSerial JOY_receive(8, 9);

int A = 0;
int timer = 0;
int MotorR = 0;
int mode = 0;
int ModeChangeFlag = 0;
byte RMotor1 = 0, RMotor2 = 0, LMotor1 = 0, LMotor2 = 0, RS = 1, LS = 1;

void setup() {
  JOY_receive.begin(9600);
  Serial.begin(115200);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
}

void loop() {


  if (A == 0) {
    JOY_receive.write('S');
    A++;
  }

  while (JOY_receive.available() <= 6) {
    delay(1);
    timer++;
    if (timer > 5000) {
      Mode_Remote();
    }
  }


  if (JOY_receive.available() > 6) {
    //Serial.println('C');
    if (JOY_receive.read() == 'H') {
      RMotor1 = JOY_receive.read();
      RMotor2 = JOY_receive.read();
      LMotor1 = JOY_receive.read();
      LMotor2 = JOY_receive.read();
      RS = JOY_receive.read();
      LS = JOY_receive.read();

      if (RS == 0 && LS == 0 && mode == 0) {
        mode = 1;
        delay(1000);
      }
      else if (RS == 0 && LS == 0 && mode == 1) {
        mode = 2;
        delay(1000);
      }
      else if (RS == 0 && LS == 0 && mode == 2) {
        mode = 0;
        delay(1000);
      }


      if (mode == 0) {
        Mode_Inactive();
      }
      else if (mode == 1) {
        Mode_Remote();
      }
      else if (mode == 2) {
        Mode_Auto();
      }

      JOY_receive.write(mode);  //現在の動作モードをコントローラにフィードバックする
      JOY_receive.write('S');  //一通り処理が終わったので、次の指令値をリモコン側にリクエスト
    }
  }
  else {
    Serial.println("wait");
  }
}
