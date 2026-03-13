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

// NOVÁ PROMĚNNÁ PRO REŽIM: 0 = Automatika (senzory), 1 = Manuál (tlačítko S2)
int rezimZavlazovani = 0; 

byte predchozi = 0;
char posledniBluetooth = '-';

void setup() {
  pinMode(pinLedCerpadlo, OUTPUT);
  digitalWrite(pinLedCerpadlo, 0);

  pinMode(pinRelais1, OUTPUT);
  digitalWrite(pinRelais1, 0);

  pinMode(pinRelais2, OUTPUT);
  digitalWrite(pinRelais2, 0);

  servoMotor.attach(pinServo);
  servoMotor.writeMicroseconds(1500); // STOP při startu

  for (int i = 0; i < 4; i = i + 1) {
    matice.shutdown(i, 0);
    matice.setIntensity(i, 8);
    matice.clearDisplay(i);
  }

  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("Arduino start");
}

void loop() {
  // ================= TM1638 =================
  byte tlacitka = panel.getButtons();

  // S1 - ovládání LED matice
  if ((tlacitka & 0x01) && !(predchozi & 0x01)) {
    stavOsvetleni = 1 - stavOsvetleni;
    if (stavOsvetleni == 1) {
      for (int i = 0; i < 4; i = i + 1)
        for (int j = 0; j < 8; j = j + 1)
          matice.setRow(i, j, 0xFF);
      digitalWrite(pinRelais1, 1);
    } else {
      for (int i = 0; i < 4; i = i + 1)
        matice.clearDisplay(i);
      digitalWrite(pinRelais1, 0);
    }
  }

  // S2 - manuální ovládání čerpadla (funguje jen v manuálním režimu, nebo ho tím zapneme)
  if ((tlacitka & 0x02) && !(predchozi & 0x02)) {
    Serial.println("Tlacitko S2 zmacknuto - MANUÁLNÍ OVLÁDÁNÍ");
    stavZavlazovani = 1 - stavZavlazovani;
    rezimZavlazovani = 1; // Pojistka: když zkusíš ovládat manuálně, automaticky se přepneš do manuálního režimu
    Serial.print("Stav zavlazovani: ");
    Serial.println(stavZavlazovani);
  }

  // S3 - servo
  if ((tlacitka & 0x04) && !(predchozi & 0x04)) {
    ovladaniServa();
  }

  // S4 - PŘEPÍNÁNÍ REŽIMU (Auto / Manuál)
  if ((tlacitka & 0x08) && !(predchozi & 0x08)) {
    rezimZavlazovani = 1 - rezimZavlazovani;
    Serial.print("Rezim zmenen na: ");
    Serial.println(rezimZavlazovani == 0 ? "AUTOMATIKA (Senzory)" : "MANUAL");
  }

  predchozi = tlacitka;

  // ================= SENSORY & LOGIKA ČERPADLA =================
  int hodnotaVlhkost = analogRead(A0);
  int hodnotaSvetlo  = analogRead(A1);
  int stavCerpadla = 0;

  if (rezimZavlazovani == 0) {
    // --- AUTOMATICKÝ REŽIM (Senzory mají volant) ---
    if (hodnotaVlhkost > 400 || hodnotaSvetlo > 300) {
      stavCerpadla = 1;
    } else {
      stavCerpadla = 0;
    }
    // Synchronizujeme i proměnnou tlačítka, aby při přepnutí na manuál čerpadlo nebliklo zpět
    stavZavlazovani = stavCerpadla; 
  } else {
    // --- MANUÁLNÍ REŽIM (Senzory jsou ignorovány) ---
    stavCerpadla = stavZavlazovani; 
  }

  // Finální propis stavu na piny (dělá se jen jednou na konci logiky)
  digitalWrite(pinLedCerpadlo, stavCerpadla);
  digitalWrite(pinRelais2, stavCerpadla);

  // ================= BLUETOOTH =================
  if (Serial1.available() > 0) {
    char znak = Serial1.read();
    posledniBluetooth = znak;

    if (znak == '1') {
      stavOsvetleni = 1;
      for (int i = 0; i < 4; i = i + 1)
        for (int j = 0; j < 8; j = j + 1)
          matice.setRow(i, j, 0xFF);
      digitalWrite(pinRelais1, 1);
    }

    if (znak == '2') {
      stavOsvetleni = 0;
      for (int i = 0; i < 4; i = i + 1)
        matice.clearDisplay(i);
      digitalWrite(pinRelais1, 0);
    }

    // Bluetooth příkazy 3 a 4 natvrdo přepnou do manuálního režimu
    if (znak == '3') {
      rezimZavlazovani = 1; // Přepnutí do manuálu
      stavZavlazovani = 1;
    }

    if (znak == '4') {
      rezimZavlazovani = 1; // Přepnutí do manuálu
      stavZavlazovani = 0;
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
  Serial.print("Rezim: ");
  Serial.print(rezimZavlazovani == 0 ? "AUTO" : "MANUAL");
  Serial.print(" | Vlhkost: ");
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
  Serial.print(" | BT: ");
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
