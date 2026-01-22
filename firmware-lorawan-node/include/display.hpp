#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Oled dimensions
#define SCREEN_WIDTH   128  
#define SCREEN_HEIGHT  64   

// I2C address and pins
#define SCREEN_ADDRESS 0x3C 
#define OLED_RESET     16   


class Display{
    TwoWire* _wire;
    Adafruit_SSD1306* _oled;
    uint8_t _sda;
    uint8_t _scl;
public:
    Display(TwoWire* Wire = &Wire): _wire(Wire) {
        _oled = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, _wire, OLED_RESET);
    }
    void setCommunicationPins(uint8_t sda, uint8_t scl);
    bool configure();
    void clear();
    void setCursor(int16_t x, int16_t y);
    void print(const String &text, uint8_t size);
    void println(const String &text, uint8_t size);
    int16_t width();
    int16_t height();
};


#endif // DISPLAY_HPP