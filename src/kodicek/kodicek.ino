#include <TM1638.h>
#include <LedControl.h>

// TM1638 – DATA, CLOCK, STROBE
TM1638 panel(2, 3, 4);

// MAX7219 LED matice
LedControl matice = LedControl(11, 13, 10, 4);

// Relé (pro budoucí čerpadlo a osvětlení)
int pinReleOsvetleni = 7;
int pinReleCerpadlo = 8;

// Senzory
int pinVlhkostPudy = A0;
int pinFotorezistor = A1;

// LED simulace čerpadla
int pinLedCerpadlo = 6;

// Stavy
int stavOsvetleni = LOW;
int stavZavlazovani = LOW;

// Minulé tlačítka
uint8_t predchoziStavTlacitek = 0;

void setup() {
  pinMode(pinReleOsvetleni, OUTPUT);
  pinMode(pinReleCerpadlo, OUTPUT);
  pinMode(pinLedCerpadlo, OUTPUT);
  pinMode(pinFotorezistor, INPUT);

  // Inicializace LED matice
  for (int i = 0; i < 4; i++) {
    matice.shutdown(i, LOW);
    matice.setIntensity(i, 8);
    matice.clearDisplay(i);
  }

  digitalWrite(pinReleOsvetleni, LOW);
  digitalWrite(pinReleCerpadlo, LOW);
  digitalWrite(pinLedCerpadlo, LOW);

  Serial.begin(9600);
}

void loop() {
  uint8_t stavTlacitek = panel.getButtons();

  // --- TM1638 tlačítka ---
  // S1 – ovládání osvětlení (LED matice)
  if ((stavTlacitek & 0x01) && !(predchoziStavTlacitek & 0x01)) {
    stavOsvetleni = (stavOsvetleni == LOW) ? HIGH : LOW;

    if (stavOsvetleni == HIGH) {
      for (int i = 0; i < 4; i++)
        for (int j = 0; j < 8; j++)
          matice.setRow(i, j, 0xFF);
    } else {
      for (int i = 0; i < 4; i++) matice.clearDisplay(i);
    }
  }

  // S2 – ruční ovládání čerpadla (LED)
  if ((stavTlacitek & 0x02) && !(predchoziStavTlacitek & 0x02)) {
    stavZavlazovani = (stavZavlazovani == LOW) ? HIGH : LOW;
  }

  predchoziStavTlacitek = stavTlacitek;

  // --- Čtení senzorů ---
  int hodnotaVlhkosti = analogRead(pinVlhkostPudy);
  int hodnotaSvetla = analogRead(pinFotorezistor);

  // --- LOGIKA LED ČERPADLA ---
  // LED svítí, když je ručně zapnuta nebo pokud je půda suchá nebo tma
  if (stavZavlazovani == HIGH || hodnotaVlhkosti < 400 || hodnotaSvetla < 300) {
    digitalWrite(pinLedCerpadlo, HIGH);
    // Relé připravené pro budoucí čerpadlo
    // digitalWrite(pinReleCerpadlo, HIGH);
  } else {
    digitalWrite(pinLedCerpadlo, LOW);
    // digitalWrite(pinReleCerpadlo, LOW);
  }

  // --- Výpis do Serial Monitoru ---
  Serial.print("Vlhkost: "); Serial.println(hodnotaVlhkosti);
  Serial.print("Svetlo: "); Serial.println(hodnotaSvetla);
  Serial.print("Osvetleni: "); Serial.println(stavOsvetleni);
  Serial.print("Zavlazovani: "); Serial.println(stavZavlazovani);

  delay(200);
}
