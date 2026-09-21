//<3 https://www.youtube.com/watch?v=IiZl3p-ZohM <3

#ifndef Robot_Eye_h
#define Robot_Eye_h


#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include "Sprites.h"

//Display specific basic colors, when changing displays, simply replace the hardware-specific colors here
//(Fairly certain that everything else works just fine)
#define BLACK     ST77XX_BLACK
#define BLUE      ST77XX_BLUE
#define RED       ST77XX_RED
#define GREEN     ST77XX_GREEN
#define CYAN      ST77XX_CYAN
#define MAGENTA   ST77XX_MAGENTA
#define YELLOW    ST77XX_YELLOW
#define WHITE     ST77XX_WHITE

// ==Code goes here==


// Different moods are represented by this enum
enum class Moods{
  NEUTRAL,
  HAPPY,
  SURPRISED,
  CONFUSED,
  SAD
};

enum class EyeStates{
  CLOSING, //decaying state, that is active for T_LENGTH_STATE_CLOSING amt of milliseconds
  OPENING,
  BLINK_CLOSING, //Different from closing en the way that this will decay into opening the eyes
  CLOSED, // static state
  OPEN,
};

Moods getMood(String moodStr);

// Todo: set the (for the user) unnnecessary variables as private
// Todo: Add support for image eyes/gifs or similair 
class Eye { 
  public:
    static const int IRIS_RADIUS = 60;
    
    int desiredPupilRadius;
    static constexpr float MAX_PUPIL_RADIUS = 0.4;//Proportional to the screen height //NOTE should be relative to the iris size, not the screen size
    static constexpr float MIN_PUPIL_RADIUS = 0.1;//Proportional to the screen height
    static const uint16_t PUPIL_COLOR = BLACK;
    
    static const uint16_t SCLERA_COLOR = WHITE;
    
    static const uint16_t EYLID_COLOR = BLACK;
    static constexpr float EYLID_MAX_OFFSET_Y = 0.5; //how far off the centerline of the eyes the eylids can move
    float frownAmt; //How much the eylids cover the eyes, 0.0 is fully open, 1.0 is fully closed
    float currentEylidState;

    String name;
    EyeStates current_state;

    //NOTE: these two will break if the system is left running for an extended period, idk how long it is 
    //      but it shouldn't be an issue under normal operations.

    Moods current_mood;
    float desired_directionX;
    float desired_directionY;
    
    Adafruit_ST7735& tft;
    GFXcanvas16& canvas;
  private:
    unsigned long time_at_last_draw; // the time at the last point where the 
    unsigned long time_at_state_change;//If the state "decays" like an animation which gets run through, this will be used to tell how long the animation has been playing for 
    int current_directionX;
    int current_directionY;
    int currentPupilRadius;

    // in millis
    const unsigned long T_LENGTH_CLOSING = 100;  
    const unsigned long T_LENGTH_OPENING = 100;  
    const unsigned long T_LENGTH_BLINK_CLOSING = 100;  
    
    const EyeStates CLOSING_DECAY_INTO = EyeStates::CLOSED;
    const EyeStates OPENING_DECAY_INTO = EyeStates::OPEN;
    const EyeStates BLINK_CLOSING_DECAY_INTO = EyeStates::OPENING;
  public:
  // Konstruktori oletusarvolla // NOTE: the default pin values won't work
  Eye( Adafruit_ST7735& tftObj, GFXcanvas16& canvasObj, String eyeName);

  
  // NOTE: the basic colors are defined at beginning of code
  uint16_t moodColor();
  float moodFrown();

  void setDirection(float desired_directionX_new, float desired_directionY_new);
  void setState(EyeStates newState);
  void changeMood(Moods newMood);
  void setPupilRadius(float newRad);
  void setFrownAmt(float newFrownAmt);
  int16_t laetWidth();
  int16_t getHeight();
  // Show eye needs to be in the draw loop
  void showEye();
  void drawTest();

  private:
  // checks if the current state (mainly animations) should decay into a static state
  void checkDecayState();
  void drawEylids();

};



// ==================

#endif