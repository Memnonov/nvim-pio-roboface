#ifndef Robot_Eye_Pair_h
#define Robot_Eye_Pair_h

#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include "Robot_Eye.h"


enum class EyePairState{
  IDLE, 
  ACTIVE,
  DEBUG,
  CONV
};

EyePairState StringToEyePairState(const char* state);


// A pair of eyes,
// Handles api stuffs
class Eye_Pair {
  // references to the two Eye objects controlled by the Eye_Pair object
  Eye& Eye1; 
  Eye& Eye2;

  // Other variables
  Moods current_mood;
  int expectedFps = 30;
private: 
  float prevDirX;
  float prevDirY;
  unsigned long timeAtLastGlance;
  static const unsigned long glanceDuration = 500;

  float prevPupilSize;
  unsigned long timeLastBlinked;
  static const unsigned long avgBlinkDelay = 1000;//Time between blinks, a *suggestion* more than a hard rule
public:
  EyePairState State;
  Eye_Pair(Eye& Eye1, Eye& Eye2);

  void blink();
  void winkEye1();//True for 2 false for 1
  void winkEye2();
  void look(float DirX, float DirY);
  void lookEye1(float DirX, float DirY);
  void lookEye2(float DirX, float DirY);
  void glance();
  void glance(float gDirX, float gDirY);//This is like a temporary "look this much more in that direction" command
  void setPupilRadius(float newRad);
  void setMood(Moods Mood);// Note that valid values for moods are listed in the defines
  void showEyes();
  void setIdle(bool idleState);
  void setEyePairState(EyePairState);
  void setPupilSize(float newPupilSize);
private: 
  void idleAnimation();
  void conversationAnimation();
  void randomBlink();
  float randomBell();//Approximates a bell curve (according to chatgpt)
  
};


#endif