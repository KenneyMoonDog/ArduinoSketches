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

const long red = 65535;
const long pink = 65536*5/6;
const long blue = 65536*2/3;
const long teal = 65536/2;
const long green = 65536/3;
const long yellow = 65536/6;

long colorSet[6] = {red, pink, blue, teal, green, yellow};

long hue[5] = {blue, blue, blue, blue, blue}; //0 (red) to 65535 (red)
byte sat[5] = {50, 255, 100, 230, 20}; //(0-255) grey to full color 
byte bright[5] = {25, 40, 20, 60, 10}; //(0-255) black to full bright
byte inout[5] = {1,0,1,0,1}; //0 or 1 
byte satStep[5] =  {1,1,1,1,1};
byte brightStep[5] = {1,2,1,2,1};
byte maxBright = 150;
byte minBright = 1;
  
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
  //strip.setBrightness(60); // Set BRIGHTNESS to about 1/5 (max = 255)
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
  warp_start();
  warp_cruise_4();
  //warp_decell();
  warp_decell_2();
  exit;
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

void warp_start() {

  //uint32_t rgbcolor = strip.gamma32(strip.ColorHSV(hue, sat, bright));
  //strip.setPixelColor(nPix, strip.gamma32(strip.ColorHSV(strip.ColorHSV(hue, sat, bright));

  for(byte tempBright=minBright; tempBright<maxBright; tempBright++) {
    long tempHue = colorSet[random(4)]; 
    for (int nPix = 0; nPix < 5; nPix++){
      strip.setPixelColor(nPix, strip.gamma32(strip.ColorHSV((tempHue+(nPix+2500)), 255, tempBright)));
      //strip.setPixelColor(nPix, strip.gamma32(strip.ColorHSV((colorSet[random(4)]), 255, tempBright)));
    } //end for
    
    strip.show(); // Update strip with new contents
    delay(25);  // Pause for a moment
  }  

  for(byte decreaseBright=(maxBright+50); decreaseBright>(maxBright/2); decreaseBright--) {
    for (int npPix = 0; npPix < 5; npPix++){
      bright[npPix]=decreaseBright;
      strip.setPixelColor(npPix, strip.gamma32(strip.ColorHSV(blue, 255, decreaseBright)));
    } //end for
    
    strip.show(); // Update strip with new contents
    delay(25);
  }

  //long hue[5] = {blue, blue, blue, blue, blue}; //0 (red) to 65535 (red)
  //byte sat[5] = {50, 255, 100, 230, 20}; //(0-255) grey to full color 
  //byte bright[5] = {25, 40, 20, 60, 10}; //(0-255) black to full bright
  //byte inout[5] = {1,0,1,0,1}; //0 or 1 
  //byte satStep[5] =  {1,1,1,1,1};
  //byte brightStep[5] = {1,2,1,2,1};
}

void resetHue(long color) {
  for (byte hIndex = 0; hIndex < 5; hIndex++) {
    hue[hIndex] = color; 
  }
}

void warp_cruise_4(){

  byte tempSat;
  byte tempBright;

  resetHue(blue);

  //uint32_t rgbcolor = strip.gamma32(strip.ColorHSV(hue, sat, bright));
  //strip.setPixelColor(nPix, strip.gamma32(strip.ColorHSV(strip.ColorHSV(hue, sat, bright));
  long startTime = millis();
  while((millis() - startTime < 10000)) {

    for (int nPix = 0; nPix < 5; nPix++){
      if (inout[nPix] == 1){
        tempBright = bright[nPix] + brightStep[nPix];
        if (tempBright >= maxBright) {
          inout[nPix] = 0;
          bright[nPix] = maxBright;
          brightStep[nPix] = random(2, 3);
        }
        else {
           bright[nPix] = tempBright;
        }
      }
      else {
        tempBright = bright[nPix] - brightStep[nPix];
        if (tempBright <= minBright) {
          inout[nPix] = 1;
          bright[nPix] = minBright;
          brightStep[nPix] = random(1, 3);
        }
        else {
          bright[nPix] = tempBright;
        }
      }       
      strip.setPixelColor(nPix, strip.gamma32(strip.ColorHSV(hue[nPix], 255, bright[nPix])));
    } //end for
    
    strip.show(); // Update strip with new contents
    delay(10);  // Pause for a moment
  }  //end while  
}

