#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"

// ====== ECRAN OLED (integre au T-Beam Supreme) ======
#define OLED_SDA  17
#define OLED_SCL  18
#define OLED_RST  -1
#define OLED_ADDR 0x3C
#define SCREEN_W  128
#define SCREEN_H  64

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, OLED_RST);

// ====== BROCHE DU POTENTIOMETRE ======
// >>> MODIFIEZ cette valeur selon votre branchement <<<
const int POT_PIN = 36;

// ====== INTERVALLE ENTRE LES APPELS ======
const unsigned long INTERVALLE_APPEL = 5000; // 5 secondes

// ====== VARIABLES GLOBALES ======
WiFiClientSecure wifiClient;
unsigned long dernierAppel = 0;

// ------------------------------------------------------------
// afficherOLED(ligne1, ligne2) — Affiche du texte sur l'OLED
// ------------------------------------------------------------
void afficherOLED(String ligne1, String ligne2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Ligne 1 en haut (petite)
  display.setCursor(0, 0);
  display.println(ligne1);

  // Ligne 2 au centre (plus grande)
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println(ligne2);

  display.display();
}

// ------------------------------------------------------------
// connexionWiFi() — Se connecte au reseau WiFi
// ------------------------------------------------------------
void connexionWiFi() {
  Serial.print("Connexion WiFi a ");
  Serial.println(WIFI_SSID);
  afficherOLED("WiFi...", WIFI_SSID);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connecte! IP: ");
  Serial.println(WiFi.localIP());
  afficherOLED("WiFi OK", WiFi.localIP().toString());
}

// ------------------------------------------------------------
// appelLLM(valeurPot) — Envoie la valeur du pot au LLM
//                        et retourne la reponse textuelle
// ------------------------------------------------------------
String appelLLM(int valeurPot) {
  // Desactiver la verification des certificats SSL (demo seulement)
  wifiClient.setInsecure();

  HTTPClient http;
  http.begin(wifiClient, OPENWEBUI_URL);

  // Headers HTTP : JSON + token d'authentification Bearer
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(API_KEY));

  // Corps de la requete — format compatible OpenAI
  // "system" = le contexte (votre scenario)
  // "user" = la valeur du potentiometre
  JsonDocument doc;
  doc["model"] = MODEL_NAME;
  doc["max_tokens"] = 60;
  doc["temperature"] = 0.8;

  JsonArray messages = doc["messages"].to<JsonArray>();

  JsonObject sysMsg = messages.add<JsonObject>();
  sysMsg["role"] = "system";
  sysMsg["content"] = SYSTEM_PROMPT;

  JsonObject userMsg = messages.add<JsonObject>();
  userMsg["role"] = "user";
  userMsg["content"] = "Valeur du potentiometre : " + String(valeurPot);

  String body;
  serializeJson(doc, body);

  Serial.println("[LLM] Envoi...");
  afficherOLED("Pot: " + String(valeurPot), "Envoi au LLM...");

  // Envoi du POST
  int httpCode = http.POST(body);

  String resultat = "";

  if (httpCode == 200) {
    String reponse = http.getString();
    JsonDocument repDoc;
    DeserializationError err = deserializeJson(repDoc, reponse);

    if (!err) {
      // La reponse se trouve dans choices[0].message.content
      const char* texte = repDoc["choices"][0]["message"]["content"];
      if (texte) {
        resultat = String(texte);
      } else {
        resultat = "[Pas de contenu]";
      }
    } else {
      resultat = "[Erreur JSON]";
    }
  } else {
    resultat = "[HTTP " + String(httpCode) + "]";
    Serial.println(http.getString());
  }

  http.end();
  return resultat;
}

// ------------------------------------------------------------
// setup()
// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("=== Labo 4 - LLM sur T-Beam Supreme ===");

  // Initialiser l'OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[OLED] Echec d'initialisation!");
  }
  afficherOLED("Labo 4", "Demarrage...");

  // Potentiometre
  pinMode(POT_PIN, INPUT);

  // WiFi
  connexionWiFi();
  delay(1000);

  afficherOLED("Pret!", "Tournez le pot...");
  Serial.println("=== Pret! Tournez le potentiometre ===");
}

// ------------------------------------------------------------
// loop()
// ------------------------------------------------------------
void loop() {
  // Reconnexion WiFi si necessaire
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] Reconnexion...");
    connexionWiFi();
  }

  // Lire le potentiometre (0 a 4095)
  int valeurPot = analogRead(POT_PIN);

  // Appeler le LLM a intervalle regulier
  unsigned long maintenant = millis();
  if (maintenant - dernierAppel >= INTERVALLE_APPEL) {
    dernierAppel = maintenant;

    String reponse = appelLLM(valeurPot);

    // Afficher sur le moniteur serie
    Serial.print("[LLM] Pot=");
    Serial.print(valeurPot);
    Serial.print(" -> ");
    Serial.println(reponse);

    // Afficher sur l'OLED
    afficherOLED("Pot: " + String(valeurPot), reponse);
  }

  delay(100);
}
