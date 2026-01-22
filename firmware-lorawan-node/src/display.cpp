#include "display.hpp"

void Display::setCommunicationPins(uint8_t sda, uint8_t scl) {
    _sda = sda;
    _scl = scl;
}

bool Display::configure(){
    if(! _wire->begin(_sda, _scl)){
        return false;
    }
    if(! _oled->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
        return false;
    }
    _oled->display();
    vTaskDelay(2000);
    _oled->clearDisplay();
    return true;
}

void Display::clear(){
    _oled->clearDisplay();
}

void Display::setCursor(int16_t x, int16_t y){
    _oled->setCursor(x, y);
}

void Display::print(const String &text, uint8_t size){
    _oled->setTextSize(size);
    _oled->setTextColor(SSD1306_WHITE);
    _oled->print(text);
    _oled->display();
}

void Display::println(const String &text, uint8_t size){
    _oled->setTextSize(size);
    _oled->setTextColor(SSD1306_WHITE);
    _oled->println(text);
    _oled->display();
}

int16_t Display::width(){
    return _oled->width();
}

int16_t Display::height(){
    return _oled->height();
}