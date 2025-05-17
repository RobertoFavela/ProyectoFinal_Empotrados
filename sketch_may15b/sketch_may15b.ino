#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ESP32Servo.h>
#include <NewPing.h>
#include <NoDelay.h>

// Red y contrasena
char* ssid = "MEGACABLE-2.4G-44AB";
const char* password = "kXbHaPf52m";

// Puerto del servidor
AsyncWebServer server(80);

// Pines
const unsigned int PIN_LED = 18;           // LED estado principal
const unsigned int PIN_LED_NO_OBJETO = 5;  // LED sin objeto detectado
const unsigned int PIN_TRIGGER = 2;        // Trigger sensor ultrasónico
const unsigned int PIN_ECHO = 4;           // Echo sensor ultrasónico
const unsigned int PIN_SERVO = 22;         // Servo motor
const unsigned int PIN_LED_FOT = 21;
const unsigned int PIN_FOT = 34;

const unsigned ADC_VALORES = 4096;
const unsigned NIVEL_ON = ADC_VALORES / 5.0;
const unsigned NIVEL_OFF = 2.0 * ADC_VALORES / 5.0;

// Constantes
const int DISTANCIA_MAX = 200;          // Distancia máxima en cm
const int DISTANCIA_ALERTA = 15;        // Umbral de alerta en cm
const long PAUSA = 1000;                // Intervalo entre mediciones
const unsigned int BAUD_RATE = 115200;  // Velocidad puerto serie

// Estados del LED
typedef enum {
  LED_APAGADO,
  LED_ENCENDIDO
} estadoLed;
estadoLed edoLed;

// Estados del LED_FOT
typedef enum {
  LED_FOT_APAGADO,
  LED_FOT_ENCENDIDO
} estadoLed_FOT;
estadoLed_FOT edoLed_FOT;

// Objetos globales
noDelay pausa(PAUSA);
NewPing sonar(PIN_TRIGGER, PIN_ECHO, DISTANCIA_MAX);
Servo servo;
int angulo = 0;

// Declaraciones
int obtenDistancia();
void apagaLED();
void enciendeLED();
void conectaRedWiFi(const char* ssid, const char* password);
void inicializaLittleFS();
void configuraServidor();
void noHallada(AsyncWebServerRequest* request);
String processor(const String& var);
void posicionaServo();

void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LED_NO_OBJETO, OUTPUT);
  pinMode(PIN_LED_FOT, OUTPUT);
  apagaLED();
  apagaLED_FOT();

  servo.attach(PIN_SERVO);
  delay(100);
  servo.write(90);

  conectaRedWiFi(ssid, password);
  inicializaLittleFS();
  configuraServidor();
}
void loop() {
  if (pausa.update()) {
    int distancia = obtenDistancia();

    // Lee y digitaliza el valor del voltaje en la
    // fotoresistencia
    unsigned valorADC = analogRead(PIN_FOT);
    Serial.print("Intensidad de luz (0 - 4095): ");
    Serial.println(valorADC);
    // Prender/apagar el LED
    switch (edoLed_FOT) {
      case LED_APAGADO:
        if (valorADC <= NIVEL_ON)
          enciendeLED_FOT();
        break;
      case LED_ENCENDIDO:
        if (valorADC >= NIVEL_OFF)
          apagaLED_FOT();
    }

    switch (edoLed) {
      case LED_APAGADO:
        if (distancia <= DISTANCIA_ALERTA) {
          enciendeLED();
          digitalWrite(PIN_LED_NO_OBJETO, LOW);
        } else {
          digitalWrite(PIN_LED_NO_OBJETO, HIGH);
        }
        break;

      case LED_ENCENDIDO:
        if (distancia > DISTANCIA_ALERTA) {
          apagaLED();
          digitalWrite(PIN_LED_NO_OBJETO, HIGH);
        } else {
          digitalWrite(PIN_LED_NO_OBJETO, LOW);
        }
        break;
    }
  }
}

int obtenDistancia() {
  int uS = sonar.ping_median();
  return sonar.convert_cm(uS);
}

void apagaLED() {
  digitalWrite(PIN_LED, LOW);
  edoLed = LED_APAGADO;
  servo.write(0);
}

void enciendeLED() {
  digitalWrite(PIN_LED, HIGH);
  edoLed = LED_ENCENDIDO;
  servo.write(90);
}

void apagaLED_FOT() {
  digitalWrite(PIN_LED_FOT, LOW);
  edoLed_FOT = LED_FOT_APAGADO;
  Serial.println("LED_FOT apagado");
}

void enciendeLED_FOT() {
  // Enciende el LED
  digitalWrite(PIN_LED_FOT, HIGH);
  // Actualiza la variable que guarda el estado del LED
  edoLed_FOT = LED_FOT_ENCENDIDO;
  Serial.println("LED_FOT encendido");
}

void conectaRedWiFi(const char* ssid, const char* password) {
  WiFi.begin(ssid, password);
  Serial.print("Conectandose a la red ");
  Serial.print(ssid);
  Serial.println(" ...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConexion establecida");
  Serial.print("Direccion IP del servidor web: ");
  Serial.println(WiFi.localIP());
}

void inicializaLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("Error al montar LittleFS");
  } else {
    Serial.println("LittleFS montado con exito");
  }
}

void configuraServidor() {
  server.serveStatic("/", LittleFS, "/");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/index.html", "text/html", false, processor);
  });

  server.on("/estado", HTTP_GET, [](AsyncWebServerRequest* request) {
    int distancia = obtenDistancia();
    String estado = (distancia <= DISTANCIA_ALERTA) ? "abierta" : "cerrada";
    request->send(200, "text/plain", estado);
  });


  server.onNotFound(noHallada);
  server.begin();
}

void noHallada(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Recurso no encontrado");
}



String processor(const String& var) {
  if (var == "ANGULO") return String(angulo);
  return String();
}

void posicionaServo() {
  Serial.print("Angulo solicitado: ");
  Serial.println(angulo);
  servo.write(angulo);
  delay(50);
  int anguloLeido = servo.read();
  Serial.print("Angulo establecido: ");
  Serial.println(anguloLeido);
}
