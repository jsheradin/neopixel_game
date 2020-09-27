//Single button game for adafruit neopixel
//Tested with neopixel rings and an ATtiny85

#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define HIGH_SCORE_ADDR 0 //eeprom address for high level tracking

#define LED_PIN 0
#define BUTTON_PIN 2
#define NUMPIXELS 32

#define BRIGHTNESS 15 //background brightness
#define BRIGHT_TARGET 25 //offset from background
#define BRIGHT_PIX 25 //offset from background

#define START_SPEED 100 //width of target
#define START_WIDTH 5 //delay before LED moves
#define SPEED_INC 10 //% speed change per level

int pixSpeed = START_SPEED; 
int targetSize = START_WIDTH;
int highScore = EEPROM.read(HIGH_SCORE_ADDR);
int numLevel = 0;

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
int pixelOrder[NUMPIXELS] = {25, 24, 23, 22, 21, 20, 19, 18, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 17, 16, 31, 30, 29, 28, 27, 26};

void setup() {
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif
  
  pixels.begin();
}

//main game loop
void loop() {
  int pixelLocation = 0;
  int targetLocation = random(0, NUMPIXELS);
  int targets[targetSize];
 
  //random colors
  uint32_t colorTarget = pixels.ColorHSV(random(65535), random(255), BRIGHTNESS+BRIGHT_TARGET);
  uint32_t colorPix = pixels.ColorHSV(random(65535), random(255), BRIGHTNESS+BRIGHT_PIX);
  uint32_t colorBackground = pixels.ColorHSV(random(65535), random(255), BRIGHTNESS);

  // level loop
  while(true) {
    //draw field
    pixels.clear();
    pixels.fill(colorBackground, 0); //background
    for(int i=0; i<targetSize; i++){ //target
      int wrappedLocation = (i+targetLocation)%NUMPIXELS; //
      targets[i] = wrappedLocation; //add to list for hit check
      pixels.setPixelColor(pixelOrder[wrappedLocation], colorTarget); //fill with wrap
    }
    pixels.setPixelColor(pixelOrder[pixelLocation], colorPix); //moving pixel
    pixels.show();

    //check every 1ms (ish) if button has been pressed
    for(int i=0; i<pixSpeed; i++) {
      if(digitalRead(BUTTON_PIN) == HIGH){ //button pressed
        //in list of target pixels
        for(int i=0; i<targetSize; i++){
          if(targets[i] == pixelLocation){
            pixels.fill(pixels.Color(0, BRIGHTNESS, 0), 0); //win screen
            pixels.show();
            delay(1000);
            int speedUp = pixSpeed * SPEED_INC / 100; //apply speed increase
            pixSpeed -= speedUp;
            if(pixSpeed <= 0){ //keep it from going negative delay
              pixSpeed = 1;
            }
            numLevel++;
            return;
          }
        }
        //not in list of target pixels
        pixels.fill(pixels.Color(BRIGHTNESS, 0, 0), 0); //lose screen
        pixels.show();
        delay(1000);
        
        //new high score
        if(numLevel>highScore){
          EEPROM.write(HIGH_SCORE_ADDR, numLevel);
          //rainbow snake until power cycled
          while(true){
            for(int j=0; j<NUMPIXELS; j++){
              for(int i=0; i<NUMPIXELS; i++){
               pixels.setPixelColor(pixelOrder[(i+j)%NUMPIXELS], pixels.ColorHSV(65535/NUMPIXELS*i, 255, BRIGHTNESS));
              }
             pixels.show();
             delay(100);
            }
          }
        }
        //reset level
        pixSpeed = START_SPEED;
        numLevel = 0;
        return;
      }
      delay(1);
    }
    
    pixelLocation++; //move pixel
    pixelLocation %= NUMPIXELS; //wrap if out of bounds
  }
}
