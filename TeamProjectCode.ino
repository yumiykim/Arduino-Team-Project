#include <LiquidCrystal.h> // LCD 디스플레이 제어를 위한 라이브러리 포함

// 핀 번호 정의 (초음파 센서 및 LED)
#define TRIG_PIN 9        // 초음파 센서의 Trig 핀을 아두이노 디지털 핀 9에 연결
#define ECHO_PIN 10       // 초음파 센서의 Echo 핀을 아두이노 디지털 핀 10에 연결
#define LED_PIN 8         // 조명을 제어할 LED를 아두이노 디지털 핀 8에 연결

// LCD 디스플레이 핀 설정 (RS, Enable, D4, D5, D6, D7)
LiquidCrystal lcd(7, 6, 5, 4, 3, 2); // LCD 객체 생성 (핀 할당)

// 조명 상태 및 타이머 관련 변수
bool isLightOn = true;                // 조명이 켜져 있는지 추적하는 변수, 초기 상태는 '켜짐'
long lastDetectedTime;                // 마지막으로 사람이 감지된 시간을 저장하는 변수
const long noMotionDuration = 30000;  // 사람이 감지되지 않은 상태에서 대기 시간 (30초)
const int numReadings = 5;            // 초음파 센서로 거리 측정 시 평균값을 구하기 위해 샘플 개수 설정 (5회 측정)

//
// 초음파 센서를 이용한 거리 측정 함수 정의
//
long getDistance() {
  long totalDistance = 0; // 거리 합산을 위한 변수 초기화

  // 거리 측정을 numReadings(5회) 반복하여 평균 계산
  for (int i = 0; i < numReadings; i++) {
    digitalWrite(TRIG_PIN, LOW);       // Trig 핀을 LOW로 설정해 센서 초기화
    delayMicroseconds(2);              // 안정화를 위해 2마이크로초 대기
    digitalWrite(TRIG_PIN, HIGH);      // Trig 핀을 HIGH로 설정해 초음파 신호 발사 시작
    delayMicroseconds(10);             // 10마이크로초 동안 신호 발사 유지
    digitalWrite(TRIG_PIN, LOW);       // Trig 핀을 다시 LOW로 설정해 신호 발사 종료

    // Echo 핀에서 초음파 신호가 반사되어 돌아오는 시간을 측정 (단위: 마이크로초)
    long duration = pulseIn(ECHO_PIN, HIGH, 20000); // 20ms (20000µs) 타임아웃 설정
    if (duration == 0) continue; // 반사된 신호가 없을 경우, 현재 측정값 무시하고 다음 반복으로 진행

    // 거리 계산: (반사된 시간 × 0.034) / 2 = 거리(cm)
    // 초음파 속도는 약 0.034cm/µs이며, 왕복 시간이므로 2로 나눔
    totalDistance += (duration * 0.034) / 2;
    delay(50); // 각 측정 간 50ms 대기 (노이즈 제거를 위해)
  }

  // 5회 측정한 거리의 평균값을 반환
  return totalDistance / numReadings;
}

//
// 초기 설정 및 시스템 시작 메시지 출력
//
void setup() {
  lcd.begin(16, 2); // 16x2 LCD 디스플레이 초기화 (16열 x 2행)
  lcd.print("System Starting..."); // 시스템 시작 시 LCD에 메시지 출력
  delay(2000); // 2초 대기 (사용자가 메시지를 읽을 수 있도록 대기 시간 제공)
  lcd.clear(); // LCD 화면 초기화 (이전 메시지 삭제)

  // 핀 모드 설정
  pinMode(TRIG_PIN, OUTPUT); // Trig 핀을 출력 모드로 설정
  pinMode(ECHO_PIN, INPUT);  // Echo 핀을 입력 모드로 설정 (초음파 수신)
  pinMode(LED_PIN, OUTPUT);  // LED 핀을 출력 모드로 설정 (조명 제어)

  // 시스템 시작 시 조명을 켠 상태로 초기화
  digitalWrite(LED_PIN, HIGH); // LED를 켬 (조명 켜짐 상태)
  isLightOn = true;            // 조명 상태를 '켜짐'으로 설정
  lastDetectedTime = millis(); // 시스템 시작 시간을 기록하여 타이머 초기화
  lcd.print("Initial State:"); // 초기 상태를 LCD에 출력
  lcd.setCursor(0, 1);         // LCD 두 번째 줄로 이동
  lcd.print("Light is ON");    // 조명이 켜져 있음을 알림
}

//
// 메인 루프 (사람 감지 및 조명 제어)
//
void loop() {
  long currentTime = millis(); // 현재 시간을 밀리초 단위로 가져옴 (시스템 시작 이후 경과 시간)
  long distance = getDistance(); // 초음파 센서를 사용해 거리 측정

  // 초음파 센서에서 유효하지 않은 거리 값이 감지되면 함수 종료 (측정값 무시)
  if (distance == 0) return;

  // 사람이 감지되었을 때 (거리가 200cm 이하인 경우)
  if (distance < 200) {
    lastDetectedTime = currentTime; // 사람이 감지되면 타이머를 현재 시간으로 리셋
    lcd.setCursor(0, 0);            // LCD 첫 번째 줄의 시작 위치로 커서 이동
    lcd.print("Person detected");   // 사람이 감지되었음을 LCD에 출력
    lcd.setCursor(0, 1);            // LCD 두 번째 줄로 커서 이동
    lcd.print("Keeping light ON");  // 조명을 켜진 상태로 유지하는 메시지 출력
    delay(1000); // 사용자가 메시지를 읽을 수 있도록 1초 대기
  }

  // 사람이 감지되지 않은 상태에서 설정된 대기 시간을 초과한 경우
  if (millis() - lastDetectedTime > noMotionDuration) {
    if (isLightOn) { // 조명이 켜져 있을 경우에만 동작
      digitalWrite(LED_PIN, LOW); // 조명을 끔 (LED OFF)
      isLightOn = false;          // 조명 상태를 '꺼짐'으로 변경
      lcd.clear();                // LCD 화면 초기화
      lcd.setCursor(0, 0);        // LCD 첫 번째 줄의 시작 위치로 커서 이동
      lcd.print("No person found"); // 사람이 감지되지 않았음을 출력
      lcd.setCursor(0, 1);        // LCD 두 번째 줄로 이동
      lcd.print("Turning off light"); // 조명이 꺼졌음을 알림
      delay(1000); // 사용자가 메시지를 읽을 수 있도록 1초 대기
    }
  }

  delay(500); // 0.5초마다 시스템 상태를 다시 확인 (반복 주기)
}
