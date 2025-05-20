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
const unsigned int PIN_SERVO = 35;         // Servo motor
const unsigned int PIN_LED_FOT = 21;
const unsigned int PIN_FOT = 34;
const int PIN_LUGAR_1 = 26;
const int PIN_LUGAR_2 = 25;
const int PIN_LUGAR_3 = 33;

const int PIN_SENSOR_IR = 32;  // Sensor infrarrojo
bool puertaAbierta = false;


const unsigned ADC_VALORES = 4096;
const unsigned NIVEL_ON = ADC_VALORES / 5.0;
const unsigned NIVEL_OFF = 2.0 * ADC_VALORES / 5.0;
bool luzEncendida = false;
unsigned int umbralFotoresistencia = 50;  // Valor inicial editable por el usuario
unsigned int valorActualFotoresistencia = 0;

// Constantes
const int DISTANCIA_MAX = 200;          // Distancia máxima en cm
const int DISTANCIA_ALERTA = 15;        // Umbral de alerta en cm
const long PAUSA = 1000;                // Intervalo entre mediciones
const unsigned int BAUD_RATE = 115200;  // Velocidad puerto serie



bool lugar1Ocupado = false;
bool lugar2Ocupado = false;
bool lugar3Ocupado = false;


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

enum ModoPaso {
  MODO_NINGUNO,
  MODO_ENTRADA_DETECCION,  // Detectado en el ultrasónico
  MODO_ENTRADA_ESPERA_IR,  // Esperamos cruce del IR
  MODO_SALIDA_DETECCION,   // Detectado en el IR
  MODO_SALIDA_ESPERA_US    // Esperamos que ultrasónico libere la zona
};
ModoPaso modoActual = MODO_NINGUNO;

bool entradaBloqueada = false;

// Objetos globales
noDelay pausa(PAUSA);
NewPing sonar(PIN_TRIGGER, PIN_ECHO, DISTANCIA_MAX);
Servo servo;
int angulo = 90;

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
  pinMode(PIN_SENSOR_IR, INPUT);
  pinMode(PIN_LUGAR_1, INPUT);
  pinMode(PIN_LUGAR_2, INPUT);
  pinMode(PIN_LUGAR_3, INPUT);


  apagaLED();
  apagaLED_FOT();

  servo.attach(PIN_SERVO);
  delay(100);
  servo.write(0);

  conectaRedWiFi(ssid, password);
  inicializaLittleFS();
  configuraServidor();
}
void loop() {
  if (pausa.update()) {
    int distancia = obtenDistancia();

    // Lee y digitaliza el valor del voltaje en la
    // fotoresistencia
    valorActualFotoresistencia = analogRead(PIN_FOT);
    switch (edoLed_FOT) {
      case LED_APAGADO:
        if (valorActualFotoresistencia < umbralFotoresistencia) {
          enciendeLED_FOT();
        }
        break;
      case LED_ENCENDIDO:
        if (valorActualFotoresistencia >= umbralFotoresistencia) {
          apagaLED_FOT();
        }
        break;
    }


    bool objetoDetectado = distancia <= DISTANCIA_ALERTA;     // Sensor ultrasónico (entrada)
    bool sensorIRActivo = digitalRead(PIN_SENSOR_IR) == LOW;  // Sensor IR activo cuando detecta

    switch (modoActual) {
      case MODO_NINGUNO:
        if (!entradaBloqueada && objetoDetectado && !sensorIRActivo) {
          Serial.println("Vehículo acercándose - Entrada");
          enciendeLED();
          modoActual = MODO_ENTRADA_ESPERA_IR;
        } else if (sensorIRActivo && !objetoDetectado) {
          Serial.println("Vehículo iniciando salida");
          enciendeLED();
          modoActual = MODO_SALIDA_ESPERA_US;
        }
        break;

      case MODO_ENTRADA_ESPERA_IR:
        if (sensorIRActivo) {
          Serial.println("Vehículo terminó de entrar");
          apagaLED();
          registrarEntrada();
          modoActual = MODO_NINGUNO;
        }
        break;

      case MODO_SALIDA_ESPERA_US:
        if (!objetoDetectado) {
          Serial.println("Vehículo terminó de salir");
          apagaLED();
          registrarSalida();
          modoActual = MODO_NINGUNO;
        }
        break;
    }
  }

  lugar1Ocupado = digitalRead(PIN_LUGAR_1) == LOW;
  lugar2Ocupado = digitalRead(PIN_LUGAR_2) == LOW;
  lugar3Ocupado = digitalRead(PIN_LUGAR_3) == LOW;
}

