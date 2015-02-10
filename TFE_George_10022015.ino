/*

	*************************
	* Project : LCD Display *
	* Author  : George Hart *
	* Date    : 25 jan 2015 *
	*************************

 	Parts required:
 		 		
                16x2 LCD screen (LCM1602C)
                Contrast: 10 k.Ohm potentiometer
                Backlight: 220 Ohm
 		
                Switches Normal open
                10 K.Ohm Pull Up resistor
 
 */


// ----- Load Libraries -----
#include <LiquidCrystal.h>
#include <LcdBarGraph.h>
#include <EEPROM.h>

// ----- Definitions -----
#define debounce 200

const byte sw_select = 6; // Button: Menu select
const byte sw_enter  = 7; // Button: Menu enter
const byte sw_pushbtn= 8; // Button: Drukknop test
const float imp_c= (20/1023); // DC/Impedantie convertie

//----- LCD -----
byte lcdNumCols = 16; // -- number of columns in the LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // initialize lcd umbers of the interface pins
LcdBarGraph lbg(&lcd, lcdNumCols);  // -- creating Barggraph


//----- Variables -----
int selection = 0;
int temp=0;
int testinomloop= 0;
int sensorValue=0;
int storedintensity=0;
int impedance=0;

  
unsigned int sample;
const int sampleWindow = 50; // Sample window width in mS (50mS = 20Hz)


// -------------------- EEPROM write ------------------
char buffer[4]; // String format for clean LCD write
char buffer2[4];
char diff[4];
char sign[]="-";

int previous=0;
int intensity=0;
float difference=0;
// -------------------- Impedance ---------------------
char imp[4];

//------ EEPROM Integer to byte transform  ... (high x 256 + low) -----
byte w_lox;
byte w_hix;

// ------------------- Smoothing ----------------------
const int numReadings = 10;     // smoothing index

int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

//--------------------------------------------------------------------------------------------

void setup() {
 
  Serial.begin(9600);   debugging
  // Print a Welcome message to the LCD.
  
  lcd.begin(2, 16); //type LCD display
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("  AUDIOCHECKER  ");
  lcd.setCursor(0, 1); 
  lcd.print("GH      jan 2015");

  // set up the switch pin as an input
  pinMode(sw_select,INPUT);
  pinMode(sw_enter,INPUT);
  pinMode(sw_pushbtn,INPUT);
  
}


