/* ________________________________________________________________________________________
      
      Soundlevel meter
      Georges Hart
      februari 2019
   ________________________________________________________________________________________ */


//--------------------------- LIBRARIES ------------------------------------------

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>


//--------------------------- DEFINITIONS ------------------------------------------

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

#define INFO_SCREEN_DELAY 3000


//--------------------------- GLOBAL VARIABLES -------------------------------------

 
const int sampleWindow = 20;                              // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
bool info_show =false;


//--------------------------- SETUP -----------------------------------------------

void setup(void) {
Serial.begin(9600); // debug info ;-)
  
 u8g2.begin();
 splash();
 
 if (info_show == false){
    info();
 } 
}

// ------------------------- LOOP FUNCTION ---------------------------------------
                  
void loop(void) {

unsigned long startMillis= millis();                   // Start of sample window
float peakToPeak = 0;                                  // peak-to-peak level

unsigned int signalMax = 0;                            //minimum value
unsigned int signalMin = 4096;                         //maximum value
 
                                                          // collect data for 50 mS
 while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(A0);                             //get reading from microphone
      if (sample < 4096)                                  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;                           // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;                           // save just the min levels
         }
      }
   }
 
  peakToPeak = (signalMax - signalMin)*5;                    // max - min = peak-peak amplitude
  
  Serial.print(sample);Serial.print("     |   ");Serial.print(signalMin);Serial.print(" - ");Serial.print(signalMax);Serial.print(" = ");Serial.println(peakToPeak);
  
  
 //float db = map(peakToPeak,20,900,49.5,90);             //calibrate for deciBels
  float db = map(peakToPeak,20,4000,0,100);
   u8g2.clearBuffer();  
   u8g2.setFontMode(1);
   u8g2.setFont(u8g2_font_cu12_tr);
   u8g2.setCursor(0,0);                                //cursor to upper left
   u8g2.print(db);
                               

     title();
     
     u8g2.setCursor(14,14);
     u8g2.print("dB SPL");
     u8g2.print(" :");
     if(db <120){
        u8g2.print(db);
     }
     else{
        u8g2.print("overload");
      
     }
     footer();

    for(int x =5;x<114;x=x+6){                            //draw scale
      u8g2.drawLine(x, 32, x, 27);
    }

    u8g2.drawLine(95, 32, 95, 20);
    
   u8g2.drawRFrame(0, 32, 120, 20,6);       //draw outline of bar graph
   int r = map(db,0,120,1,120);                           //set bar graph for width of screen
   u8g2.drawRBox(1, 33, r, 18,6);           //draw bar graph with a width of r
   u8g2.sendBuffer();                                    //show all that we just wrote & drew
                                 //clear the display
 
}

//-------------------------- FUNCTIONS -------------------------------------------

                           

   
void splash(void){
       
  u8g2.clearBuffer();  
  u8g2.setFontMode(1);
  u8g2.setFont(u8g2_font_cu12_tr);

  u8g2.setCursor(0,15);
  u8g2.print(F("TFE - 2019"));
  u8g2.setCursor(0,30);
  u8g2.print(F("Calibration Checker"));
  u8g2.setCursor(32,60);
  u8g2.print(F("Prototype  GH"));
  u8g2.sendBuffer();
  delay(3000);
 
} 


bool info(void){
  u8g2.clearBuffer(); 
  // u8g2.drawFrame(0,0,128,64); 
  u8g2.setFontMode(1);
  u8g2.setFont(u8g2_font_6x12_mf);      
  u8g2.setCursor(1,15);
  //u8g2.print(F("Cette appareil mesure"));  // an other way to print a string to screen
  u8g2.drawStr(1,15,"Cette appareil mesure");
  u8g2.setCursor(1,25);
  u8g2.print(F("et memorise des"));
  u8g2.setCursor(1,35);
  u8g2.print(F("intensitees sonores"));
  u8g2.setCursor(1,55);
  u8g2.print(F("Patienter :-)  ..."));
  u8g2.sendBuffer();
  info_show=true;
  delay(5000);
}

void page1(void){
       
  u8g2.clearBuffer();  
  u8g2.setFontMode(1);
  u8g2.setFont(u8g2_font_cu12_tr);

  u8g2.setCursor(0,15);
  u8g2.print(F("Measuring"));
  u8g2.setCursor(0,30);
  u8g2.print(F("Sound intensities"));

  u8g2.sendBuffer();
  delay(2000);
 
}
void title(void){
  u8g2.setFontMode(1);
  u8g2.setFont(u8g2_font_cu12_tr);
 // u8g2.setCursor(12,15);
 // u8g2.print(F("Intensite Sonore"));
}  
void footer(void){
  u8g2.setFontMode(1);
  u8g2.setFont(u8g2_font_6x12_mf); 
  u8g2.setCursor(1,64);
  u8g2.print(F("12/04/2019: 95 dB SPL."));
} 

/* ---------- suppl. info u8gl library --------------------------------------------------
 *  
 *  https://github.com/olikraus/u8g2/wiki/u8g2reference
 *  https://github.com/olikraus/u8g2/wiki/fntlistall#all-u8g2-fonts-capital-a-height
 *  
 ---------------------------------------------------------------------------------------- */