void warp_decell_2(){
  
  int tempBright = maxBright;

  long startColor = blue;
  long endColor = red;

  float colorDelta = (abs(endColor - startColor))/5;
 
  //long colorSet[6] = {red, pink, blue, teal, green, yellow};
  
  long startTime = millis();
  byte deadPos = 4; 
  byte deadPosCount = 0;
  int deadPosCountLimit = 1;
  
  while((millis() - startTime < 10000) && (tempBright > 10)) {

       for (byte pload = 0; pload < 5; pload++){
         hue[pload] = startColor - (4-pload) * colorDelta;

         if (pload == deadPos) {
           bright[pload] = 0;
         }
         else {
           bright[pload] = tempBright; // - ((4-pload) * brightDelta);
         }
         
         strip.setPixelColor(pload, strip.gamma32(strip.ColorHSV(hue[pload], 255, bright[pload])));
       }

       //if (deadPosCount++ > deadPosCountLimit ){
       //  deadPosCount = 0;
         if (deadPos > 0) {
            deadPos--;
         }
         else {
            deadPos = 4;
         }
       //}
       
       strip.show(); // Update strip with new contents
       
       ( tempBright > 0 ) ? tempBright-=1 : tempBright = 0;
       startColor += 100;
       endColor -=100; 
       colorDelta = (abs(endColor - startColor))/4;
       delay(20);
  } //end while 
}

void warp_decell(){

  int tepBright = maxBright;
  long brightPosCounter = millis(); 
  long startTime = millis();
  byte brightPos = 0; 
  
  while((millis() - startTime < 10000) && (tepBright > 0)) {
      if ( (millis() - brightPosCounter) > 25){
        if (brightPos > 0) {
           brightPos--;
        }
        else {
           brightPos = 4;
        }
        brightPosCounter = millis();

         //int slope = tepBright/4;
         switch ( brightPos ){
           case 0:
              bright[0] = tepBright;
              bright[1] = 0.6 * tepBright;
              bright[2] = 0.3 * tepBright;
              bright[3] = 0.15 * tepBright;
              bright[4] = 0;
              break;
           case 1:
              bright[0] = 0.6 * tepBright;
              bright[1] = tepBright;
              bright[2] = 0.6 * tepBright;
              bright[3] = 0.3 * tepBright;
              bright[4] = 0.1 * tepBright;
              break;  
           case 2:
              bright[0] = 0.3 * tepBright;
              bright[1] = 0.6 * tepBright;
              bright[2] = tepBright;
              bright[3] = 0.6 * tepBright;
              bright[4] = 0.3* tepBright;
              break;
           case 3:
              bright[0] = 0.15 * tepBright;
              bright[1] = 0.3 * tepBright;
              bright[2] = 0.6 * tepBright;
              bright[3] = tepBright;
              bright[4] = 0.6 * tepBright;
              break;
           case 4:
              bright[0] = 0;
              bright[1] = 0.15 * tepBright;
              bright[2] = 0.3 * tepBright;
              bright[3] = 0.6 * tepBright;
              bright[4] = tepBright;
              break;
         }

         for (byte pload = 0; pload < 5; pload++){
           strip.setPixelColor(pload, strip.gamma32(strip.ColorHSV(hue[pload], 255, bright[pload])));
         }
         
         strip.show(); // Update strip with new contents
         tepBright -=1;
      } //end if
  } //end while 
}

void warp_stop(){
  
}

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
