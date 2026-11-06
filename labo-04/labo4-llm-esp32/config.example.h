/*
 * config.example.h - Template de configuration
 * Copier vers config.h et remplir avec vos valeurs.
 */

#pragma once

// =============================================
// CONFIGURATION WIFI
// =============================================

#define WIFI_SECURITY_WPA2_PERSONAL
const char* WIFI_SSID     = "votre-ssid";
const char* WIFI_PASSWORD = "votre-mot-de-passe";

// =============================================
// CONFIGURATION LLM
// =============================================

// --- Étape 1 : endpoint du cours ---
const char* OPENWEBUI_URL = "https://chat.ve2fpd.com/api/chat/completions";
const char* API_KEY        = "cle-fournie-en-classe";
const char* MODEL_NAME     = "assistant-iot-v2";

// --- Étape 2 : votre propre compte Groq ---
// const char* OPENWEBUI_URL = "https://api.groq.com/openai/v1/chat/completions";
// const char* API_KEY        = "votre-cle-api-groq";
// const char* MODEL_NAME     = "meta-llama/llama-4-scout-17b-16e-instruct";

// =============================================
// SYSTEM PROMPT - À PERSONNALISER!
// =============================================

const char* SYSTEM_PROMPT =
  "Tu es un entraineur de CrossFit intense et motivant. "
  "Le potentiometre represente le niveau d'effort de l'athlete "
  "de 0 (echauffement tranquille) a 4095 (effort maximal absolu). "
  "REGLE ABSOLUE: ta reponse doit faire MAXIMUM 50 caracteres. "
  "Une seule phrase ultra courte. Pas de ponctuation superflue.";