void loop(){ 
  
  
// -- Smoothinh : set buffer to 0
    for(int thisReading = 0; thisReading < numReadings; thisReading++) 
// ----- EEPROM Integer uitlezen -----
    previous= (EEPROM.read(1)*256)+(EEPROM.read(2));    
         
//----- check the status of the switches  -----
  while(selection<=6){
        
/* ----- ENTER function -----------------------------------------*/
     
    if(digitalRead(sw_enter) == LOW){
        delay(debounce); // debouncing switch
       
        lcd.setCursor(0, 1);
        lcd.print("Your choice = ");
        lcd.print(selection);     
        test(selection);    
    } 
   
/* ----- SELECTION functions ------------------------------------*/   
   
    if(selection==5){selection=0;} // loop     
     
    if(digitalRead(sw_select) == LOW){
        delay(debounce); // debouncing switch
         writeLCD(selection); // function call
         selection=selection+temp; // increase selection counter        
    }
     
/* ----- PUSH Buton Test ----------------------------------------*/ 
          
    if((testinomloop==2) && digitalRead(sw_pushbtn)==LOW){
         lcd.setCursor(4, 1);
         for(int f=0;f<8;f++){
           lcd.print(char(255)); // char(255) = Blokje -- ASCII
         }
         delay(debounce);  
         digitalWrite(sw_pushbtn,HIGH);
         lcd.setCursor(0, 1);
         lcd.print("                ");
         delay(debounce); 
    }
     
/* ----- Imedance test -------------------------------------------*/ 
       
     while(testinomloop==3){
          lcd.setCursor(0, 1);
          sensorValue = analogRead(A0);
          impedance= sensorValue/5; // /5 omdat 5 volt
          if(impedance < 200){
          sprintf(imp,"%02d",impedance);
          lcd.print("Impedance:");
          lcd.print(imp);
          lcd.print(" Ohm");
          delay(debounce); 
          
          }else{
            lcd.setCursor(0, 1);
           lcd.print("Impedance > 100 ");
          }
                    
          if(digitalRead(sw_select) == LOW){ // Verlaat impedantie meting;
           testinomloop=0;
           lcd.setCursor(0, 1);
           lcd.print("                ");
           break;
          }
     }
     
/* ----- SOUND LEVEL -----------------------------------------------*/
       
      while(testinomloop==1){
                   
      char buffer[4]; // schrijf op LCD in string formaat
      char buffer_ref[4];         
            
            
//----- Setup Microphone Sampling -----------------------------------*/

      unsigned long startMillis= millis(); // Start of sample window
      unsigned int peakToPeak = 0; // peak-to-peak level  
      unsigned int signalMax = 0;
      unsigned int signalMin = 1024;    
                   
       // collect data for 50 mS
       while (millis() - startMillis < sampleWindow){
           sample = analogRead(5);
           if (sample < 1024){ // toss out spurious readings    
              if (sample > signalMax){
                  signalMax = sample; // save just the max levels
                }else if (sample < signalMin){
                  signalMin = sample; // save just the min levels
              }
            }
       }
          
            peakToPeak = signalMax - signalMin; // max - min = peak-peak amplitude

//-----------------------  SMOOTHING ---------------------------            

            total= total - readings[index];  // subtract the last reading:     
            readings[index] = peakToPeak;    // read from the sensor:  
            total= total + readings[index];  // add the reading to the total:
            index = index + 1;               // advance to the next position in the array:  
          
            if (index >= numReadings)        // if we're at the end of the array...
              index = 0;                     // ...wrap around to the beginning:
            
            average = total / numReadings;   // calculate the average:
            delay(1);        // delay in between reads for stability
                      
                      
            intensity = (average)*4;// * 3.3) / 1024; // convert to volts
            
            // -- draw bar graph from the analog value readed
            lbg.drawValue( intensity, 1024);
            // -- do some delay: frequent draw may cause broken visualization
            delay(10);
            

            
            lcd.setCursor(0, 1); 
            sprintf(buffer2,"%04d",previous);
            lcd.print(buffer2);
            
            writedifference(previous, intensity);
            
            sprintf(buffer,"%04d",intensity);
            lcd.setCursor(12, 1); 
            lcd.print(buffer);
           
            // ----- Push both buttons to store -----         
            if((digitalRead(sw_enter)==LOW)&&(digitalRead(sw_select)==LOW)){
                         
                        lcd.clear();
                        lcd.setCursor(0, 0); 
                        lcd.print("EEPROM WRITE");
                        
                        w_lox=lowByte(intensity);
                        w_hix=highByte(intensity);
                        
                        EEPROM.write(1,w_hix);
                        EEPROM.write(2,w_lox);
                        
                        
                        // ----- EEPROM Integer uitlezen -----
                    
                        previous= (EEPROM.read(1)*256)+(EEPROM.read(2)); 
                        lcd.setCursor(0, 1);
                        lcd.print("Prev.value: ");
                        sprintf(buffer2,"%04d",previous);
                        lcd.print(buffer2);
                    
                    
                        delay(1500);
                        lcd.clear();
          
            }
            
            if(digitalRead(sw_select) == LOW){ // Verlaat impedantie meting;
               testinomloop=0;
               lcd.setCursor(0, 1);
               lcd.print("                ");
             break;
            }
      }

      
   }  

 }
//--------------------------------------------------------------------------------------------






