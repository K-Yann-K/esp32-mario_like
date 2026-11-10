#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* AP_SSID = "MarioESP";

const int SOL_Y = 56;
const int PLAFOND = 16;
const float VIT_MARCHE = 1.5;
const float GRAVITE = 0.5;
const float VIT_SAUT = -4.5;
const float VIT_CHUTE_MAX = 6.0;
const uint32_t PERIODE_FRAME = 33;

const unsigned char PROGMEM stickman_fix[] = {0x3C,0x24,0x3C,0x18,0x5A,0x18,0x24,0x42};
const unsigned char PROGMEM stickman_marche[]   = {0x3C,0x24,0x3C,0x18,0x3A,0x18,0x28,0x48};
const unsigned char PROGMEM stickman_saut[]     = {0x3C,0x24,0x3C,0x7E,0x18,0x24,0x42,0x81};
const unsigned char* const SPRITES[3] = {stickman_fix, stickman_marche, stickman_saut};

float x = 10, y = SOL_Y - 8, vx = 0, vy = 0;
uint8_t image = 0;
bool gauche = false, droite = false, sautDemande = false, auSol = true;
uint32_t frame = 0, prochaine = 0;

Adafruit_SSD1306 ecran(128, 64, &Wire, -1, 400000UL, 400000UL);
WiFiServer serveur(5000);
WiFiClient client;
bool aUnClient = false;

void lireReseau() {
  bool connecte = client && client.connected();
  if (!connecte) {
    if (aUnClient) {
      aUnClient = false;
      client.stop();
      gauche = droite = sautDemande = false;
    }
    WiFiClient nouveau = serveur.available();
    if (nouveau) {
      client = nouveau;
      client.setNoDelay(true);
      aUnClient = true;
      client.println("MARIO pret");
    }
    return;
  }
  while (client.available()) {
    switch ((char)client.read()) {
      case 'G': gauche = true;  break;
      case 'g': gauche = false; break;
      case 'D': droite = true;  break;
      case 'd': droite = false; break;
      case 'S': sautDemande = true; break;
    }
  }
}

void moteur() {
  vx = 0;
  if (gauche) vx -= VIT_MARCHE;
  if (droite) vx += VIT_MARCHE;
  x = constrain(x + vx, 0, 120);

  if (sautDemande && auSol) { vy = VIT_SAUT; auSol = false; }
  sautDemande = false;

  vy = min(vy + GRAVITE, VIT_CHUTE_MAX);
  y += vy;

  if (y >= SOL_Y - 8) { y = SOL_Y - 8; vy = 0; auSol = true; }
  else auSol = false;
  if (y < PLAFOND) { y = PLAFOND; vy = max(vy, 0.0f); }

  if (!auSol) image = 2;
  else if (vx != 0) image = ((frame / 6) % 2) ? 0 : 1;
  else image = 0;
}

void dessiner() {
  ecran.clearDisplay();
  ecran.drawFastHLine(0, SOL_Y, 128, SSD1306_WHITE);
  ecran.drawBitmap((int16_t)x, (int16_t)y, SPRITES[image], 8, 8, SSD1306_WHITE);
  ecran.display();
}

void setup() {
  Wire.begin(15, 18);
  ecran.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  ecran.setTextColor(SSD1306_WHITE);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID);
  serveur.begin();
}

void loop() {
  if (millis() < prochaine) return;
  prochaine = millis() + PERIODE_FRAME;
  frame++;
  lireReseau();
  moteur();
  dessiner();
}