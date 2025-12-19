#include <TM1638.h>
#include <LedControl.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// -------------------- TM1638 --------------------
TM1638 panel(2, 3, 4);

// -------------------- LED MATICE ----------------
LedControl matice = LedControl(11, 13, 10, 4);

// -------------------- BLUETOOTH -----------------
#define BT_RX 5
#define BT_TX 9
SoftwareSerial bluetooth(BT_RX, BT_TX);

// -------------------- PINY ----------------------
int pinLedCerpadlo = 6;
int pinServo = 12;

// -------------------- SERVO ---------------------
Servo servoMotor;
int stavServa = 0;   // 0 = zavřeno, 1 = otevřeno

int stavOsvetleni = 0;
int stavZavlazovani = 0;

byte predchozi = 0;
char posledniBluetooth = '-';  // uloží poslední znak z mobilu

void setup() {
  pinMode(pinLedCerpadlo, OUTPUT);

  servoMotor.attach(pinServo);
  servoMotor.writeMicroseconds(1500); // STOP při startu

  // LED matice
  for (int i = 0; i < 4; i = i + 1) {
    matice.shutdown(i, 0);
    matice.setIntensity(i, 8);
    matice.clearDisplay(i);
  }

  bluetooth.begin(9600);
  bluetooth.println("HC-05 pripraven");

  Serial.begin(9600);
  Serial.println("Arduino start");
}

void loop() {

  // ================= TM1638 =================
  byte tlacitka = panel.getButtons();

  // S1 – LED matice
  if ((tlacitka & 0x01) && !(predchozi & 0x01)) {
    stavOsvetleni = 1 - stavOsvetleni;

    if (stavOsvetleni == 1) {
      for (int i = 0; i < 4; i = i + 1)
        for (int j = 0; j < 8; j = j + 1)
          matice.setRow(i, j, 0xFF);
    } else {
      for (int i = 0; i < 4; i = i + 1)
        matice.clearDisplay(i);
    }
  }

  // S2 – LED čerpadla
  if ((tlacitka & 0x02) && !(predchozi & 0x02)) {
    stavZavlazovani = 1 - stavZavlazovani;
    digitalWrite(pinLedCerpadlo, stavZavlazovani);
  }

  // S3 – SERVO
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
  if (bluetooth.available() > 0) {
    char znak = bluetooth.read();
    posledniBluetooth = znak;  // uložíme hodnotu pro výpis

    // 1 – LED MATICE
    if (znak == '1') {
      stavOsvetleni = 1;
      for (int i = 0; i < 4; i = i + 1)
        for (int j = 0; j < 8; j = j + 1)
          matice.setRow(i, j, 0xFF);
    }

    // 2 – LED CERPADLA
    if (znak == '2') {
      stavZavlazovani = 1 - stavZavlazovani;
      digitalWrite(pinLedCerpadlo, stavZavlazovani);
    }

    // 3 – SERVO
    if (znak == '3') {
      ovladaniServa();
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
  Serial.print(" | Bluetooth prikaz: ");
  Serial.println(posledniBluetooth);

  delay(50);
}

// ================= FUNKCE SERVO =================
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
