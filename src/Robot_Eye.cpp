//<3 https://www.youtube.com/watch?v=IiZl3p-ZohM <3

#include "Robot_Eye.h"


//Returns the Moods enum that has the same name as the string, if no enum item matches the string, returns neutral
Moods getMood(String moodStr) {
  if (moodStr == "NEUTRAL") return Moods::NEUTRAL;
  if (moodStr == "HAPPY") return Moods::HAPPY;
  if (moodStr == "SAD") return Moods::SAD;
  if (moodStr == "SURPRISED") return Moods::SURPRISED;
  if (moodStr == "CONFUSED") return Moods::CONFUSED;

  return Moods::NEUTRAL;  // Oletuspalautus
}

String getMoodStr(Moods mood) {
  switch (mood) {
    case Moods::NEUTRAL: return "NEUTRAL";
    case Moods::HAPPY: return "HAPPY";
    case Moods::SAD: return "SAD";
    case Moods::SURPRISED: return "SURPRISED";
    case Moods::CONFUSED: return "CONFUSED";
    default: return "UNKNOWN";  // Virhetilanteita varten
  }
}


Eye::Eye(Adafruit_ST7735& tftObj, GFXcanvas16& canvasObj, String eyeName = "Silmä")
  : current_mood(Moods::NEUTRAL),
    current_state(EyeStates::OPEN),
    current_directionX(0),
    current_directionY(0),
    desired_directionX(0.0),
    desired_directionY(0.0),
    desiredPupilRadius(30),
    currentPupilRadius(30),
    frownAmt(0.0),
    currentEylidState(0.0),
    tft(tftObj),
    canvas(canvasObj) {

  name = eyeName;

  time_at_state_change = millis();
}

uint16_t Eye::moodColor() {
  switch (current_mood) {
    case Moods::NEUTRAL:
      return WHITE;
    case Moods::HAPPY:
      return GREEN;
    case Moods::SAD:
      return BLUE;
    case Moods::CONFUSED:
      return YELLOW;
    case Moods::SURPRISED:
      return MAGENTA;
    default:
      return 1;  // Oletuspalautus
  }
}

float Eye::moodFrown() {
  switch (current_mood) {
    case Moods::NEUTRAL:
      return 0.2;
    case Moods::HAPPY:
      return 0.3;
    case Moods::SAD:
      return 0.5;
    case Moods::CONFUSED:
      return 0.7;
    case Moods::SURPRISED:
      return 0.0;
    default:
      return 0.5;  // Oletuspalautus
  }
}


void Eye::setDirection(float desired_directionX_new, float desired_directionY_new) {
  desired_directionX = desired_directionX_new;
  desired_directionY = desired_directionY_new;
}

void Eye::setState(EyeStates newState) {
  //overwrites the old state with the new state
  time_at_state_change = millis();
  current_state = newState;
}

void Eye::changeMood(Moods newMood) {
  current_mood = newMood;
}

// checks if the current state (mainly animations) should decay into a static state
void Eye::checkDecayState() {
  switch (current_state) {

    case EyeStates::CLOSING:
      if (time_at_state_change + T_LENGTH_CLOSING > millis()) return;
      setState(CLOSING_DECAY_INTO);
      return;

    case EyeStates::OPENING:
      if (time_at_state_change + T_LENGTH_OPENING > millis()) return;
      setState(OPENING_DECAY_INTO);
      return;
    case EyeStates::BLINK_CLOSING:
      if (time_at_state_change + T_LENGTH_BLINK_CLOSING > millis()) return;
      setState(BLINK_CLOSING_DECAY_INTO);
      return;
  }
}