void registrarEntrada() {
  if (!lugar1Ocupado) {
    lugar1Ocupado = true;
  } else if (!lugar2Ocupado) {
    lugar2Ocupado = true;
  } else if (!lugar3Ocupado) {
    lugar3Ocupado = true;
  } else {
    Serial.println("¡Estacionamiento lleno!");
  }
}

bool estacionamientoLleno() {
  return lugar1Ocupado && lugar2Ocupado && lugar3Ocupado;
}

void registrarSalida() {
  // En un sistema real necesitarías saber cuál vehículo salió
  // Aquí lo hacemos simple y liberamos el último ocupado
  if (lugar3Ocupado) {
    lugar3Ocupado = false;
  } else if (lugar2Ocupado) {
    lugar2Ocupado = false;
  } else if (lugar1Ocupado) {
    lugar1Ocupado = false;
  } else {
    Serial.println("¡No hay autos que hayan salido!");
  }
}

bool estacionamientoVacio() {
  return !lugar1Ocupado && !lugar2Ocupado && !lugar3Ocupado;
}

int obtenDistancia() {
  int uS = sonar.ping_median();
  return sonar.convert_cm(uS);
}

void apagaLED() {
  digitalWrite(PIN_LED, LOW);
  edoLed = LED_APAGADO;
  digitalWrite(PIN_LED_NO_OBJETO, HIGH);
  servo.write(0);
  puertaAbierta = false;
}

void enciendeLED() {
  digitalWrite(PIN_LED, HIGH);
  edoLed = LED_ENCENDIDO;
  digitalWrite(PIN_LED_NO_OBJETO, LOW);
  servo.write(90);
  puertaAbierta = true;
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
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.on("/bloqueo", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (request->hasParam("activo", true)) {
      String valor = request->getParam("activo", true)->value();
      entradaBloqueada = (valor == "true");
      Serial.printf("Entrada bloqueada: %s\n", entradaBloqueada ? "Sí" : "No");
      request->send(200, "text/plain", "Estado actualizado");
    } else {
      request->send(400, "text/plain", "Falta parámetro");
    }
  });

  server.on("/umbral", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (request->hasParam("valor", true)) {
      String valorStr = request->getParam("valor", true)->value();
      int nuevoUmbral = valorStr.toInt();
      if (nuevoUmbral >= 0 && nuevoUmbral <= 4095) {
        umbralFotoresistencia = nuevoUmbral;
        Serial.printf("Nuevo umbral de fotoresistencia: %d\n", umbralFotoresistencia);
        request->send(200, "text/plain", "Umbral actualizado correctamente");
      } else {
        request->send(400, "text/plain", "Valor fuera de rango");
      }
    } else {
      request->send(400, "text/plain", "Falta parámetro 'valor'");
    }
  });

  server.on("/estado", HTTP_GET, [](AsyncWebServerRequest* request) {
    String json = "{";
    json += "\"puerta\":\"" + String(puertaAbierta ? "abierta" : "cerrada") + "\",";
    json += "\"lugar1\":\"" + String(lugar1Ocupado ? "ocupado" : "vacio") + "\",";
    json += "\"lugar2\":\"" + String(lugar2Ocupado ? "ocupado" : "vacio") + "\",";
    json += "\"lugar3\":\"" + String(lugar3Ocupado ? "ocupado" : "vacio") + "\",";
    json += "\"luz\":\"" + String(edoLed_FOT == LED_FOT_ENCENDIDO ? "encendida" : "apagada") + "\",";
    json += "\"fotoValor\":" + String(analogRead(PIN_FOT)) + ",";
    json += "\"umbral\":" + String(NIVEL_ON) + ",";
    json += "\"bloqueado\":" + String(entradaBloqueada ? "true" : "false");
    json += "}";
    request->send(200, "application/json", json);
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