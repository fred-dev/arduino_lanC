//commit/*
//commit/*
//commit/*

SIMPLE LANC REMOTE
Version 1.0
Sends LANC commands to the LANC port of a video camera.
Tested with a Canon XF300 camcorder
For the interface circuit interface see 
http://controlyourcamera.blogspot.com/2011/02/arduino-controlled-video-recording-over.html
Feel free to use this code in any way you want.
2011, Martin Koch

"LANC" is a registered trademark of SONY.
CANON calls their LANC compatible port "REMOTE".
*/

#define cmdPin 7 
#define lancPin 11
#define recButton 6
#define zoomOutButton 5
#define zoomInButton 4
#define focusNearButton 3
#define focusFarButton 2
int cmdRepeatCount;
int bitDuration = 104; //Duration of one LANC bit in microseconds. 


//LANC commands byte 0 + byte 1
//Tested with Canon XF300

//Start-stop video recording
boolean REC[] = {LOW,LOW,LOW,HIGH,HIGH,LOW,LOW,LOW,   LOW,LOW,HIGH,HIGH,LOW,LOW,HIGH,HIGH}; //18 33

//Zoom in from slowest to fastest speed
boolean ZOOM_IN_0[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW}; //28 00
boolean ZOOM_IN_1[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,LOW,LOW,HIGH,LOW}; //28 02
boolean ZOOM_IN_2[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,LOW,HIGH,LOW,LOW}; //28 04
boolean ZOOM_IN_3[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,LOW,HIGH,HIGH,LOW}; //28 06
boolean ZOOM_IN_4[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,HIGH,LOW,LOW,LOW}; //28 08
boolean ZOOM_IN_5[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,HIGH,LOW,HIGH,LOW}; //28 0A
boolean ZOOM_IN_6[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,HIGH,HIGH,LOW,LOW}; //28 0C
boolean ZOOM_IN_7[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,HIGH,HIGH,HIGH,LOW}; //28 0E

//Zoom out from slowest to fastest speed
boolean ZOOM_OUT_0[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,LOW,LOW,LOW,LOW}; //28 10
boolean ZOOM_OUT_1[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,LOW,LOW,HIGH,LOW}; //28 12
boolean ZOOM_OUT_2[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,LOW,HIGH,LOW,LOW}; //28 14
boolean ZOOM_OUT_3[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,LOW,HIGH,HIGH,LOW}; //28 16
boolean ZOOM_OUT_4[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,HIGH,LOW,LOW,LOW}; //28 18
boolean ZOOM_OUT_5[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,HIGH,LOW,HIGH,LOW}; //28 1A
boolean ZOOM_OUT_6[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,HIGH,HIGH,LOW,LOW}; //28 1C
boolean ZOOM_OUT_7[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,HIGH,HIGH,HIGH,LOW}; //28 1E

//Focus control. Camera must be switched to manual focus
boolean FOCUS_NEAR[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,HIGH,LOW,LOW,LOW,HIGH,HIGH,HIGH}; //28 47
boolean FOCUS_FAR[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,HIGH,LOW,LOW,LOW,HIGH,LOW,HIGH}; //28 45

boolean FOCUS_AUTO[] = {LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,HIGH,LOW,LOW,LOW,LOW,LOW,HIGH}; //28 41

//boolean POWER_OFF[] = {LOW,LOW,LOW,HIGH,HIGH,LOW,LOW,LOW,   LOW,HIGH,LOW,HIGH,HIGH,HIGH,HIGH,LOW}; //18 5E
//boolean POWER_ON[] = {LOW,LOW,LOW,HIGH,HIGH,LOW,LOW,LOW,   LOW,HIGH,LOW,HIGH,HIGH,HIGH,LOW,LOW}; //18 5C  Doesn't work because there's no power supply from the LANC port when the camera is off
//boolean POWER_OFF2[] = {LOW,LOW,LOW,HIGH,HIGH,LOW,LOW,LOW,   LOW,LOW,HIGH,LOW,HIGH,LOW,HIGH,LOW}; //18 2A Turns the XF300 off and then on again
//boolean POWER_SAVE[] = {LOW,LOW,LOW,HIGH,HIGH,LOW,LOW,LOW,   LOW,HIGH,HIGH,LOW,HIGH,HIGH,LOW,LOW}; //18 6C Didn't work