// Show eye needs to be in the draw loop
void Eye::showEye() {

  // draw the background
  //canvas.fillScreen(SCLERA_COLOR);
  canvas.drawRGBBitmap(0, 0, SPRITE_EYE_SCLERA.image, SPRITE_EYE_SCLERA.w, SPRITE_EYE_SCLERA.h);

  //Check if the current state should decay
  checkDecayState();
  unsigned long dt = millis() - time_at_last_draw;
  {
    //Lerp the eyes current position towards the desired position according to the deltatime between these draw calls
    const float t = constrain(0.9 * float(dt) * 0.0001, 0.3, 0.95);

    current_directionX = round(t * float(current_directionX) + (1.0 - t) * desired_directionX * float(tft.width()));
    current_directionY = round(t * float(current_directionY) + (1.0 - t) * desired_directionY * float(tft.height()));
  }

  //Lerp the pupil radius towards the desired radius:
  {
    const float t = constrain(0.9 * float(dt) * 0.003, 0.3, 0.95);
    currentPupilRadius = round(t * float(currentPupilRadius) + (1.0 - t) * desiredPupilRadius);
  }


  //Show the eye in the correct position and with the correct animation etc.
  // NOTE: adafruit gfx library help: https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives


  currentEylidState = 0.0;  // reset current eylid state if not blinking, since frowning has a separate variable this works


  //Draw the iris
  //canvas.fillCircle(current_directionX, current_directionY, IRIS_RADIUS, moodColor());
  drawRGBBitmapTransparent(canvas, current_directionX - SPRITE_EYE_IRIS.w/2, current_directionY - SPRITE_EYE_IRIS.h/2, SPRITE_EYE_IRIS.image, SPRITE_EYE_IRIS.w, SPRITE_EYE_IRIS.h);


  // Switch case statements for drawing each different state
  switch (current_state) {
    case EyeStates::OPEN:
      canvas.fillCircle(current_directionX, current_directionY, currentPupilRadius, PUPIL_COLOR);

      break;
    case EyeStates::CLOSED:
      currentEylidState = 1.0;
      canvas.fillScreen(EYLID_COLOR);
      break;

    //Animation states from this point onwards
    case EyeStates::OPENING:
      {
        canvas.fillCircle(current_directionX, current_directionY, currentPupilRadius, PUPIL_COLOR);

        //Draw the eylids as boxes for now.
        //Depending on how much time has passed since the last
        currentEylidState = 1.0 - min(float(millis() - time_at_state_change) / float(T_LENGTH_OPENING), 1.0f);
        break;
      }
    case EyeStates::CLOSING:
      {
        canvas.fillCircle(current_directionX, current_directionY, currentPupilRadius, PUPIL_COLOR);
        currentEylidState = min(float(millis() - time_at_state_change) / float(T_LENGTH_CLOSING), 1.0f);
        break;
      }
    case EyeStates::BLINK_CLOSING:
      {
        canvas.fillCircle(current_directionX, current_directionY, currentPupilRadius, PUPIL_COLOR);
        currentEylidState = min(float(millis() - time_at_state_change) / float(T_LENGTH_BLINK_CLOSING), 1.0f);
        break;
      }
  }
  drawEylids();

  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), canvas.width(), canvas.height());
  time_at_last_draw = millis();
}

//amt 0 = eylids open, amt 1 = eylids closed, lerp in between
void Eye::drawEylids() {
  setFrownAmt(moodFrown());
  float amtActual = max(frownAmt, currentEylidState);

  float offset = constrain(float(current_directionY - tft.height() / 2) / float(tft.height()), -EYLID_MAX_OFFSET_Y, EYLID_MAX_OFFSET_Y);
  //Upper eylid
  canvas.fillRect(0, 0, tft.width(), round(float(tft.height()) * (amtActual * 0.5 + offset)), EYLID_COLOR);
  //Lower eylid
  canvas.fillRect(0, tft.height() - round(float(tft.height()) * (amtActual * 0.5 - offset)), tft.width(), round(float(tft.height()) * (amtActual * 0.5 - offset)), EYLID_COLOR);
}


void Eye::drawTest() {
  // 1. Tyhjennetään näyttö (0 = musta)
  tft.fillScreen(0);

  // Määritetään väri (käytetään arvoa 1, tai esim. ST7735_WHITE jos käytössä on värinäyttö)
  uint16_t color = WHITE;

  // --- AKSELIT JA NUOLET ---
  // X-akseli (vaakaviiva vasemmasta yläkulmasta oikealle)
  tft.drawLine(0, 0, tft.width() - 5, 0, RED);
  // X-akselin nuolenpää (osoittaa oikealle, positiivinen X)
  tft.drawLine(tft.width() - 5, 0, tft.width() - 10, 4, BLUE);
  tft.setCursor(tft.width() - 12, 8);
  tft.print("+X");

  // Y-akseli (pystyviiva vasemmasta yläkulmasta alas)
  tft.drawLine(0, 0, 0, tft.height() - 5, color);
  // Y-akselin nuolenpää (osoittaa alas, positiivinen Y)
  tft.drawLine(0, tft.height() - 5, 4, tft.height() - 10, color);
  tft.setCursor(8, tft.height() - 12);
  tft.print("+Y");


  // --- TEKSTIN KESKITTÄMINEN ---
  String text = "Eye: " + name;

  int16_t x1, y1;
  uint16_t textWidth, textHeight;

  // Lasketaan tekstin viemä tila koordinaateissa (0,0)
  tft.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);

  // Lasketaan keskikohta vähentämällä näytön puolivälistä tekstin puolikas koko
  int16_t centerX = (tft.width() - textWidth) / 2;
  int16_t centerY = (tft.height() - textHeight) / 2;

  // Asetetaan kursori ja tulostetaan keskitetty teksti
  tft.setCursor(centerX, centerY);
  tft.print(text);
}


void Eye::setPupilRadius(float newRad) {
  // Clamp input to [0, 1]
  if (newRad < 0.0f) newRad = 0.0f;
  if (newRad > 1.0f) newRad = 1.0f;

  const int screenH = tft.height();

  int minRadius = (int)(MIN_PUPIL_RADIUS * screenH);
  int maxRadius = (int)(MAX_PUPIL_RADIUS * screenH);

  // Linear interpolation between min and max
  desiredPupilRadius = int(float(minRadius) + float(maxRadius - minRadius) * newRad);
}

void Eye::setFrownAmt(float newFrownAmt) {
  frownAmt = newFrownAmt;
}
