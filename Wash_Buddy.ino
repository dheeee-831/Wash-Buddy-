//----------Wash Buddy-----------//


// 라이브러리
#include <Wire.h>
#include <Servo.h> // 서보모터 라이브러리 추가
#include <LiquidCrystal_I2C.h> // LCD I2C 라이브러리 추가

// 핀 정의
#define TRIG_PIN 10
#define ECHO_PIN 11
#define RED_PIN 3
#define GREEN_PIN 5
#define BLUE_PIN 9
#define SERVO_PIN_R 6
#define SERVO_PIN_L 7

// 음계별 주파수 정의
#define NOTE_C5  523   //도
#define NOTE_D5  587   //레
#define NOTE_E5  659   //미
#define NOTE_F5  698   //파
#define NOTE_G5  784   //솔
#define NOTE_A5  880   //라
#define NOTE_B5  988   //시
#define NOTE_C6  1047  //도

// 곰 세마리 맬로디
int melody[] = {
    NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5, 
    NOTE_E5, NOTE_G5, NOTE_G5, NOTE_E5, NOTE_C5,
    NOTE_G5, NOTE_G5, NOTE_E5, NOTE_G5, NOTE_G5, NOTE_E5,
    NOTE_C5, NOTE_C5, NOTE_C5,

    NOTE_G5, NOTE_G5, NOTE_E5, NOTE_C5,
    NOTE_G5, NOTE_G5, NOTE_G5,
    NOTE_G5, NOTE_G5, NOTE_E5, NOTE_C5,
    NOTE_G5, NOTE_G5, NOTE_G5,

    NOTE_G5, NOTE_G5, NOTE_E5, NOTE_C5,
    NOTE_G5, NOTE_G5, NOTE_G5, NOTE_A5, NOTE_G5,
    NOTE_C6, NOTE_G5, NOTE_C6, NOTE_G5,
    NOTE_E5, NOTE_D5, NOTE_C5
};

// 곰 세마리 음 길이
int noteDurations[]={
4,8,8,4,
4,8,8,4,4,
8,8,4,8,8,4,
4,4,2,
4,4,4,4,
4,4,2,
4,4,4,4,
4,4,2,
4,4,4,4,
8,8,8,8,2,
4,4,4,4,
4,4,2
};

// LCD라는 이름의 LCD I2C 객체 생성
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C 주소, 열과 행의 개수 입력

int tonepin = 13; //부저 핀

Servo servoMotor;       // 손 서보 모터
Servo servoMotorLeft;   // 펌프 누르는 서보 모터

// 변수 정의
int melodyIndex = 0; // 멜로디 인덱스 초기화
bool objectDetected = false; // 물체 감지 상태

long duration; // 초음파 신호 시간
int distance; // 초음파 측정 거리 값
int previousDistance = 0; // 초음파 이전 측정 거리 값 


// RGB LED 색상 설정 함수
void setColor(int red, int green, int blue) {
  analogWrite(RED_PIN, 255 - red);   // 빨간색 값 설정
  analogWrite(GREEN_PIN, 255 - green); // 초록색 값 설정
  analogWrite(BLUE_PIN, 255 - blue);  // 파란색 값 설정
}


// LCD 카운트다운 및 RGB LED 색상 변경 함수
void countdownAndSetColor(int start, int end, int red, int green, int blue) 
{
  int i = start; // 카운트다운 초기값

  while (i >= end) 
  {
    lcd.setCursor(0, 1); // LCD의 2행 1열로 커서 이동

    // 초음파 거리 측정
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH);
    distance = duration * 0.034 / 2;

    // 직전 거리 값과 현재 거리 값의 차이가 클 경우 LCD 카운트다운 진행
    if (abs(distance - previousDistance) >= 5) 
    {
      lcd.print("Time: ");
      if (i < 10) lcd.print("0"); // 한 자리 숫자인 경우 앞에 0 추가
      lcd.print(i);
      lcd.print(" sec");
      i--; // 카운트다운
    }
    // 직전 거리 값과 현재 거리 값의 차이가 적을 경우 LCD 카운트다운 Stop
    else 
    {
      lcd.print("No ");
      lcd.print("  "); // 기존 텍스트 지우기
    }

    previousDistance = distance; // 현재 값을 이전 값으로 저장
    setColor(red, green, blue); // RGB LED 색상 설정

    // 멜로디 재생
    if (melodyIndex < sizeof(melody) / sizeof(melody[0])) 
    {
      tone(tonepin, melody[melodyIndex], 500); // 각 음 재생 (0.5초)
      delay(500); // 음 길이만큼 대기
      melodyIndex++; // 다음 음으로 진행
    }
    // 노래 반복 재생을 위한 멜로디 인덱스 초기화 
    else 
    { 
      melodyIndex = 0; // 처음부터 다시 반복
    }
  }

  noTone(tonepin); // 카운트다운 종료 후 맬로디 중지
  melodyIndex = 0; // 멜로디 인덱스 초기화
   
}

void setup() 
{
  // LCD 초기화
  lcd.init(); // lcd 초기화
  lcd.backlight(); // lcd 백라이트 on

  // 초음파 센서 설정
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // LED 핀 설정
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);

  // 서보 모터 초기화
  servoMotor.attach(SERVO_PIN_R); //'좋아요'표시 손 서보모터
  servoMotorLeft.attach(SERVO_PIN_L); //펌프 누르는 서보모터
  servoMotor.write(90);
  servoMotorLeft.write(90);

  Serial.begin(9600);
}

void loop() 
{
  // LCD 화면 초기화 및 시작메세지 출력
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scanning...");
  
  if (!objectDetected) 
  {
    // 초음파 거리 측정
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH);
    distance = duration * 0.034 / 2;

    // 측정된 거리를 시리얼 모니터에 출력 
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // 15cm이하 감지 시 펌프 작동
    if (distance > 0 && distance <= 15) {
      Serial.println("Object Detected!");
      objectDetected = true; // 물체감지 
      setColor(255, 0, 0); // Led를 빨간색으로 설정

      // 펌프작동-down
      servoMotorLeft.write(0);  // 반시계 방향 최대 속도
      delay(500);               // 0.5초 동안 회전 (펌프 누르기)
      servoMotorLeft.write(90); // 정지
      delay(1000);              // 1초 대기 (누른 상태 유지)

      // 펌프작동-up
      servoMotorLeft.write(180); // 시계 방향 최대 속도
      delay(500);                // 0.5초 동안 회전 (펌프 복귀)
      servoMotorLeft.write(90);  // 정지
      delay(2000);               // 2초 대기 (다음 동작 전 안정화)
    
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Wash your hands!");

      // 카운트다운 시작
      countdownAndSetColor(30, 21, 255, 0, 0);   // 빨간색
      countdownAndSetColor(20, 11, 255, 255, 0); // 노링색
      countdownAndSetColor(10, 1, 0, 255, 0);    // 초록색

      setColor(0, 0, 255); // 최종적으로 파랑색으로 설정

      servoMotor.write(135); //기기 손 하트 --> '좋아요'
      delay(500); ///0.5초동안 회전
      servoMotor.write(90); //정지

      lcd.clear();
      lcd.setCursor(4, 0);
      lcd.print("Good Job!");

      delay(5000); //5초 대기('좋아요' 유지)

      servoMotor.write(45); //기기 손 '좋아요' --> 하트
      delay(500); //0.5초 동안 회전
      servoMotor.write(90); //정지
      delay(1000);
      
      objectDetected = false; // 물체감지 상태 초기화

    }
  }

  delay(1000); // 루프 대기
}