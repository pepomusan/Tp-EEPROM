// TOVBEIN, MUSAN, NISIM
// GRUPO 10
#include <Preferences.h>
#include <DHT.h>
#include <DHT_U.h>
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <Adafruit_Sensor.h>

#define DHTTYPE DHT11
#define DHTPIN 23

#define ESTADO_SUMA 100
#define ESTADO_RESTA 200
#define ESTADO_P1 300
#define ESTADO_P2 400
#define ESTADO_ESPERA1 500
#define ESTADO_ESPERA2 600
#define PIN_BOTON_A 35
#define PIN_BOTON_B 34

Preferences preferences;

int tiempoTranscurrido;
int tiempoActual;
int tiempoInicioEspera = 0;
int estadoActual = ESTADO_P1;
int umbralTemp;
int umbralTempAnterior;

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
DHT dht(DHTPIN, DHTTYPE);

void manejarEstados(float temperatura, int estadoBotonA, int estadoBotonB);
int estadoBotonA;
int estadoBotonB;

void setup() {
  Serial.begin(115200);
  Serial.println(F("OLED test"));
  u8g2.begin();
  dht.begin();
  pinMode(PIN_BOTON_A, INPUT_PULLUP);
  pinMode(PIN_BOTON_B, INPUT_PULLUP);
  pinMode(25, OUTPUT);
  preferences.begin("Umbral", false);
  umbralTemp = preferences.getInt("Umbral", 28);
}

void loop() {
  estadoBotonA = digitalRead(PIN_BOTON_A);
  estadoBotonB = digitalRead(PIN_BOTON_B);
  float temperatura = dht.readTemperature();
  manejarEstados(temperatura, estadoBotonA, estadoBotonB);
  delay(100);
}

void manejarEstados(float temperatura, int estadoBotonA, int estadoBotonB) {
  char textoTemp[6];
  char textoUmbral[5];
  tiempoActual = millis();

  switch (estadoActual) {
    case ESTADO_P1:
      u8g2.clearBuffer();
      sprintf(textoTemp, "%.2f", temperatura);
      sprintf(textoUmbral, "%d", umbralTemp);
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(15, 15, "Temp:");
      u8g2.drawStr(70, 15, textoTemp);
      u8g2.drawStr(15, 30, "Umbral:");
      u8g2.drawStr(70, 30, textoUmbral);
      u8g2.sendBuffer();
      if (estadoBotonA == LOW) {
        tiempoInicioEspera = tiempoActual;
        estadoActual = ESTADO_ESPERA1;
      }
      break;

    case ESTADO_ESPERA1:
      tiempoTranscurrido = tiempoActual - tiempoInicioEspera;
      Serial.println(tiempoTranscurrido / 1000);
      umbralTempAnterior = umbralTemp;
      if (estadoBotonA == HIGH && tiempoTranscurrido >= 5000) {
        estadoActual = ESTADO_P2;
      }
      if (estadoBotonA == HIGH && tiempoTranscurrido <= 5000) {
        estadoActual = ESTADO_P1;
      }
      break;

    case ESTADO_P2:
      u8g2.clearBuffer();
      sprintf(textoUmbral, "%d", umbralTemp);
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(15, 15, "Umbral:");
      u8g2.drawStr(70, 15, textoUmbral);
      u8g2.sendBuffer();
      if (estadoBotonA == LOW) {
        estadoActual = ESTADO_SUMA;
      }
      if (estadoBotonB == LOW) {
        tiempoInicioEspera = tiempoActual;
        estadoActual = ESTADO_RESTA;
      }
      break;

    case ESTADO_SUMA:
      if (estadoBotonA == HIGH) {
        umbralTemp += 1;
        estadoActual = ESTADO_P2;
      }
      break;

    case ESTADO_RESTA:
      tiempoTranscurrido = tiempoActual - tiempoInicioEspera;
      Serial.println(tiempoTranscurrido / 1000);
      if (estadoBotonB == HIGH && tiempoTranscurrido <= 5000) {
        umbralTemp -= 1;
        estadoActual = ESTADO_P2;
      }
      if (estadoBotonB == HIGH && tiempoTranscurrido >= 5000) {
        estadoActual = ESTADO_P1;
        if (umbralTemp != umbralTempAnterior) {
          preferences.putInt("Umbral", umbralTemp);
        }
      }
      break;
  }
}
