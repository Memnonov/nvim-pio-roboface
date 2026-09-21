#ifndef SPRITES_H
#define SPRITES_H

#include <Arduino.h>
#include <Adafruit_GFX.h>

// ======================================================
// BASIC SPRITE STRUCTURE
// ======================================================

struct Sprite {
    const uint16_t* image;  // bitmap pixel data (RGB565)
    uint16_t w;             // width
    uint16_t h;             // height
};

void drawRGBBitmapTransparent(
    Adafruit_GFX &tft,
    int16_t x, int16_t y,
    const uint16_t *bitmap,
    int16_t w, int16_t h
);


static inline uint16_t tint565(
    uint16_t color,
    uint16_t tintColor,
    uint8_t amount);


void drawRGBBitmapTinted(
    Adafruit_GFX &canvas,
    int16_t x,
    int16_t y,
    const uint16_t *bitmap,
    int16_t w,
    int16_t h,
    uint16_t tintColor,
    uint8_t tintAmount);

// ======================================================
// EYE SPRITES
// ======================================================

// SCLERA / BASE
extern const uint16_t sprite_eye_sclera[];
extern const Sprite SPRITE_EYE_SCLERA;

// IRIS
extern const uint16_t sprite_eye_iris[];
extern const Sprite SPRITE_EYE_IRIS;


// UI SPRITES
extern const uint16_t sprite_question_mark[];
extern const Sprite SPRITE_QUESTION_MARK;

extern const uint16_t sprite_exclamation[];
extern const Sprite SPRITE_EXCLAMATION;

#endif