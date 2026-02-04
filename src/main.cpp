#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const int semaforoVerde   = 23;
const int semaforoAmarillo= 22;
const int semaforoRojo    = 21;

const int peatonRojo      = 19;
const int peatonVerde     = 18;

const int botonCambio     = 15;

AsyncWebServer server(80);

volatile bool solicitudPeaton = false;
unsigned long lastBtnMs = 0;

void estadoNormal() {
  digitalWrite(semaforoVerde, HIGH);
  digitalWrite(semaforoAmarillo, LOW);
  digitalWrite(semaforoRojo, LOW);

  digitalWrite(peatonRojo, HIGH);
  digitalWrite(peatonVerde, LOW);
}

void fasePeaton() {
  digitalWrite(semaforoVerde, HIGH);
  digitalWrite(peatonRojo, HIGH);
  delay(2000);
  digitalWrite(semaforoVerde, LOW);

  digitalWrite(semaforoAmarillo, HIGH);
  delay(2000);
  digitalWrite(semaforoAmarillo, LOW);

  digitalWrite(semaforoRojo, HIGH);
  digitalWrite(peatonRojo, LOW);
  digitalWrite(peatonVerde, HIGH);

  unsigned long inicio = millis();
  while (millis() - inicio < 10000) {
    if (digitalRead(botonCambio) == LOW) break;
    delay(10);
  }

  delay(2000);
  digitalWrite(semaforoRojo, LOW);
  digitalWrite(semaforoAmarillo, HIGH);
  delay(2000);

  digitalWrite(peatonVerde, LOW);
  digitalWrite(peatonRojo, HIGH);
  delay(2000);

  digitalWrite(semaforoAmarillo, LOW);
  digitalWrite(semaforoVerde, HIGH);
}

String paginaHTML() {
  String html = "<!doctype html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Semaforo ESP32</title></head><body>";
  html += "<h1>Semaforo ESP32</h1>";
  html += "<p><a href='/cruzar'>Solicitar cruce peaton</a></p>";
  html += "<p><a href='/reset'>Reset (estado normal)</a></p>";
  html += "<p>Tip: tambien funciona el boton fisico (GPIO 15).</p>";
  html += "</body></html>";
  return html;
}

void setup() {
  Serial.begin(115200);

  pinMode(semaforoVerde, OUTPUT);
  pinMode(semaforoAmarillo, OUTPUT);
  pinMode(semaforoRojo, OUTPUT);

  pinMode(peatonVerde, OUTPUT);
  pinMode(peatonRojo, OUTPUT);

  pinMode(botonCambio, INPUT_PULLUP);

  estadoNormal();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando a Wi-Fi...");
  }
  Serial.println("Conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // las rutas web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", paginaHTML());
  });

  server.on("/cruzar", HTTP_GET, [](AsyncWebServerRequest *request) {
    solicitudPeaton = true;  // lo ejecuta loop 
    request->send(200, "text/html", "<p>OK, cruce solicitado. <a href='/'>Volver</a></p>");
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    solicitudPeaton = false;
    estadoNormal();
    request->send(200, "text/html", "<p>Reset hecho. <a href='/'>Volver</a></p>");
  });

  server.begin();
}

void loop() {
  //boton físico por el rebote de mantener pulsaddo
  if (digitalRead(botonCambio) == LOW && (millis() - lastBtnMs) > 250) {
    lastBtnMs = millis();
    solicitudPeaton = true;
  }

  //si hay solicitud ejecutro 
  if (solicitudPeaton) {
    solicitudPeaton = false;
    fasePeaton();
    estadoNormal();
  }

  delay(10);
}
