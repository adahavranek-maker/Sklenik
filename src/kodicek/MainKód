#include <TM1638.h>
#include <LedControl.h>
#include <Servo.h>

// -------------------- TM1638 --------------------
TM1638 panel(2, 3, 4);

// -------------------- LED MATICE ----------------
LedControl matice = LedControl(11, 13, 10, 4);

// -------------------- PINY ----------------------
int pinLedCerpadlo = 6;
int pinServo = 12;

// 2kanálový relé modul
int pinRelais1 = 7; // relé pro LED matici
int pinRelais2 = 8; // relé pro čerpadlo

// -------------------- SERVO ---------------------
Servo servoMotor;
int stavServa = 0;   // 0 = zavřeno, 1 = otevřeno

int stavOsvetleni = 0;
int stavZavlazovani = 0;

byte predchozi = 0;
char posledniBluetooth = '-';

void setup() {
  pinMode(pinLedCerpadlo, OUTPUT);
  pinMode(pinRelais1, OUTPUT);
  pinMode(pinRelais2, OUTPUT);

  digitalWrite(pinRelais1, LOW);
  digitalWrite(pinRelais2, LOW);

  servoMotor.attach(pinServo);
  servoMotor.writeMicroseconds(1500); // STOP při startu

  for (int i = 0; i < 4; i = i + 1) {
    matice.shutdown(i, 0);
    matice.setIntensity(i, 8);
    matice.clearDisplay(i);
  }

  // PC
  Serial.begin(9600);

  // BLUETOOTH (HC-06)
  Serial1.begin(9600);

  Serial.println("Arduino start");
}

void loop() {

  // ================= TM1638 =================
  byte tlacitka = panel.getButtons();

  if ((tlacitka & 0x01) && !(predchozi & 0x01)) {
    stavOsvetleni = 1 - stavOsvetleni;

    if (stavOsvetleni == 1) {
      for (int i = 0; i < 4; i = i + 1)
        for (int j = 0; j < 8; j = j + 1)
          matice.setRow(i, j, 0xFF);
      digitalWrite(pinRelais1, HIGH);
    } else {
      for (int i = 0; i < 4; i = i + 1)
        matice.clearDisplay(i);
      digitalWrite(pinRelais1, LOW);
    }
  }

  if ((tlacitka & 0x02) && !(predchozi & 0x02)) {
    stavZavlazovani = 1 - stavZavlazovani;
    digitalWrite(pinLedCerpadlo, stavZavlazovani);
    digitalWrite(pinRelais2, stavZavlazovani ? HIGH : LOW);
  }

  if ((tlacitka & 0x04) && !(predchozi & 0x04)) {
    ovladaniServa();
  }

  predchozi = tlacitka;

  // ================= SENSORY =================
  int hodnotaVlhkost = analogRead(A0);
  int hodnotaSvetlo  = analogRead(A1);

  if (hodnotaVlhkost < 400 || hodnotaSvetlo < 300) {
    digitalWrite(pinLedCerpadlo, 1);
  } else {
    digitalWrite(pinLedCerpadlo, stavZavlazovani);
  }

  // ================= BLUETOOTH =================
  if (Serial1.available() > 0) {
    char znak = Serial1.read();
    posledniBluetooth = znak;

    if (znak == '1') {
      stavOsvetleni = 1;
      for (int i = 0; i < 4; i = i + 1)
        for (int j = 0; j < 8; j = j + 1)
          matice.setRow(i, j, 0xFF);
      digitalWrite(pinRelais1, HIGH);
    }

    if (znak == '2') {
      stavOsvetleni = 0;
      for (int i = 0; i < 4; i = i + 1)
        matice.clearDisplay(i);
      digitalWrite(pinRelais1, LOW);
    }

    if (znak == '3') {
      stavZavlazovani = 1;
      digitalWrite(pinLedCerpadlo, 1);
      digitalWrite(pinRelais2, HIGH);
    }

    if (znak == '4') {
      stavZavlazovani = 0;
      digitalWrite(pinLedCerpadlo, 0);
      digitalWrite(pinRelais2, LOW);
    }

    if (znak == '5') {
      servoMotor.writeMicroseconds(2000);
      delay(600);
      servoMotor.writeMicroseconds(1500);
      stavServa = 1;
    }

    if (znak == '6') {
      servoMotor.writeMicroseconds(1250);
      delay(600);
      servoMotor.writeMicroseconds(1500);
      stavServa = 0;
    }
  }

  // ================= VÝPIS =================
  Serial.print("Vlhkost: ");
  Serial.print(hodnotaVlhkost);
  Serial.print(" | Svetlo: ");
  Serial.print(hodnotaSvetlo);
  Serial.print(" | LED: ");
  Serial.print(digitalRead(pinLedCerpadlo));
  Serial.print(" | Servo: ");
  Serial.print(stavServa);
  Serial.print(" | Relay1: ");
  Serial.print(digitalRead(pinRelais1));
  Serial.print(" | Relay2: ");
  Serial.print(digitalRead(pinRelais2));
  Serial.print(" | Bluetooth prikaz: ");
  Serial.println(posledniBluetooth);

  delay(50);
}

// ================= SERVO =================
void ovladaniServa() {
  if (stavServa == 0) {
    servoMotor.writeMicroseconds(2000);
    delay(600);
    servoMotor.writeMicroseconds(1500);
    stavServa = 1;
  } else {
    servoMotor.writeMicroseconds(1250);
    delay(600);
    servoMotor.writeMicroseconds(1500);
    stavServa = 0;
  }
}
