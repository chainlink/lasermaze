#include "neopixel/neopixel.h"

//Clock init
unsigned long Watch, _micro, timeNow = micros();
unsigned int Clock = 0, R_clock;
boolean Reset = false, Stop = false, Paused = false;
volatile boolean timeFlag = false;

//IO Pins
#define BUTTON_PIN D3

#define PIXEL_PIN D5
#define PIXEL_COUNT 140 // 28 LEDS (7 segments * 5 pixels per seg * 4 digits)
#define PIXEL_TYPE WS2812B

#define SENSOR_PIN A5

#define NUM_DIGITS 4
#define SEG_LENGTH 5
#define NUM_SEGMENTS 7 //7 Segments in display
#define HIT_DELAY_MS 1000 //How long to display hit for

bool started = false;
int highVal = 0;
int hitDelay = 0;
float change = 0.0;
int currSensor = 0;
int btnVal = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

int seg_on = strip.Color(255, 0, 0); // Red
int seg_off = strip.Color(0, 0, 0);//Black (off) 

//Digit Segments
// // b //
// a    c
// // d //
// e    g
// // f //
byte DIGITS [10][7] = {
 { 1, 1, 1, 0, 1, 1, 1 }, //ZERO
 { 1, 0, 0, 0, 1, 0, 0 }, //ONE
 { 0, 1, 1, 1, 1, 1, 0 }, //TWO
 { 0, 1, 1, 1, 0, 1, 1 }, //THREE
 { 1, 0, 1, 1, 0, 0, 1 }, //FOUR 
 { 1, 1, 0, 1, 0, 1, 1 }, //FIVE
 { 1, 1, 0, 1, 1, 1, 1 }, //SIX
 { 0, 1, 1, 0, 0, 0, 1 }, //SEVEN
 { 1, 1, 1, 1, 1, 1, 1 }, //EIGHT
 { 1, 1, 1, 1, 0, 0, 1 }, //NINE
};


int timeControl(String command) {
    if(command == "start") {
        StartTimer();
        return 1;
    } else if (command == "stop") {
        PauseTimer();
        return 1;
    } else if (command == "reset") {
        ResetTimer();
        return 1;
    }   
    
}

void setup() {
    //WiFi.setCredentials("MTS464", "2041459581");
    Particle.function("timeControl", timeControl);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    hitDelay = millis(); //Now
    SetTimer(0,59,59);
    PauseTimer();
    //Initialize pixels
    strip.begin();
    strip.setBrightness(60);
    strip.show(); // Initialize all pixels to 'off'
    
    //Particle.variable("highVal", &highVal, INT);
    //Particle.variable("currSensor", &currSensor, INT);
}

void loop() {
  CountDownTimer(); // run the timer
  
  //Read current values
  currSensor = analogRead(SENSOR_PIN);
  btnVal = digitalRead(BUTTON_PIN);
  
  if(!started)
  {
    delay(5000);
    highVal = analogRead(SENSOR_PIN);
    started = true;
    setDigits();
  }
  
  change = (float)abs(highVal - currSensor) / (float)highVal;
  
  //If hit and delay has been exceeded
  if (change > 1.0 && (hitDelay + HIT_DELAY_MS) < millis()) { //Add game started check
    hitDelay = millis();
    int totalSeconds = ShowTotalSeconds();
    int newTime = totalSeconds - 30;
    
    if(newTime < 0) {
        newTime = 0;
    }
    //Particle.publish("LASER", "HIT" , 60, PRIVATE);
    SetTimer(newTime);
    setHit();
  }
  
  if(btnVal == LOW) {
    StopTimer();
  } else {
    //StartTimer();
  }

  // this prevents the time from being constantly shown.
  if (TimeHasChanged()) 
  {
    Particle.publish("change", String(change, DEC), 60, PRIVATE);
    //Particle.publish("timeNow","TIME!",60,PRIVATE);
    //String timeNow = ShowHours() + ":" + ShowMinutes() + ":" + ShowSeconds();
    //Particle.publish("time", timeNow , 60, PRIVATE);
    //Serial.print(ShowHours());
    //Serial.print(":");
    //Serial.print(ShowMinutes());
    //Serial.print(":");
    //Serial.print(ShowSeconds());
    //Serial.print(":");
    //Serial.print(ShowMilliSeconds());
    //Serial.print(":");
    //Serial.println(ShowMicroSeconds());
    // This DOES NOT format the time to 0:0x when seconds is less than 10.
    // if you need to format the time to standard format, use the sprintf() function.
    if (hitDelay == 0 || (hitDelay + HIT_DELAY_MS) < millis()) {
        setDigits();
    }
  }
}

