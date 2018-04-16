/* Como estan puestos los pins desde el lcd hacia el arduino

  1 - GND                              9  - XXXX
  2 - 3.3V                             10 - XXXX
  3 - MID POTENCIOMETRO                11 - PIN 10
  4 - PIN 12                           12 - PIN 9
  5 - GND                              13 - PIN 8
  6 - PIN 11                           14 - PIN 7
  7 - XXXX                             15 - 3.3V +RESISTENCIA 1K
  8 - XXXX                             16 - GND

  PINS DESDE EL ARDUINO USADOS:
  12,11,10,9,8,7 Y TENSION
  AL POTENCIOMETRO DE FRENTE IZQUIERDA POSITIVO DERECHA NEGATIVO

  FALTARIA CONTRA LOS LED Y EL LECTOR RFID

  LECTOR RFID

*/

// include the library code:
#include <LiquidCrystal.h>
#include <DHT.h>

#define DHTTYPE DHT11   // DHT 11 o DHT 22  (AM2302), AM2321
const int DHTPin = 22;     // what digital pin we're connected to
DHT dht(DHTPin, DHTTYPE);

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

byte smiley[8] = {
  B00000,
  B01010,
  B01010,
  B01010,
  B00000,
  B10001,
  B01110,
  B00000
};

void setup() {
  
   //se crea el caracter de la cara
  lcd.createChar(1, smiley);
  //Inicializacion del lcd
  lcd.begin(16, 2);
  //Se imprimen las caritas
  caritas();
  delay(3000);
  
  lcd.clear();
   
  lcd.print("Hola Vitooo!");
   
  Serial.begin(9600);
  Serial.println("DHT11 test!");
 
  dht.begin();
}

void loop() {

   float h = dht.readHumidity();
   float t = dht.readTemperature();
 
   if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
   }
 
   Serial.print("Humedad: ");
   Serial.print(h);
   Serial.print(" %\t");
   Serial.print("Temperatura: ");
   Serial.print(t);
   Serial.print(" *C\n");
   
   delay(2000);
    //Imprime la humedad
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Humedad:  ");
    lcd.print(h);
    lcd.print("%");
   
    //Imprime la temperatura
    lcd.setCursor(0, 1);
    lcd.print("Temp:     "); 
    lcd.print(t);
    lcd.print("C");
}

void caritas(){
    //Se imprimen las caritas 1 x 1
  for (byte i = 0; i < 16; i = i + 2) {
    lcd.setCursor(i, 0);
    lcd.write(1);
  }
  for (byte i = 1; i < 16; i = i + 2) {
    lcd.setCursor(i, 1);
    lcd.write(1);
  }
}

