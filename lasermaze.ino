#include <Button.h>
#include <Adafruit_NeoPixel.h>

//http://playground.arduino.cc/Main/CountDownTimer

//Clock init
unsigned long Watch, _micro, time = micros();
unsigned int Clock = 0, R_clock;
boolean Reset = false, Stop = false, Paused = false;
volatile boolean timeFlag = false;

//IO Pins
#define NEOPIXEL_PIN 6
#define BUTTON_PIN 12
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
// 28 LEDS (7 segments * 5 pixels per seg * 4 digits)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(140, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

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
 { 1, 0, 1, 1, 1, 0, 0 }, //FOUR 
 { 1, 1, 0, 1, 0, 1, 1 }, //FIVE
 { 1, 1, 0, 1, 1, 1, 1 }, //SIX
 { 0, 1, 1, 0, 0, 0, 1 }, //SEVEN
 { 1, 1, 1, 1, 1, 1, 1 }, //EIGHT
 { 1, 1, 1, 1, 0, 0, 1 }, //NINE
};

#define NUM_DIGITS 4
#define SEG_LENGTH 5
#define NUM_SEGMENTS 7 //7 Segments in display
int seg_on = strip.Color(255, 0, 0); // Red
int seg_off = strip.Color(0, 0, 0);//Black (off) 

Button button = Button(BUTTON_PIN);

void onPress(Button& b){
    Serial.print("HIT: ");
    int totalSeconds = ShowTotalSeconds();
    Serial.println(totalSeconds);
    int newTime = totalSeconds - 30;
    SetTimer(newTime);
}

void setup()
{
  Serial.begin(115200);
  SetTimer(0,59,0); 
  StartTimer();
  button.pressHandler(onPress);  
  strip.begin();
  strip.setBrightness(60);
  strip.show(); // Initialize all pixels to 'off'
}

void loop()
{
  CountDownTimer(); // run the timer
  // update the buttons' internals
  button.process();
  // this prevents the time from being constantly shown.
  if (TimeHasChanged() ) 
  {
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
    setDigits();
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
      int startIndex = (digitIndex * 7 * SEG_LENGTH) + (segmentIndex * SEG_LENGTH);
      //Digit offset + segment offset + seg length
      int endIndex = startIndex + SEG_LENGTH;
      
      //Determine if segment on or off
      if (DIGITS[digit][segmentIndex] == 1){
        //ON
        setRangeOn(startIndex, endIndex);
        //Serial.print("ON:");
        //Serial.print(startIndex);
        //Serial.print(":");
        //Serial.print(endIndex);
      } else {
        //OFF
        setRangeOff(startIndex, endIndex);
      }
    }
  }
  Serial.println();
  strip.show();
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

// Timer Code
// ------------------------------------------------------------------------------------------
boolean CountDownTimer()
{
  static unsigned long duration = 1000000; // 1 second
  timeFlag = false;

  if (!Stop && !Paused) // if not Stopped or Paused, run timer
  {
    // check the time difference and see if 1 second has elapsed
    if ((_micro = micros()) - time > duration ) 
    {
      Clock--;
      timeFlag = true;

      if (Clock == 0) // check to see if the clock is 0
        Stop = true; // If so, stop the timer

     // check to see if micros() has rolled over, if not,
     // then increment "time" by duration
      _micro < time ? time = _micro : time += duration; 
    }
  }
  return !Stop; // return the state of the timer
}

void ResetTimer()
{
  SetTimer(R_clock);
  Stop = false;
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
  unsigned int _S = (seconds / 60), _M = (minutes / 60);
  if(_S) minutes += _S;
  if(_M) hours += _M;

  Clock = (hours * 3600) + (minutes * 60) + (seconds % 60);
  R_clock = Clock;
  Stop = false;
}

void SetTimer(unsigned int seconds)
{
 // StartTimer(seconds / 3600, (seconds / 3600) / 60, seconds % 60);
 Clock = seconds;
 R_clock = Clock;
 Stop = false;
}

int ShowHours()
{
  return Clock / 3600;
}

int ShowMinutes()
{
  return (Clock / 60) % 60;
}

int ShowSeconds()
{
  return Clock % 60;
}

int ShowTotalSeconds() {
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