//Set display code
// ------------------------------------------------------------------------------------------
//Array representing values for each digit
void setDigits() {
  for(int digitIndex = 0; digitIndex < NUM_DIGITS; digitIndex++) {
    int digit;
    if (digitIndex == 0) {
      digit = ( ShowMinutes() / 10 ) % 10;
    } else if (digitIndex == 1) {
      digit = ShowMinutes() % 10;
    } else if (digitIndex == 2) {
      digit = ( ShowSeconds() / 10 ) % 10;
    } else if (digitIndex == 3) {
      digit = ShowSeconds() % 10;
    }
    
    Serial.print(digit);
    Serial.print(":");
    for(int segmentIndex = 0; segmentIndex < NUM_SEGMENTS; segmentIndex++) {
      //Digit Offset + Segment Offset
      int startIndex = calcStartIndex(digitIndex, segmentIndex);
      //Digit offset + segment offset + seg length
      int endIndex = calcEndIndex(startIndex);
      
      //Determine if segment on or off
      if (DIGITS[digit][segmentIndex] == 1){
        //ON
        setRangeOn(startIndex, endIndex);
      } else {
        //OFF
        setRangeOff(startIndex, endIndex);
      }
    }
  }
  strip.show();
}

//Set all to white
void setHit() {
    for (int i = 0; i < 140; i++) {
        strip.setPixelColor(i, strip.Color(255,255,255));
    }
    strip.show();
}
int calcStartIndex(int digitIndex, int segmentIndex) {
    return (digitIndex * 7 * SEG_LENGTH) + (segmentIndex * SEG_LENGTH);
}

int calcEndIndex(int startIndex) {
    return startIndex + SEG_LENGTH;
}

//Sets Neopixel on or off (inclusive)
void setRangeOn(int startIndex, int endIndex) {
  for(int i = startIndex; i <= endIndex; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0)); 
  }
}
void setRangeOff(int startIndex, int endIndex) {
  for(int i = startIndex; i <= endIndex; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0)); 
  }
}

// ------------------------------------------------------------------------------------------
// //http://playground.arduino.cc/Main/CountDownTimer
// Timer Code
// ------------------------------------------------------------------------------------------
boolean CountDownTimer()
{
  static unsigned long duration = 1000000; // 1 second
  timeFlag = false;

  if (!Stop && !Paused) // if not Stopped or Paused, run timer
  {
    // check the time difference and see if 1 second has elapsed
    if ((_micro = micros()) - timeNow > duration ) 
    {
      Clock--;
      timeFlag = true;

      if (Clock == 0) // check to see if the clock is 0
        Stop = true; // If so, stop the timer

     // check to see if micros() has rolled over, if not,
     // then increment "time" by duration
      _micro < timeNow ? timeNow = _micro : timeNow += duration; 
    }
  }
  return !Stop; // return the state of the timer
}

void ResetTimer()
{
  SetTimer(R_clock);
  //Stop = false;
}

void StartTimer()
{
  Watch = micros(); // get the initial microseconds at the start of the timer
  Stop = false;
  Paused = false;
}

void StopTimer()
{
  Stop = true;
}

void StopTimerAt(unsigned int hours, unsigned int minutes, unsigned int seconds)
{
  if (TimeCheck(hours, minutes, seconds) )
    Stop = true;
}

void PauseTimer()
{
  Paused = true;
}

void ResumeTimer() // You can resume the timer if you ever stop it.
{
  Paused = false;
}

void SetTimer(unsigned int hours, unsigned int minutes, unsigned int seconds)
{
  // This handles invalid time overflow ie 1(H), 0(M), 120(S) -> 1, 2, 0
  unsigned int adjustedSeconds = (seconds / 60);
  unsigned int adjustedMinutes = (minutes / 60);
  if(adjustedSeconds > 0) minutes += adjustedSeconds;
  if(adjustedMinutes > 0) hours += adjustedMinutes;

  Clock = (hours * 3600) + (minutes * 60) + (seconds % 60);
  R_clock = Clock;
  //Stop = false;
}

void SetTimer(unsigned int seconds)
{
 // StartTimer(seconds / 3600, (seconds / 3600) / 60, seconds % 60);
 Clock = seconds;
 R_clock = Clock;
 //Stop = false;
}

unsigned int ShowHours()
{
  return Clock / 3600;
}

unsigned int ShowMinutes()
{
  return (Clock / 60) % 60;
}

unsigned int ShowSeconds()
{
  return Clock % 60;
}

unsigned int ShowTotalSeconds() {
   return Clock; 
}
unsigned long ShowMilliSeconds()
{
  return (_micro - Watch)/ 1000.0;
}

unsigned long ShowMicroSeconds()
{
  return _micro - Watch;
}

boolean TimeHasChanged()
{
  return timeFlag;
}

// output true if timer equals requested time
boolean TimeCheck(unsigned int hours, unsigned int minutes, unsigned int seconds) 
{
  return (hours == ShowHours() && minutes == ShowMinutes() && seconds == ShowSeconds());
}
// ------------------------------------------------------------------------------------------