void setup() {

 pinMode(lancPin, INPUT); //listens to the LANC line
 pinMode(cmdPin, OUTPUT); //writes to the LANC line

 pinMode(recButton, INPUT); //start-stop recording button
        digitalWrite(recButton, HIGH); //turn on an internal pull up resistor
        pinMode(zoomOutButton, INPUT); 
        digitalWrite(zoomOutButton, HIGH);
        pinMode(zoomInButton, INPUT); 
        digitalWrite(zoomInButton, HIGH);
        pinMode(focusNearButton, INPUT); 
        digitalWrite(focusNearButton, HIGH);
        pinMode(focusFarButton, INPUT); 
        digitalWrite(focusFarButton, HIGH);
        
        digitalWrite(cmdPin, LOW); //set LANC line to +5V
        delay(5000); //Wait for camera to power up completly
        bitDuration = bitDuration - 8; //Writing to the digital port takes about 8 microseconds so only 96 microseconds are left for each bit
}

void loop() {
  
  
   if (!digitalRead(recButton)) {
    lancCommand(REC); 
  }
  
  if (!digitalRead(zoomOutButton)) {
    lancCommand(ZOOM_OUT_4); 
  }
  
  if (!digitalRead(zoomInButton)) {
    lancCommand(ZOOM_IN_4); 
  }
  
  if (!digitalRead(focusNearButton)) {
    lancCommand(FOCUS_NEAR); 
  }
  
  if (!digitalRead(focusFarButton)) {
    lancCommand(FOCUS_FAR); 
  }
}



void lancCommand(boolean lancBit[]) {
       
        cmdRepeatCount = 0;
  
   while (cmdRepeatCount < 5) {  //repeat 5 times to make sure the camera accepts the command

                while (pulseIn(lancPin, HIGH) < 5000) {   
                  //"pulseIn, HIGH" catches any 0V TO +5V TRANSITION and waits until the LANC line goes back to 0V 
                  //"pulseIn" also returns the pulse duration so we can check if the previous +5V duration was long enough (>5ms) to be the pause before a new 8 byte data packet
                  //Loop till pulse duration is >5ms
                }

   //LOW after long pause means the START bit of Byte 0 is here
   delayMicroseconds(bitDuration);  //wait START bit duration

   //Write the 8 bits of byte 0 
                        //Note that the command bits have to be put out in reverse order with the least significant, right-most bit (bit 0) first
                        for (int i=7; i>-1; i--) {
     digitalWrite(cmdPin, lancBit[i]);  //Write bits. 
     delayMicroseconds(bitDuration); 
                        }
   
                        //Byte 0 is written now put LANC line back to +5V
                        digitalWrite(cmdPin, LOW);
                        delayMicroseconds(10); //make sure to be in the stop bit before byte 1
                        
                        while (digitalRead(lancPin)) { 
                          //Loop as long as the LANC line is +5V during the stop bit
                        }

                        //0V after the previous stop bit means the START bit of Byte 1 is here
         delayMicroseconds(bitDuration);  //wait START bit duration
      
         //Write the 8 bits of Byte 1
                        //Note that the command bits have to be put out in reverse order with the least significant, right-most bit (bit 0) first
                        for (int i=15; i>7; i--) {
              digitalWrite(cmdPin,lancBit[i]);  //Write bits 
             delayMicroseconds(bitDuration);
                        }
 
                        //Byte 1 is written now put LANC line back to +5V
                        digitalWrite(cmdPin, LOW); 

   cmdRepeatCount++;  //increase repeat count by 1
   
   /*Control bytes 0 and 1 are written, now don’t care what happens in Bytes 2 to 7
   and just wait for the next start bit after a long pause to send the first two command bytes again.*/
  

 }//While cmdRepeatCount < 5
}


