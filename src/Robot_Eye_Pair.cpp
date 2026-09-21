#include "Robot_Eye_Pair.h"

EyePairState StringToEyePairState(const char* state) {

  if (state == nullptr) {
    return EyePairState::IDLE; // default fallback
  }

  if (strcmp(state, "IDLE") == 0) {
    return EyePairState::IDLE;
  }
  else if (strcmp(state, "ACTIVE") == 0) {
    return EyePairState::ACTIVE;
  }
  else if (strcmp(state, "DEBUG") == 0) {
    return EyePairState::DEBUG;
  }
  else if (strcmp(state, "CONV") == 0) {
    return EyePairState::CONV;
  }

  // fallback if unknown string
  return EyePairState::IDLE;
}


// Konstruktori: Alustaa luokan Eye-viittaukset alustuslistassa
Eye_Pair::Eye_Pair(Eye& Eye1, Eye& Eye2) : Eye1(Eye1), Eye2(Eye2), timeLastBlinked(0), prevDirX(0.0), prevDirY(0.0), timeAtLastGlance(0) {
  this->current_mood = Moods::NEUTRAL; //Eyes are set to Neutral by default
  State = EyePairState::IDLE;
}

void Eye_Pair::blink() {
  Eye1.setState(EyeStates::BLINK_CLOSING);
  Eye2.setState(EyeStates::BLINK_CLOSING);
  timeLastBlinked = millis();
}

void Eye_Pair::winkEye1() {
  Eye1.setState(EyeStates::BLINK_CLOSING);
}

void Eye_Pair::winkEye2() {
  Eye2.setState(EyeStates::BLINK_CLOSING);
}

void Eye_Pair::look(float DirX, float DirY) {
  Eye1.setDirection(DirX, DirY);
  Eye2.setDirection(DirX, DirY);
  prevDirX = DirX;
  prevDirY = DirY;
}

void Eye_Pair::lookEye1(float DirX, float DirY) {
  Eye1.setDirection(DirX, DirY);
}

void Eye_Pair::lookEye2(float DirX, float DirY) {
  Eye2.setDirection(DirX, DirY);
}

void Eye_Pair::glance(){
  
  
  float x = randomBell();
  float y = randomBell();
  glance(x, y);
}

void Eye_Pair::glance(float gDirX, float gDirY){
  timeAtLastGlance = millis();
  
  Eye1.setDirection(max(min(prevDirX + gDirX, 1.0f), 0.0f), max(min(prevDirY + gDirY, 1.0f), 0.0f));//Set direction "manually" so prevDir doesn't get overwritten
  Eye2.setDirection(max(min(prevDirX + gDirX, 1.0f), 0.0f), max(min(prevDirY + gDirY, 1.0f), 0.0f));  
}

void Eye_Pair::setPupilRadius(float newRad){
  Eye1.setPupilRadius(newRad);
  Eye2.setPupilRadius(newRad);
}

void Eye_Pair::setMood(Moods Mood) {
  Eye1.changeMood(Mood);
  Eye2.changeMood(Mood);
}

void Eye_Pair::showEyes(){
  if(State == EyePairState::IDLE) idleAnimation();
  else if(State == EyePairState::CONV) conversationAnimation();
  else if(State == EyePairState::DEBUG){
    Eye1.drawTest();
    Eye2.drawTest();
    return;
  }

  // Check if glance is over
  if(glanceDuration + timeAtLastGlance < millis()){
    look(prevDirX, prevDirY);
  }

  Eye1.showEye();
  Eye2.showEye();
  
}



void Eye_Pair::setIdle(bool idleState){
  if(idleState){
    State = EyePairState::IDLE;
    return;
  }
  if(!idleState) State = EyePairState::ACTIVE;
}

void Eye_Pair::setEyePairState(EyePairState newState){
  State = newState;
}

void Eye_Pair::idleAnimation(){
  if(random(0,4 * expectedFps) == 1){
    this->look(float(random(0, 100))*0.01, float(random(0, 100))*0.01);
  }

  randomBlink();
  if(random(0,30 * expectedFps) == 1){
    this->winkEye1();
  }
}


//TODO: Eye pair conversation animation, (looks, glances, frowns, etc.)
void Eye_Pair::conversationAnimation(){
  randomBlink();
  if(random(0, expectedFps*10) == 1){
    glance();
  }
}



void Eye_Pair::randomBlink(){
  if(timeLastBlinked + avgBlinkDelay < millis() && random(0,expectedFps*3) == 1){
    this->blink();
  }
}

float Eye_Pair::randomBell()
{
    float a = (float)random(0, 10000) / 10000.0f;
    float b = (float)random(0, 10000) / 10000.0f;

    return a - b;  // Range [-1,1]
}
