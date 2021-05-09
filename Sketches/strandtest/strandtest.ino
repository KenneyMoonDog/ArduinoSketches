// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    6

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 5

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

long red = 0; // or 65535
long pink = 65536*5/6;
long blue = 65536*2/3;
long teal = 65536/2;
long green = 65536/3;
long yellow = 65536/6;

// setup() function -- runs once at startup --------------------------------

void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(60); // Set BRIGHTNESS to about 1/5 (max = 255)
}


// loop() function -- runs repeatedly as long as board is on ---------------

void loop() {
  // Fill along the length of the strip in various colors...
  //colorWipe(strip.Color(255,   0,   0), 50); // Red
  //colorWipe(strip.Color(  0, 255,   0), 50); // Green
  //colorWipe(strip.Color(  0,   0, 255), 50); // Blue

  // Do a theater marquee effect in various colors...
  //theaterChase(strip.Color(127, 127, 127), 50); // White, half brightness
  //theaterChase(strip.Color(127,   0,   0), 50); // Red, half brightness
  //theaterChase(strip.Color(  0,   0, 127), 50); // Blue, half brightness

  //rainbow(5);             // Flowing rainbow cycle along the whole strip
  //theaterChaseRainbow(50); // Rainbow-enhanced theaterChase variant

  warp_cruise_2();
}


// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

void warp_cruise_4(){

  long hue[5] = {blue, blue, blue, blue, blue}; //0 (red) to 65535 (red)
  byte sat[5] = {50, 255, 100, 230, 20}; //(0-255) grey to full color 
  byte bright[5] = {60, 60, 60, 60, 60}; //(0-255) black to full bright
  byte inout[5] = {1,0,1,0,1}; //0 or 1 

  //red = 0 or 65535
  //pink = 65536*5/6
  //blue = 65536*2/3
  //teal = 65536/2
  //green = 65536/3
  //yellow = 65536/6
  
  //uint32_t rgbcolor = strip.gamma32(strip.ColorHSV(hue, sat, bright));
  //strip.setPixelColor(nPix, strip.gamma32(strip.ColorHSV(strip.ColorHSV(hue, sat, bright));

  //seed hue with blue with various levels of saturation and inout
  

while(1) {

    for (int nPix = 0; nPix < 5; nPix++){
      if (nPix == 0){
        if (primaryDirection == 1){
           check = primaryPixelHue + increment;
           if (check > endPixelHue) {
             primaryPixelHue = primaryPixelHue - increment;
             primaryDirection = 0;
           }
           else {
             primaryPixelHue = check;
           }
         }
         else {
           check = primaryPixelHue - increment;
           if (check < beginPixelHue) {
             primaryPixelHue = primaryPixelHue + increment;
             primaryDirection = 1;
           }
           else {
             primaryPixelHue = check;
           }       
         }
       }
       
       strip.setPixelColor(nPix, strip.gamma32(strip.ColorHSV(primaryPixelHue + ( nPix*750))));
    }
    
    strip.show(); // Update strip with new contents
    delay(10);  // Pause for a moment
  }  //end while  
}
  /*long beginPixelHue = 65536/2;
  long endPixelHue = (5*65536)/6;
  int increment = 256;
  int leadFactor = 750;
  long primaryPixelHue = beginPixelHue;
  byte primaryDirection;
  long check = 0;

  while(1) {

    for (int nPix = 0; nPix < 5; nPix++){
      if (nPix == 0){
        if (primaryDirection == 1){
           check = primaryPixelHue + increment;
           if (check > endPixelHue) {
             primaryPixelHue = primaryPixelHue - increment;
             primaryDirection = 0;
           }
           else {
             primaryPixelHue = check;
           }
         }
         else {
           check = primaryPixelHue - increment;
           if (check < beginPixelHue) {
             primaryPixelHue = primaryPixelHue + increment;
             primaryDirection = 1;
           }
           else {
             primaryPixelHue = check;
           }       
         }
       }
       
       strip.setPixelColor(nPix, strip.gamma32(strip.ColorHSV(primaryPixelHue + ( nPix*750))));
    }
    
    strip.show(); // Update strip with new contents
    delay(10);  // Pause for a moment
  }  //end while 
  
} */