//----- Functions -----
void writedifference(int previous, int intensity){
  
 int a= previous;
 int b = intensity;
 float x= (float)a;
 float y= (float)b;
 float diff=0;
 String verschil;
   
 int f=0;
               
             if(x>y){
              diff= (int)(100*((x-y)/x));           

                  if(diff < 10){
                    verschil="[OK]";
                    lcd.setCursor(7, 1);
                    lcd.print(verschil);
                  }else if(diff < 25){
                    verschil="-   ";
                    lcd.setCursor(7, 1);
                    lcd.print(verschil);
                  }else if(diff < 75){
                  //Serial.print("x :");Serial.print(x);Serial.print(" y :");Serial.print(y);Serial.print("                            ");Serial.println(diff);
                    verschil="--  ";
                    lcd.setCursor(7, 1);
                    lcd.print(verschil);
                  }else{
                    verschil="--- ";
                    lcd.setCursor(7, 1);
                    lcd.print(verschil);
                  }
              f=1;
              
              }else if(x<y) {
                  diff= (int)(100*((y-x)/x));
                  
                  if(diff < 10){
                    verschil="[OK]";
                    lcd.setCursor(7, 1);
                    lcd.print(verschil);
                  }else if(diff < 25){
                    verschil="+   ";
                    lcd.setCursor(7, 1);
                    lcd.print(verschil);
                  }else if(diff < 75){
                    verschil="++  ";
                    lcd.setCursor(7, 1);
                    lcd.print(verschil);
                  }else{
                    verschil="+++ ";
                    lcd.setCursor(7, 1);
                    lcd.print(verschil);
                }
              
              }else{
              f=0;
             }
}



int writeLCD(int x){
  int y=x;
          switch(y){
                  case 0:
                    lcd.clear();  
                    lcd.setCursor(0, 0);
                    lcd.print("Sound Level Test");
                    lcd.setCursor(0, 1);
                    lcd.print("Prev.value: ");
                    sprintf(buffer2,"%04d",previous);
                    lcd.print(buffer2);
                    selection=selection+1;
                    delay(debounce);
                    break;
                  
                  case 1:
                    lcd.clear();  
                    lcd.setCursor(0, 0);
                    lcd.print("Push Button");
                    selection=selection+1;
                    delay(debounce);
                    
                    break;
                  
                  case 2:
                    lcd.clear();  
                    lcd.setCursor(0, 0);
                    lcd.print("Headset");
                    selection=selection+1;
                    delay(debounce);
                    
                    break;
                  
                  case 3:
                    lcd.clear();  
                    lcd.setCursor(0, 0);
                    lcd.print("About");
                    selection=selection+1;
                    delay(debounce);
                    
                    break; 
                  
                  case 4:
                    lcd.clear();  
                    lcd.setCursor(0, 0);
                    lcd.print("Reset");
                    selection=selection+1;
                    delay(debounce);
    
                    break;                        
              }
   return 1;           
}

int test(int x){
          switch(x){
                 case 0:
                    asm volatile ("  jmp 0");
                    delay(2000);
                 break;
                 
                 case 1:
                   testinomloop= 1;
                   lcd.clear();  
                   lcd.setCursor(2, 0);
                   lcd.print("SOUND LEVEL");
                   delay(500);
                   lcd.setCursor(2, 0);
                   lcd.print("               ");

                 break;
                 
                 case 2:
                    testinomloop= 2;
                    lcd.clear();  
                    lcd.setCursor(2, 0);
                    lcd.print("PUSH  BUTTON");
                    lcd.setCursor(5, 1);
                    lcd.print("IN TEST");
                    lcd.setCursor(0, 1); 
                 break;
                 
                 case 3:
                    testinomloop= 3;
                    lcd.clear();  
                    lcd.setCursor(0, 0);
                    lcd.print("HEADSET IN TEST");
 
                 break;
                 
                 case 4:
                    lcd.clear();  
                    lcd.setCursor(2, 0);
                    lcd.print("AUDIOCHECKER");
                    lcd.setCursor(4, 1);
                    lcd.print("Rel. 1.0");
                    delay(debounce);
                  
                 break;
    
          }
     return 0;
}  
