// TODO: Figure out a way to rename the device without having to go into boot
// mode to upload new code // NOTE: probably wont do it since it seems it messes
// with arduino IDE too much, a workaround in python is better

/*Drawing sprites:
 * in the beginning have global bitmap variables that you can copy (probs from a
 * python script that converts images to bitmap) In the eyes draw commands
 * instead of drawing shapes, draw these bitmaps The eye sclera(the white part
 * aka the background) is a sprite (maybe with some tasteful shading probs), The
 * iris(colorful part) is also a sprite The pupil(black part) is procedural, a
 * circle For displaying images, have a separate rendering mode for rendering
 * lone sprites (question marks, emojis etc.) These images are sent over usb as
 * raw bitmaps.
 */

// ==========For display code===========
#include "Robot_Eye.h"
#include "Robot_Eye_Pair.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <ArduinoJson.h>     // JSON library for easy communication
#include <SPI.h>

// For the breakout board, you can use any 2 or 3 pins.
// These pins will also work for the 1.8" TFT shield.
#define TFT1_CS D5
#define TFT2_CS D4
#define TFT_RST D3 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC D2

// OPTION 1 (recommended) is to use the HARDWARE SPI pins, which are unique
// to each board and not reassignable. For Arduino Uno: MOSI = pin 11 and
// SCLK = pin 13. This is the fastest mode of operation and is required if
// using the breakout board's mic
// For 1.44" and 1.8" TFT with ST7735 use:
Adafruit_ST7735 tft1 = Adafruit_ST7735(TFT1_CS, TFT_DC, TFT_RST);
Adafruit_ST7735 tft2 = Adafruit_ST7735(
    TFT2_CS, TFT_DC,
    -1); // help gotten from https://forums.adafruit.com/viewtopic.php?t=171191

// Canvas is initialized the same size as the tft1 display, assuming both
// displays are of the same size this is fine
GFXcanvas16 canvas(tft1.width(),
                   tft1.height()); // offscreen canvas to avoid flickering

Eye Eye1 = Eye(tft1, canvas, "L");
Eye Eye2 = Eye(tft2, canvas, "R");
Eye_Pair Eyes = Eye_Pair(Eye1, Eye2);

// ======================================

// =====Api commands through serial======

using CommandHandler = void (*)(JsonDocument &);

void handleState(JsonDocument &doc) {
  const char *state = doc["state"];
  if (state)
    Eyes.setEyePairState(StringToEyePairState(state));
}

void handleLook(JsonDocument &doc) {
  float x = doc["x"] | 0.0;
  float y = doc["y"] | 0.0;

  Eyes.look(x, y);
}

void handleMood(JsonDocument &doc) {
  const char *mood = doc["mood"];
  if (mood)
    Eyes.setMood(getMood(mood));
}

void handlePupilRadius(JsonDocument &doc) {
  float radius = doc["radius"] | 0.0;

  Eyes.setPupilRadius(radius);
}

void handleBlink(JsonDocument &doc) { Eyes.blink(); }

void handleGlance(JsonDocument &doc) {
  if (doc["x"].is<float>() && doc["y"].is<float>())
    Eyes.glance(doc["x"], doc["y"]);
  else
    Eyes.glance();
}

struct Command {
  const char *name;
  CommandHandler handler;
};

const Command commands[] = {
    {"state", handleState}, {"look", handleLook},
    {"mood", handleMood},   {"pupil_radius", handlePupilRadius},
    {"blink", handleBlink}, {"glance", handleGlance},
};

void handleCommand(JsonDocument &doc) {
  const char *cmd = doc["cmd"];

  if (!cmd)
    return;

  for (const Command &command : commands) {
    if (strcmp(cmd, command.name) == 0) {
      command.handler(doc);
      return;
    }
  }

  Serial.print("Unknown cmd: ");
  Serial.println(cmd);
}

// ======================================

// Muuttuja sarjaliikenteestä saadulle tiedolle
String receivedData = "";
bool newDataAvailable = false;

// Tehtävän kahva (valinnainen, jos tehtävää pitää hallita myöhemmin)
TaskHandle_t SerialTaskHandle = NULL;

// Tehtävä sarjaliikenteen kuunteluun (ajetaan taustalla)
void serialListenerTask(void *pvParameters) {
  while (1) {
    if (Serial.available() > 0) {
      char c = Serial.read();

      if (c == '\n') { // Rivinvaihto tarkoittaa viestin loppua
        newDataAvailable = true;
      } else if (c != '\r') { // Jätetään vaununpalautus huomiotta
        receivedData += c;
      }
    }
    vTaskDelay(
        pdMS_TO_TICKS(10)); // Pieni viive estää prosessorin ylikuormituksen
  }
}

void setup() {
  Serial.begin(9600);

  delay(1000); // Odotetaan, että sarjaportti herää

  // Luodaan taustatehtävä sarjaliikenteelle corelle 0
  xTaskCreatePinnedToCore(serialListenerTask, // Suoritettava funktio
                          "SerialListener",   // Tehtävän nimi
                          3000,               // Pinon koko tavuina
                          NULL,               // Parametrit funktiolle
                          1, // Prioriteetti (1 = matala/normaali)
                          &SerialTaskHandle, // Tehtävän kahva
                          0 // Suoritinydin (ESP32-S3:ssa on ytimet 0 ja 1)
  );

  Serial.println("ESP32-S3 connected to serial.");
  Serial.println("Initializing displays:");

  // OR use this initializer (uncomment) if using a 1.44" TFT:
  tft1.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab
  tft2.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab
  tft1.setRotation(3);           // Rotate display 1 by 90 degrees
  tft2.setRotation(1);           // Rotate display 1 by 90 degrees

  // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can
  // override it here Note that speed allowable depends on chip and quality of
  // wiring, if you go too fast, you may end up with a black screen some times,
  // or all the time.
  // tft1.setSPISpeed(40000000);

  Serial.println(F("Displays Initialized"));

  Eyes.setIdle(true);
}

// Pääsilmukka, joka pyörii itsenäisesti corella 1
void loop() {
  // --- TÄSSÄ ON MUU CODESI, JOKA PYÖRII SAMANAIKAISESTI ---
  if (newDataAvailable) {

    // Parse JSON
    StaticJsonDocument<256> doc;

    DeserializationError error = deserializeJson(doc, receivedData);

    if (error) {
      Serial.print("JSON parse failed: ");
      Serial.println(error.c_str());

      // Reset state even on failure
      receivedData = "";
      newDataAvailable = false;
      return;
    }

    handleCommand(doc);

    receivedData = "";
    newDataAvailable = false;
  }
  // if(Eyes.State == EyePairState::ACTIVE)Serial.println("Active");
  // if(Eyes.State == EyePairState::DEBUG)Serial.println("Debug");
  // if(Eyes.State == EyePairState::IDLE)Serial.println("Idle");
  Eyes.showEyes();
}
