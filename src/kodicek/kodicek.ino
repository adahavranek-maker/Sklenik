#include <TM1638.h>
#include <LedControl.h>

// TM1638 ovládací panel
TM1638 panel(4, 3, 2); // strobe, clock, data

// MAX7219 LED matice (32x8)
LedControl matice = LedControl(11, 13, 10, 4); // DIN, CLK, CS, počet MAX7219 modulů

// Piny relé modulů
int pinReleOsvetleni = 7;   // relé pro LED matici
int pinReleCerpadlo = 8;    // relé pro LED simulující čerpadlo

// Piny senzorů
int pinVlhkostPudy = A0;    // půdní vlhkoměr
int pinFotorezistor = A1;   // fotorezistor

// LED simulace
int pinLedCerpadlo = 6;     // LED simulující čerpadlo
int pinLedSvetlo = 5;       // LED indikující tmu

// Stav relé / LED
int stavOsvetleni = LOW;
int stavZavlazovani = LOW;

// Předchozí stav tlačítek TM1638
uint8_t predchoziStavTlacitek = 0;

void setup() {
  pinMode(pinReleOsvetleni, OUTPUT);
  pinMode(pinReleCerpadlo, OUTPUT);
  pinMode(pinLedCerpadlo, OUTPUT);
  pinMode(pinLedSvetlo, OUTPUT);
  pinMode(pinFotorezistor, INPUT);
  pinMode(pinVlhkostPudy, INPUT);

  // Inicializace TM1638
  panel.setupDisplay(HIGH, 7);

  // Inicializace LED matice
  for (int i = 0; i < 4; i++) {
    matice.shutdown(i, LOW);
    matice.setIntensity(i, 8);
    matice.clearDisplay(i);
  }

  // Vypnout vše na začátku
  digitalWrite(pinReleOsvetleni, LOW);
  digitalWrite(pinReleCerpadlo, LOW);
  digitalWrite(pinLedCerpadlo, LOW);
  digitalWrite(pinLedSvetlo, LOW);

  Serial.begin(9600);
}

void loop() {
  uint8_t stavTlacitek = panel.getButtons();

  // Tlačítko S1 → LED matice
  if ((stavTlacitek & 0x01) == 0x01 && (predchoziStavTlacitek & 0x01) == 0x00) {
    if (stavOsvetleni == LOW) {
      stavOsvetleni = HIGH;
      digitalWrite(pinReleOsvetleni, HIGH);
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
          matice.setRow(i, j, 0xFF);
        }
      }
    } else {
      stavOsvetleni = LOW;
      digitalWrite(pinReleOsvetleni, LOW);
      for (int i = 0; i < 4; i++) {
        matice.clearDisplay(i);
      }
    }
  }

  // Tlačítko S2 → LED simulující čerpadlo
  if ((stavTlacitek & 0x02) == 0x02 && (predchoziStavTlacitek & 0x02) == 0x00) {
    if (stavZavlazovani == LOW) {
      stavZavlazovani = HIGH;
      digitalWrite(pinReleCerpadlo, HIGH);
      digitalWrite(pinLedCerpadlo, HIGH);
    } else {
      stavZavlazovani = LOW;
      digitalWrite(pinReleCerpadlo, LOW);
      digitalWrite(pinLedCerpadlo, LOW);
    }
  }

  predchoziStavTlacitek = stavTlacitek;

  // --- Automatické ovládání podle senzorů ---
  int hodnotaVlhkosti = analogRead(pinVlhkostPudy);
  int hodnotaSvetla = analogRead(pinFotorezistor);

  // Automatické čerpadlo při suché půdě
  if (hodnotaVlhkosti < 400) {
    digitalWrite(pinReleCerpadlo, HIGH);
    digitalWrite(pinLedCerpadlo, HIGH);
  } else {
    if (stavZavlazovani == LOW) {
      digitalWrite(pinReleCerpadlo, LOW);
      digitalWrite(pinLedCerpadlo, LOW);
    }
  }

  // LED indikující tmu
  if (hodnotaSvetla < 300) {
    digitalWrite(pinLedSvetlo, HIGH);
  } else {
    digitalWrite(pinLedSvetlo, LOW);
  }

  // --- Serial výpis ---
  Serial.print("Vlhkost: "); Serial.println(hodnotaVlhkosti);
  Serial.print("Svetlo: "); Serial.println(hodnotaSvetla);
  Serial.print("Osvetleni: "); Serial.println(stavOsvetleni);
  Serial.print("Zavlazovani: "); Serial.println(stavZavlazovani);

  delay(200);
}