void warp_cruise_3(){

  long beginPixelHue = 65536/2;
  long endPixelHue = (5*65536)/6;
  int increment = 256;
  int leadFactor = 750;
  long currentPixelHue[5] = {0,0,0,0,0};
  byte currentDirection[5] = {1,1,1,1,1};
  long check = 0;

  for (int n = 0; n < 5; n++){
    currentPixelHue[n] = beginPixelHue + (n*leadFactor);
    currentDirection[n] = 1;
  }

  while(1) {

    for (int nPix = 0; nPix < 5; nPix++){
      if (nPix == 0){
        if (currentDirection[nPix] == 1){
           check = currentPixelHue[nPix] + increment;
           if (check > endPixelHue) {
             currentPixelHue[nPix] = currentPixelHue[nPix] - increment;
             currentDirection[nPix] = 0;
           }
           else {
             currentPixelHue[nPix] = check;
           }
         }
         else {
           check = currentPixelHue[nPix] - increment;
           if (check < beginPixelHue) {
             currentPixelHue[nPix] = currentPixelHue[nPix] + increment;
             currentDirection[nPix] = 1;
           }
           else {
             currentPixelHue[nPix] = check;
           }       
         }
       }
       else {
         currentPixelHue[nPix] = currentPixelHue[nPix-1] + (nPix*leadFactor);
       }
       
       strip.setPixelColor(nPix, strip.gamma32(strip.ColorHSV(currentPixelHue[nPix])));
    }
    
    strip.show(); // Update strip with new contents
    delay(10);  // Pause for a moment
  }  //end while
}

void warp_cruise_2(){

  long beginPixelHue = 65536/2;
  long endPixelHue = (5*65536)/6;
  int increment = 256;
  int leadFactor = 750;
  long primaryPixelHue = beginPixelHue;
  byte primaryDirection;
  long check = 0;

  while(1) {

    for (int nPix = 0; nPix < 5; nPix++){
      if (nPix == 0){
        if (primaryDirection == 1){
           check = primaryPixelHue + increment;
           if (check > endPixelHue) {
             primaryPixelHue = primaryPixelHue - increment;
             primaryDirection = 0;
           }
           else {
             primaryPixelHue = check;
           }
         }
         else {
           check = primaryPixelHue - increment;
           if (check < beginPixelHue) {
             primaryPixelHue = primaryPixelHue + increment;
             primaryDirection = 1;
           }
           else {
             primaryPixelHue = check;
           }       
         }
       }
       
       strip.setPixelColor(nPix, strip.gamma32(strip.ColorHSV(primaryPixelHue + ( nPix*750))));
    }
    
    strip.show(); // Update strip with new contents
    delay(10);  // Pause for a moment
  }  //end while
}

void warp_cruise_1(){

  long beginPixelHue = 65536/2;
  long endPixelHue = (5*65536)/6;
  int increment = 256;
  long currentPixelHue[5] = {0,0,0,0,0};
  byte currentDirection[5] = {1,1,1,1,1};
  long check = 0;

  for (int n = 0; n < 5; n++){
    currentPixelHue[n] = beginPixelHue + ( n*750);
    currentDirection[n] = 1;
  }

  while(1) {

    for (int nPix = 0; nPix < 5; nPix++){
       if (currentDirection[nPix] == 1){
          check = currentPixelHue[nPix] + increment;
          if (check > endPixelHue) {
             currentPixelHue[nPix] = currentPixelHue[nPix] - increment;
             currentDirection[nPix] = 0;
          }
          else {
             currentPixelHue[nPix] = check;
          }
       }
       else {
          check = currentPixelHue[nPix] - increment;
          if (check < beginPixelHue) {
             currentPixelHue[nPix] = currentPixelHue[nPix] + increment;
             currentDirection[nPix] = 1;
          }
          else {
             currentPixelHue[nPix] = check;
          }       
       }
       strip.setPixelColor(nPix, strip.gamma32(strip.ColorHSV(currentPixelHue[nPix])));
    }
    
    strip.show(); // Update strip with new contents
    delay(10);  // Pause for a moment
  }  //end while
}
  
// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}
