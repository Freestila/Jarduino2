// Do not remove the include below
#include "Jarduino_basic.h"

//************************************************************************************/
// Jarduino Aquarium Controller v.1.1 - release date: April 2012
//   Written and Debugged by Jamie M. Jardin
//   Copyright 2011, 2012 Jamie M. Jardin
//     email:  jjardi56@msn.com
//     http://www.ultimatereef.net/forums/member.php?u=44387
//     http://www.reefcentral.co.uk/member.php/13262-TheDOdblG
//************************************************************************************/
//
// Previous Version - Jarduino Aquarium Controller v.1.0 (December 2011)
//
// Main code based on Stilo
//   http://code.google.com/p/stilo/
//
// LED controlling algorithm is based on Krusduino by Hugh Dangerfield
//   http://code.google.com/p/dangerduino/
//
// Moon Phase algorithm is based in part on a Moon Phase function by NightAtTheOpera
//   http://www.nano-reef.com/forums/index.php?showtopic=217305&st=0&
//
// Special Thanks:
//    Dave Rosser & Hugh Dangerfield - (aka Lewis & Clark?) - Krusduino's their baby
//      http://www.ultimatereef.net/forums/showthread.php?t=363432
//    Mark Chester aka "Koyaanisqatsi" - Knows everything there's to know about LEDs
//      http://www.chestersgarage.com/
//    Kev Tench aka "tangtastic" - DIY Reef Wizard!
//      http://ukreefs.com/index.php?action=profile;u=1
//    Ned Simpson aka "Surff" - Another DIY Reef Guy
//      http://www.ultimatereef.net/forums/showthread.php?t=400993
//    Neil Williams aka "neildotwilliams" - Yet another DIY Reefer
//      http://www.ultimatereef.net/forums/member.php?u=37721
//    Albert aka "selfonlypath" - Arduino genius
//      http://arduino.cc/forum/index.php?action=profile;u=12410
//
//************************************************************************************/
//
// Known Bugs/Issues:
//   - It may be possible to set a date that does not exist (ie. FEB 31, 2011)
//   - Occasionally when returing from ScreenSaver, minor Time and Date Bar issues
//   - The WaveMaker may cut the set amount of seconds by half
//   - The level of automation may make you lazy
//   - If you spot an error or bug, please let me know!
//   -
//   -
//************************************************************************************/
//
// LEGAL DISCLAIMER:
//   Jarduino Aquarium Controller v.1.1, Copyright 2011, 2012 Jamie M. Jardin.
//   I'm providing this program as free software with the sole intent on furthering
//   DIY Aquarium Lighting, but WITHOUT ANY WARRANTY; without even the implied warranty
//   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. You may modify and/or use
//   it under the terms of the GNU General Public License as published by the Free
//   Software Foundation version 3 of the License, or (at your option) any later
//   version.  However if you intend on using the program (either in its entirety or any
//   part of it) in any way, shape, or form for PROFIT, you MUST contact me and obtain
//   my EXPRESS WRITTEN CONSENT (contact information is provided above).
//   VIOLATORS WILL BE PROSECUTED TO THE FULLEST EXTENT OF THE LAW.
//
//************************************************************************************/
//
// IF YOU WANT SOME HELP, PLEASE READ ME!
//   Feel free to make changes to suit your needs.  For you convenience, Iâ€™m listing the
//   line numbers in the sketch most commonly changed.  Some of these values will require
//   a change while other values can simply be modified according to user preference.
//   If you make changes to lines other than those listed below, know that it may render
//   the program inoperable or cause unpredictable behavior.
//
//   86, 87, 92, 93, 104-110, 113-116, 119-122, 125, 129-131, 136, 137, 155, 176, 207,
//   209, 212, 234, 298-309, 313-324, 328-339, 343-354, 358-369, 373-384
//
//************************************************************************************/

//LIBRARIES
#include <ITDB02_Graph16.h>
#include <avr/pgmspace.h>
#include <ITDB02_Touch.h>
#include <Wire.h>
#include <EEPROM.h>
#include "writeAnything.h"
#include <DS1307.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EasyTransfer.h>

//dimm execution

struct SEND_DATA_STRUCTURE {
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  uint8_t blau1;
  uint8_t blau2;
  uint8_t weiss1;
  uint8_t weiss2;
  uint8_t weiss3;
};
//create object
EasyTransfer ET;

//give a name to the group of data
SEND_DATA_STRUCTURE mydata;

//Default Controller Settings
boolean RECOM_RCD = true;            //For Mean Well drivers change "true" to "false"
boolean CLOCK_SCREENSAVER = true;    //For a Clock Screensaver "true" / Blank Screen "false"
//You can turn the Screensaver ON/OFF in the pogram

//TOUCH PANEL and ITDB02 MEGA SHIELD
//(Mega Shield utilizes pins 5V, 3V3, GND, 2-6, 20-41, & (50-53 for SD Card))
ITDB02 myGLCD ( 38, 39, 40, 41, ITDB32S );    //May need to add "ITDB32S" depending on LCD controller
ITDB02_Touch myTouch ( 6, 5, 4, 3, 2 );

//Declare which fonts to be utilized
extern uint8_t SmallFont [];
extern uint8_t BigFont [];
extern uint8_t SevenSegNumFont [];

#define LARGE true
#define SMALL false
#define BUTTONCOLOR_BLUE 0
#define BUTTONCOLOR_RED 1
#define BUTTONCOLOR_GREEN 2

//Free pins: 7-13,14,15,1,2,46,

//Define the PWM PINS for the LEDs
const int ledPinSump = 53;            //PowerLed Shield pin 10
const int ledPinBlue = 53;            //PowerLed Shield pin 5
const int ledPinWhite = 53;           //PowerLed Shield pin 3
const int ledPinRoyBlue = 53;        //PowerLed Shield pin 6
const int ledPinRed = 53;            //PowerLed Shield pin 9
const int ledPinUV = 53;             //PowerLed Shield pin 11
const int ledPinMoon = 53;           //PowerLed Shield pin 13 (Modification to Shield & NOT controlled by an array)
// Pin 18 & 19 are used for Serial1 comm with Dimm Module
// Pin 16 & 17 are reserved for Bluetooth-communication (or use Serial?)

// Define the other DIGITAL and/or PWM PINS being used
const int tempHeatPin = 53;          //Heater on/off (set thermostat on heater to highest desired level)
const int tempChillPin = 53;         //Chiller on/off (set thermostat on chiller to lowest desired level)
const int WaveMakerTop = 7;         //Hydor Koralia Evolution (Top Plug)
const int WaveMakerBottom = 8;      //Hydor Koralia Evolution (Bottom Plug)
const int HoodFansPWM = 9;          //PWM Hood Heatsink Fan (code added so frequency = 25kHz)
const int SumpFanPWM = 10;           //PWM Sump Heatsink Fan (code added so frequency = 25kHz)
const int HoodFansTranzPin = 44;     //Hood Heatsink Fan on/off
const int SumpFanTranzPin = 43;      //Sump Heatsink Fan on/off
const int tempAlarmPin = 53;         //Buzzer Alarm for Temperature offsets
const int autoFeeder = 53;           //Automatic Fish Feeder
const int dosingPump1Pin = 45;   		//
const int dosingPump2Pin = 46;   		//
const int dosingPump3Pin = 47;   		//
const int dosingPump4Pin = 48;   		//
const int aquaTemp = 49;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Dosing pump values									//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++//

int mlPerTimePump1 = 10;  // ML per second for pump 1. real value is this / 10 (so no float is needed)
int mlPerTimePump2 = 10;  // ML per second for pump 2. real value is this / 10 (so no float is needed)
int mlPerTimePump3 = 10;  // ML per second for pump 3. real value is this / 10 (so no float is needed)
int mlPerTimePump4 = 10;  // ML per second for pump 4. real value is this / 10 (so no float is needed)
const int testtime = 10;  // test for 10 seconds
int pump1On = 3;
int pump2On = 3;
int pump3On = 3;
int pump4On = 3;  // values 0 = deactived=3; int 1 = one pump time=3; int 2 = two pump time=3; int ... till 3
int pump1_1h = 2;
int pump1_1m = 3;
int pump1_2h = 1;
int pump1_2m = 4;
int pump1_3h = 5;
int pump1_3m = 1;  // time values for pump1
int pump2_1h = 1;
int pump2_1m = 10;
int pump2_2h = 10;
int pump2_2m = 2;
int pump2_3h = 3;
int pump2_3m = 1;  // time values for pump2
int pump3_1h = 2;
int pump3_1m = 10;
int pump3_2h = 10;
int pump3_2m = 4;
int pump3_3h = 4;
int pump3_3m = 1;  // time values for pump3
int pump4_1h = 2;
int pump4_1m = 10;
int pump4_2h = 2;
int pump4_2m = 1;
int pump4_3h = 1;
int pump4_3m = 1;  // time values for pump4
int pump1_1ml = 10;
int pump1_2ml = 220;
int pump1_3ml = 111;  // ml values for pump 1 at given times
int pump2_1ml = 330;
int pump2_2ml = 320;
int pump2_3ml = 22;  // ml values for pump 1 at given times
int pump3_1ml = 33;
int pump3_2ml = 222;
int pump3_3ml = 300;  // ml values for pump 1 at given times
int pump4_1ml = 134;
int pump4_2ml = 120;
int pump4_3ml = 1122;  // ml values for pump 1 at given times
int *pumph [4] [3] = { 
  { 
    &pump1_1h, &pump1_2h, &pump1_3h   }
  , { 
    &pump2_1h, &pump2_2h, &pump2_3h   }
  , { 
    &pump3_1h, &pump3_2h, &pump3_3h   }
  , { 
    &pump4_1h, &pump4_2h,
    &pump4_3h   } 
};
int *pumpm [4] [3] = { 
  { 
    &pump1_1m, &pump1_2m, &pump1_3m   }
  , { 
    &pump2_1m, &pump2_2m, &pump2_3m   }
  , { 
    &pump3_1m, &pump3_2m, &pump3_3m   }
  , { 
    &pump4_1m, &pump4_2m,
    &pump4_3m   } 
};
int *pumpml [4] [3] = { 
  { 
    &pump1_1ml, &pump1_2ml, &pump1_3ml   }
  , { 
    &pump2_1ml, &pump2_2ml, &pump2_3ml   }
  , { 
    &pump3_1ml, &pump3_2ml, &pump3_3ml   }
  , { 
    &pump4_1ml,
    &pump4_2ml, &pump4_3ml   } 
};
int *pumpCalibrate [4] = { 
  &mlPerTimePump1, &mlPerTimePump2, &mlPerTimePump3, &mlPerTimePump4 };
int *pumpOn [4] = { 
  &pump1On, &pump2On, &pump3On, &pump4On };
bool pumpRunning [4] ={ 
  false, false, false, false};
const int *pumpPins [4] = { 
  &dosingPump1Pin, &dosingPump2Pin, &dosingPump3Pin, &dosingPump4Pin };

boolean dosingPumpCalibrationActive = false;  // For calibrating dosing pump
boolean dosingPumpSettingsChanged = false;

long dosingPumpOffTimes [4] = { 
  0L, 0L, 0L, 0L };  // End times: when shoudl pumpes be deactivated?

//++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// other values								//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++//
boolean checkTemp = true;
// DS18B20 Temperature sensors plugged into pin 51 (Water, Hood, & Sump)
OneWire OneWireBus ( 51 );     //Choose a digital pin
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors ( &OneWireBus );
// Assign the addresses of temperature sensors.  Add/Change addresses as needed.
// dtaa: green. White for fanspeed / pwm
DeviceAddress waterThermometer = { 
  0x28, 0x5C, 0x56, 0x59, 0x03, 0x00, 0x00, 0xEB };
//10 C6 95 77 02 08 00 A4
DeviceAddress hoodThermometer2 = { 
  0x10, 0x79, 0xAA, 0x77, 0x02, 0x08, 0x00, 0x6C };
DeviceAddress hoodThermometer = { 
  0x10, 0xC6, 0x95, 0x77, 0x02, 0x08, 0x00, 0xA4 };

DeviceAddress sumpThermometer = { 
  0x10, 0x35, 0x90, 0x77, 0x02, 0x08, 0x00, 0x5D };

float tempW = 0;                     //Water temperature values
float tempH = 0;                     //Heatsink temperature
float tempS = 0;                     //Sump heatsink temperature
int TempToBeginHoodFanInDegC = 29;   //Temperature to Turn on Hood Fans (in Degrees C)
int TempToBeginSumpFanInDegC = 29;   //Temperature to Turn on Sump Fan (in Degrees C)
float FanOn = 0.2;                   //Starts Fan(s) at 20% Duty Cycle
int HoodTempInterval = 0;            //Used for PWM Duty calculations
int SumpTempInterval = 0;            //Used for PWM Duty calculations
float HoodFanSpeedIncrease = 0;      //Used for PWM Duty calculations
float SumpFanSpeedIncrease = 0;      //Used for PWM Duty calculations
float HoodPWM = 0;                   //Used for PWM Duty calculations
float SumpPWM = 0;                   //Used for PWM Duty calculations

float setTempC = 0.0;                //Desired Water Temperature (User input in program)
float setTempF = 0.0;
float offTempC = 0.0;                //Desired Water Temp. Offsets for Heater & Chiller (User input in program)
float offTempF = 0.0;
float alarmTempC = 0.0;              //Temperature the Alarm will sound (User input in program)
float alarmTempF = 0.0;
boolean tempCoolflag = 0;            //1 if cooling on
boolean tempHeatflag = 0;            //1 if heating on
boolean tempAlarmflag = 0;           //1 if alarm on
unsigned long intervalAlarm = 1000 * 30;      //Interval to beep the Alarm (1000 * seconds)
float temp2beS;                      //Temporary Temperature Values
float temp2beO;                      //Temporary Temperature Values
float temp2beA;                      //Temporary Temperature Values
int setTempScale = 0;                //Celsius=0 || Fahrenheit=1 (change in prog)
char degC_F [2];                       //Used in the Conversion of Celsius to Fahrenheit

int setCalendarFormat = 0;           //DD/MM/YYYY=0 || Month DD, YYYY=1 (change in prog)
int setTimeFormat = 0;               //24HR=0 || 12HR=1 (change in prog)
int rtc [7], rtcSet [7];               //Clock arrays
int rtcSet2, AM_PM, yTime;           //Setting clock stuff
int timeDispH, timeDispM, xTimeH, xTimeM10, xTimeM1, xTimeAMPM, xColon;
char otherData [19];  // other printed data
//char formattetTime12[9]; //07:48 PM
//char formattetTime24[6]; //19:48
//byte data [56];
char time [11], date [16], day [16];

float LC = 29.53059;                 //1 Lunar Cycle = 29.53059 days
char LP [16];                           //LP = Lunar Phase - variable used to print out Moon Phase
double AG;
int MI, tMI;                         //Maximum Illumination of Moon (User Defined/Set in Prog. -- Default = 0)
int MoonLow = 43;                    //Highest Value (0-255) at which Led is Still OFF, or the Value
//you wish the New Moon to Shine (it will increase from here to MI)

unsigned int *MoonPic;               //Pointer to the Lunar Phase Pics
extern unsigned int                  //Lunar Phase Pics
New_Moon [0xD24], Waxing_Crescent [0xD24], First_Quarter [0xD24], Waxing_Gibbous [0xD24], Full_Moon [0xD24], Waning_Gibbous [0xD24], Last_Quarter [0xD24],
Waning_Crescent [0xD24];

int dispScreen = 0;                    //0-Main Screen, 1-Menu, 2-Clock Setup, 3-Temp Control,
//4-LED Test Options, 5-Test LED Arrays, 6-Test Individual
//LED Colors, 7-Choose LED Color, 8-View Selected Color
//Values, 9-Change Selected Color Array Values
//10-Wavemaker, 11-Wavemaker Settings, 12-General
//Settings, 13-Automatic Feeder, 14-Set Feeder Timers,
//15-About, 16-Options menu 2

#define MAINSCREEN 0
#define MENUSCREEN_ONE 1
#define CLOCKSETUP 2
#define TEMPCONTROL 3
#define LED_TEST_OPTIONS 4
#define TEST_LED_ARRAYS 5
#define TEST_INDIVIDUAL_LED 6
#define CHOOSE_LED_COLOR_VALUES 7
#define VIEW_LED_COLOR_VALUES 8
#define CHANGE_LED_COLOR_VALUES 9
#define WAVEMAKER_MAIN 10
#define WAVEMAKER_SETTINGS 11
#define GENERAL_SETTINGS 12
#define AUTOMATIC_FEEDER 13
#define FEEDER_TIMER 14
#define ABOUT 15
#define MENUSCREEN_TWO 16
#define DOSINGPUMP_MENU 17

int x, y;                            //touch coordinates

unsigned long previousMillisLED = 0;          //Used in the Test LED Array Function
unsigned long previousMillisWave = 0;         //Used in the WaveMaker Function wave_output()
unsigned long previousMillisFive = 0;         //Used in the Main Loop (Checks Time,Temp,LEDs,Screen)
unsigned long previousMillisAlarm = 0;        //Used in the Alarm

int setScreensaver = 2;              //ON=1 || OFF=2 (change in prog)
int screenSaverTimer = 0;            //counter for Screen Saver
int setScreenSaverTimer = ( 20 ) * 12;   //how long in (minutes) before Screensaver comes on

boolean SCREEN_RETURN = true;        //Auto Return to mainScreen() after so long of inactivity
int returnTimer = 0;                 //counter for Screen Return
int setReturnTimer = setScreenSaverTimer * .75;       //Will return to main screen after 75% of the amount of
//time it takes before the screensaver turns on

boolean waveMakerOff = false;        //For Turning ON/OFF WaveMaker
boolean waveMakerTest = false;       //For Testing Wave Settings

unsigned long wPump1, wPump2;                 //Total Alternating Times
unsigned long intervalAlt = wPump1;           //Changing Interval for Alternating Mode
unsigned long wOnForT, wOffForT;              //Total Synchronous-Pulse Times
unsigned long intervalSynch = wOnForT;        //Changing Interval for Synch-Pulse Mode
int PumpTstate = LOW;                //Used to set the Top Powerhead ON or OFF
int PumpBstate = LOW;                //Used to set the Bottom Powerhead ON or OFF

int WAVE, Pump1m, Pump1s, Pump2m,    //EEPROM vars
Pump2s, Synch, OnForTm, OnForTs, OffForTm, OffForTs;
int MODE = WAVE;
int MIN1 = 0, SEC1 = 0, MIN2 = 0, SEC2 = 0,  //Used in the Wavemaker viewWaveTimes()
minY1, minY2;
int Min1 = 0, Sec1 = 0, Min2 = 0, Sec2 = 0;  //Used in the Wavemaker viewWaveTimesPage() & wave+/-
int min1X = 91, sec1X = 237,             //Used in the Wavemaker waveModePlusMinus()
min2X = 91, sec2X = 237, tTime1 = 0, tTime2 = 0;
int WaveCorrector = 2;               //Fix for halving of wave seconds (Change to "1" if
//your wave seconds are doubled)

unsigned long previousMillisCt = 0;           //stores the last update for the Countdown Timer
unsigned long intervalCt = 1000;              //One Second Interval for Countdown Timer
int countDown = 5 * 60 + 0;           //Countdown for 5 minutes and zero seconds
int MIN_O = 5;                       //Start the Time at 5 (for looks only)
int SEC_T = 0;
int SEC_O = 0;

int LedChangTime = 0;                //LED change page, time and values

int min_cnt;                         //Used to determine the place in the color arrays

boolean LEDtestTick = false;         //for testing leds and speed up clock

int whiteLed, blueLed, rblueLed,     //previous LED output values
redLed, uvLed, sumpLed;
int bled_out, wled_out, rbled_out,   //current LED output values
rled_out, uvled_out, sled_out, moonled_out, colorled_out;

int COLOR = 0, WHITE = 1, BLUE = 2,        //Used to Determine View/Change Color LED Values
ROYAL = 3, RED = 4, ULTRA = 5, SUMP = 6, MOON = 7;

boolean colorLEDtest = false;        //To test individual color LEDs
int Rgad = 0, Ggad = 0, Bgad = 0,          //Used in the Test Ind. Color LEDs Gadget
Rfont = 0, Gfont = 0, Bfont = 0, Rback = 0, Gback = 0, Bback = 0, Rline = 0, Gline = 0, Bline = 0;
int CL_1 = 0, CL_10 = 0, CL_100 = 0,       //Used in the Test Ind. Color LEDs Gadget
cl_1, cl_10, cl_100;
int bcol_out, wcol_out, rbcol_out,   //Current LED output values for Test Ind. Color LEDs
rcol_out, uvcol_out, scol_out, mooncol_out;
int x1Bar = 0, x2Bar = 0,                //Used in LED Output Chart on Test Ind. LEDs Screen
xValue = 0, yValue = 0, LEDlevel, yBar;

int feedTime;
int dosingPumpSelected;
int FEEDTime1, FEEDTime2, FEEDTime3, FEEDTime4;

int feedFish1H, feedFish1M,          //Times to feed the fish
feedFish2H, feedFish2M, feedFish3H, feedFish3M, feedFish4H, feedFish4M;

int setAutoStop = 2;                 //ON=1 || OFF=2 (change in prog)
int fiveTillBackOn1, fiveTillBackOn2, fiveTillBackOn3, fiveTillBackOn4;
boolean FeedWaveCtrl_1 = false;
boolean FeedWaveCtrl_2 = false;
boolean FeedWaveCtrl_3 = false;
boolean FeedWaveCtrl_4 = false;

float getAverageOnTime ( const byte* led [] ) {
  int result = 0;
  for ( int i = 0; i < 96; i++ ) {
    result += *led [i];
  }
  // multiply by 100 and divert by 96 to get two points
  result = ( result * 100 ) / 96;
  return result / 100;
}

//DIMMING VALUES can be changed below BUT the EEPROM must be cleared first.
//To CLEAR EEPROM, use arduino-0022\libraries\EEPROM\examples\eeprom_clear\eeprom_clear.pde
//and change the 512 to 4096 before Upload.  After the LED comes on indicating the EEPROM
//has been cleared, it is now ok to change DIMMING VALUES below & Upload the sketch.
//SUMP Dimming Values 8pm to 8am
byte sled [96] = { 
  192, 200, 200, 200, 200, 200, 200, 200,   //0 - 1
  200, 200, 200, 200, 200, 200, 200, 200,   //2 - 3
  197, 195, 192, 190, 175, 175, 175, 175,   //4 - 5
  140, 140, 140, 140, 105, 95, 85, 75,      //6 - 7
  0, 0, 0, 0, 0, 0, 0, 0,                   //8 - 9
  0, 0, 0, 0, 0, 0, 0, 0,                   //10 - 11
  0, 0, 0, 0, 0, 0, 0, 0,                   //12 - 13
  0, 0, 0, 0, 0, 0, 0, 0,                   //14 - 15
  0, 0, 0, 0, 0, 0, 0, 0,                   //16 - 17
  0, 0, 0, 0, 0, 0, 0, 0,                   //18 - 19
  75, 85, 95, 105, 140, 140, 140, 140,      //20 - 21
  175, 175, 175, 175, 190, 192, 195, 197    //22 - 23
};
//REGULAR BLUE Dimming Values
byte bled [96] = { 
  0, 0, 0, 0, 0, 0, 0, 0,                   //0 - 1
  0, 0, 0, 0, 0, 0, 0, 0,                   //2 - 3
  0, 0, 0, 0, 0, 0, 0, 0,                   //4 - 5
  0, 0, 0, 0, 0, 0, 0, 0,                 //6 - 7
  5, 10, 30, 80, 80, 120, 120, 160,           //8 - 9
  160, 200, 255, 255, 255, 255, 255, 255,      //10 - 11
  255, 255, 255, 255, 255, 255, 255, 255,   //12 - 13
  255, 255, 255, 255, 255, 255, 255, 255,   //14 - 15
  255, 255, 255, 255, 255, 255, 255, 255,   //16 - 17
  255, 255, 255, 255, 255, 255, 255, 255,   //18 - 19
  255, 255, 255, 255, 200, 180, 140, 100,      //20 - 21
  30, 20, 10, 0, 0, 0, 0, 0                //22 - 23
};
//WHITE Dimming Values (White LED array in RAM)
byte wled [96] = { 
  0, 0, 0, 0, 0, 0, 0, 0,                   //0 - 1
  0, 0, 0, 0, 0, 0, 0, 0,                   //2 - 3
  0, 0, 0, 0, 0, 0, 0, 0,                   //4 - 5
  0, 0, 0, 0, 0, 0, 0, 0,                 //6 - 7
  0, 0, 0, 30, 80, 120, 120, 160,           //8 - 9
  160, 200, 255, 255, 255, 255, 255, 255,      //10 - 11
  255, 255, 255, 255, 255, 255, 255, 255,   //12 - 13
  255, 255, 255, 255, 255, 255, 255, 255,   //14 - 15
  255, 255, 255, 255, 255, 255, 255, 255,   //16 - 17
  255, 255, 255, 255, 255, 255, 255, 255,   //18 - 19
  255, 255, 255, 255, 200, 180, 140, 50,      //20 - 21
  0, 0, 0, 0, 0, 0, 0, 0                //22 - 23
};
//ROYAL BLUE Dimming Values
byte rbled [96] = { 
  0, 0, 0, 0, 0, 0, 0, 0,                   //0 - 1
  0, 0, 0, 0, 0, 0, 0, 0,                   //2 - 3
  0, 0, 0, 0, 0, 0, 0, 0,                   //4 - 5
  0, 0, 0, 0, 0, 0, 35, 40,                 //6 - 7
  40, 43, 47, 55, 65, 75, 80, 85,           //8 - 9
  90, 95, 95, 100, 110, 110, 115, 120,      //10 - 11
  125, 130, 135, 145, 145, 145, 150, 155,   //12 - 13
  160, 165, 170, 175, 180, 180, 180, 180,   //14 - 15
  180, 180, 180, 180, 175, 170, 165, 160,   //16 - 17
  155, 150, 145, 145, 145, 135, 130, 125,   //18 - 19
  120, 115, 110, 110, 100, 75, 65, 50,      //20 - 21
  40, 35, 33, 28, 0, 0, 0, 0                //22 - 23
};
//RED Dimming Values
byte rled [96] = { 
  0, 0, 0, 0, 0, 0, 0, 0,                   //0 - 1
  0, 0, 0, 0, 0, 0, 0, 0,                   //2 - 3
  0, 0, 0, 0, 0, 0, 0, 0,                   //4 - 5
  0, 0, 0, 0, 0, 0, 0, 0,                   //6 - 7
  0, 0, 0, 0, 0, 0, 0, 0,                   //8 - 9
  0, 0, 0, 0, 0, 0, 0, 0,                   //10 - 11
  30, 30, 40, 40, 50, 50, 60, 60,           //12 - 13
  60, 60, 50, 50, 40, 40, 30, 30,           //14 - 15
  0, 0, 0, 0, 0, 0, 0, 0,                   //16 - 17
  0, 0, 0, 0, 0, 0, 0, 0,                   //18 - 19
  0, 0, 0, 0, 0, 0, 0, 0,                   //20 - 21
  0, 0, 0, 0, 0, 0, 0, 0 };
//ULTRA VIOLET (UV) Dimming Values
byte uvled [96] = { 
  0, 0, 0, 0, 0, 0, 0, 0,                   //0 - 1
  0, 0, 0, 0, 0, 0, 0, 0,                   //2 - 3
  0, 0, 0, 0, 0, 0, 0, 0,                   //4 - 5
  0, 0, 0, 0, 0, 0, 0, 0,                   //6 - 7
  0, 0, 0, 0, 0, 0, 0, 0,                   //8 - 9
  0, 0, 0, 0, 20, 25, 30, 35,               //10 - 11
  40, 45, 50, 55, 60, 65, 70, 70,           //12 - 13
  65, 60, 55, 50, 45, 40, 35, 30,           //14 - 15
  25, 20, 0, 0, 0, 0, 0, 0,                 //16 - 17
  0, 0, 0, 0, 0, 0, 0, 0,                   //18 - 19
  0, 0, 0, 0, 0, 0, 0, 0,                   //20 - 21
  0, 0, 0, 0, 0, 0, 0, 0 };
byte tled [96];     //Temporary Array to Hold changed LED Values

/**************************** CHOOSE OPTION MENU1 BUTTONS *****************************/
const int tanD [] = { 
  10, 29, 155, 59 };        //"TIME and DATE" settings
const int temC [] = { 
  10, 69, 155, 99 };        //"H2O TEMP CONTROL" settings
const int wave [] = { 
  10, 109, 155, 139 };      //"Wavemaker CONTROL" settings
const int gSet [] = { 
  10, 149, 155, 179 };      //"GENERAL SETTINGS" page
const int tesT [] = { 
  165, 29, 310, 59 };       //"LED TESTING OPTIONS" menu
const int ledChM [] = { 
  165, 69, 310, 99 };     //"CHANGE LED VALUES" menu
const int aFeed [] = { 
  165, 109, 310, 139 };    //"AUTOMATIC FEEDER" menu
const int about [] = { 
  165, 149, 310, 179 };    //"ABOUT" program information
/**************************** CHOOSE OPTION MENU2 BUTTONS *****************************/
const int dosingPumpButton [] = { 
  10, 29, 155, 59 };        //"Dosing Pump" settings
/**************************** Dosing Pump Main BUTTONS *****************************/
//const int about [] = { 165, 149, 310, 179 };    //"ABOUT" program information
// defined in function
/**************************** TIME AND DATE SCREEN BUTTONS ***************************/
const int houU [] = { 
  110, 22, 135, 47 };       //hour up
const int minU [] = { 
  180, 22, 205, 47 };       //min up
const int ampmU [] = { 
  265, 22, 290, 47 };      //AM/PM up
const int houD [] = { 
  110, 73, 135, 96 };       //hour down
const int minD [] = { 
  180, 73, 205, 96 };       //min down
const int ampmD [] = { 
  265, 73, 290, 96 };      //AM/PM down
const int dayU [] = { 
  110, 112, 135, 137 };     //day up
const int monU [] = { 
  180, 112, 205, 137 };     //month up
const int yeaU [] = { 
  265, 112, 290, 137 };     //year up
const int dayD [] = { 
  110, 162, 135, 187 };     //day down
const int monD [] = { 
  180, 162, 205, 187 };     //month down
const int yeaD [] = { 
  265, 162, 290, 187 };     //year down
/*************************** H2O TEMP CONTROL SCREEN BUTTONS *************************/
const int temM [] = { 
  90, 49, 115, 74 };        //temp. minus
const int temP [] = { 
  205, 49, 230, 74 };       //temp. plus
const int offM [] = { 
  90, 99, 115, 124 };       //offset minus
const int offP [] = { 
  205, 99, 230, 124 };      //offset plus
const int almM [] = { 
  90, 149, 115, 174 };      //alarm minus
const int almP [] = { 
  205, 149, 230, 174 };     //alarm plus
/**************************** LED TESTING MENU BUTTONS *******************************/
const int tstLA [] = { 
  40, 59, 280, 99 };      //"Test LED Array Output" settings
const int cntIL [] = { 
  40, 109, 280, 149 };    //"Control Individual Leds" settings
/********************** TEST LED ARRAY OUTPUT SCREEN BUTTONS *************************/
const int stsT [] = { 
  110, 105, 200, 175 };     //start/stop
const int tenM [] = { 
  20, 120, 90, 160 };       //-10s
const int tenP [] = { 
  220, 120, 290, 160 };     //+10s
/******************** TEST INDIVIDUAL LED VALUES SCREEN BUTTONS **********************/
//These Buttons are made within the function
/****************** CHANGE INDIVIDUAL LED VALUES SCREEN BUTTONS **********************/
//These Buttons are made within the function
/************************* CHANGE LED VALUES MENU BUTTONS ****************************/
const int btCIL [] = { 
  5, 188, 90, 220 };       //back to Change Individual LEDs Screen
const int ledChV [] = { 
  110, 200, 210, 220 };   //LED Change Values
const int eeprom [] = { 
  215, 200, 315, 220 };   //Save to EEPROM (Right Button)
const int miM [] = { 
  90, 115, 115, 140 };       //MI minus
const int miP [] = { 
  205, 115, 230, 140 };      //MI plus
/********************* WAVEMAKER SCREEN & STETTINGS BUTTONS **************************/
//Many Wavemaker Buttons are made within the function(s)
const int pump1Mm [] = { 
  21, 70, 46, 95 };      //Pump 1 minute minus
const int pump1Mp [] = { 
  120, 70, 145, 95 };    //Pump 1 minute plus
const int pump1Sm [] = { 
  175, 70, 200, 95 };    //Pump 1 second minus
const int pump1Sp [] = { 
  274, 70, 299, 95 };    //Pump 1 second plus
const int pump2Mm [] = { 
  21, 147, 46, 172 };    //Pump 2 minute minus
const int pump2Mp [] = { 
  120, 147, 145, 172 };  //Pump 2 minute plus
const int pump2Sm [] = { 
  175, 147, 200, 172 };  //Pump 2 second minus
const int pump2Sp [] = { 
  274, 147, 299, 172 };  //Pump 2 second plus
/************************* AUTOMATIC FISH FEEDER BUTTONS *****************************/
//These Buttons are made within the function
/******************* SET AUTOMATIC FISH FEEDING TIMES BUTTONS ************************/
const int houP [] = { 
  110, 38, 135, 63 };       //hour up
const int minP [] = { 
  180, 38, 205, 63 };       //min up
const int ampmP [] = { 
  265, 38, 290, 63 };      //AM/PM up
const int houM [] = { 
  110, 89, 135, 114 };      //hour down
const int minM [] = { 
  180, 89, 205, 114 };      //min down
const int ampmM [] = { 
  265, 89, 290, 114 };     //AM/PM down
/***************************** MISCELLANEOUS BUTTONS *********************************/
const int back [] = { 
  5, 200, 105, 220 };       //BACK
const int prSAVE [] = { 
  110, 200, 210, 220 };   //SAVE or NEXT
const int canC [] = { 
  215, 200, 315, 220 };     //CANCEL
/***************************** DOSING PUMP   BUTTONS *********************************/
//const int
const int calibrateDosingPump [] = { 
  60, 20, 240, 40 };     //CANCEL
const int dosingHour1up [] = { 
  30, 45, 50, 67 };     //plus
const int dosingHour1down [] = { 
  30, 73, 50, 95 };     //plus

const int dosingHour2up [] = { 
  30, 98, 50, 118 };     //plus
const int dosingHour2down [] = { 
  30, 122, 50, 142 };     //plus

const int dosingHour3up [] = { 
  30, 144, 50, 164 };     //plus
const int dosingHour3down [] = { 
  30, 170, 50, 190 };     //plus

const int dosingMinute1up [] = { 
  110, 45, 130, 67 };     //plus
const int dosingMinute1down [] = { 
  110, 73, 130, 95 };     //plus

const int dosingMinute2up [] = { 
  110, 98, 130, 118 };     //plus
const int dosingMinute2down [] = { 
  110, 122, 130, 142 };     //plus

const int dosingMinute3up [] = { 
  110, 144, 130, 164 };     //plus
const int dosingMinute3down [] = { 
  110, 170, 130, 190 };     //plus

const int dosingML1up [] = { 
  135, 45, 157, 67 };     //plus
const int dosingML1down [] = { 
  135, 73, 157, 95 };     //plus

const int dosingML2up [] = { 
  135, 98, 157, 118 };     //plus
const int dosingML2down [] = { 
  135, 122, 157, 142 };     //plus

const int dosingML3up [] = { 
  135, 144, 157, 164 };     //plus
const int dosingML3down [] = { 
  135, 170, 157, 190 };     //plus

const int dosingSubML1up [] = { 
  225, 45, 247, 67 };     //plus
const int dosingSubML1down [] = { 
  225, 73, 247, 95 };     //plus

const int dosingSubML2up [] = { 
  225, 98, 247, 118 };     //plus
const int dosingSubML2down [] = { 
  225, 122, 247, 142 };     //plus

const int dosingSubML3up [] = { 
  225, 144, 247, 164 };     //plus
const int dosingSubML3down [] = { 
  225, 170, 247, 190 };     //plus

const int dosingPump1Off [] = { 
  265, 55, 315, 85 };
const int dosingPump2Off [] = { 
  265, 110, 315, 140 };
const int dosingPump3Off [] = { 
  265, 154, 315, 180 };

const int dosingCalibrationStart [] = { 
  60, 20, 240, 60 };

const int dosingCalibrationMLup [] = { 
  135, 144, 157, 164 };     //plus
const int dosingCalibrationMLdown [] = { 
  135, 170, 157, 190 };     //plus

const int dosingCalibrationSubMLup [] = { 
  225, 144, 247, 164 };     //plus
const int dosingCalibrationSubMLdown [] = { 
  225, 170, 247, 190 };     //plus

/**************************** END OF PRIMARY BUTTONS *********************************/

/********************************* EEPROM FUNCTIONS ***********************************/
struct config_t {
  int tempset;
  int tempFset;
  int tempoff;
  int tempFoff;
  int tempalarm;
  int tempFalarm;
} 
tempSettings;  //640 - 660

struct config_m {
  int MI_t;
} 
MIsettings;  // 600 -620

struct config_w {
  int waveMode;
  int altPump1m;
  int altPump1s;
  int altPump2m;
  int altPump2s;
  int synchMode;
  int synchPumpOnM;
  int synchPumpOnS;
  int synchPumpOffM;
  int synchPumpOffS;
} 
WAVEsettings;  // 620 - 640

struct config_g {
  int calendarFormat;
  int timeFormat;
  int tempScale;
  int SCREENsaver;
  int autoStop;
} 
GENERALsettings;  //660 - 680

struct config_f {
  int feedFish1h;
  int feedFish1m;
  int feedFish2h;
  int feedFish2m;
  int feedFish3h;
  int feedFish3m;
  int feedFish4h;
  int feedFish4m;
  int feedTime1;
  int feedTime2;
  int feedTime3;
  int feedTime4;
} 
FEEDERsettings;  //680 - 700

struct config_d {
  int mlPerTimePump1;  // ML per testtime for pump 1. real value is this / 10 (so no float is needed)
  int mlPerTimePump2;  // ML per testtime for pump 2. real value is this / 10 (so no float is needed)
  int mlPerTimePump3;  // ML per testtime for pump 3. real value is this / 10 (so no float is needed)
  int mlPerTimePump4;  // ML per testtime for pump 4. real value is this / 10 (so no float is needed)
  int pump1On;
  int pump2On;
  int pump3On;
  int pump4On;  // values 0 = deactived;int  1 = one pump time;int  2 = two pump time;int  ... till 3
  int pump1_1h;
  int pump1_1m;
  int pump1_2h;
  int pump1_2m;
  int pump1_3h;
  int pump1_3m;  // time values for pump1
  int pump2_1h;
  int pump2_1m;
  int pump2_2h;
  int pump2_2m;
  int pump2_3h;
  int pump2_3m;  // time values for pump2
  int pump3_1h;
  int pump3_1m;
  int pump3_2h;
  int pump3_2m;
  int pump3_3h;
  int pump3_3m;  // time values for pump3
  int pump4_1h;
  int pump4_1m;
  int pump4_2h;
  int pump4_2m;
  int pump4_3h;
  int pump4_3m;  // time values for pump4
  int pump1_1ml;
  int pump1_2ml;
  int pump1_3ml;  // ml values for pump 1 at given times
  int pump2_1ml;
  int pump2_2ml;
  int pump2_3ml;  // ml values for pump 1 at given times
  int pump3_1ml;
  int pump3_2ml;
  int pump3_3ml;  // ml values for pump 1 at given times
  int pump4_1ml;
  int pump4_2ml;
  int pump4_3ml;

} 
DOSINGSettings;  // 24 ints -> 48 byte; 700 - 760

void SaveLEDToEEPROM () {  // 0 - 600
  EEPROM.write ( 0, 123 );         //to determine if data available in EEPROM
  for ( int i = 1; i < 97; i++ ) {
    EEPROM.write ( i + ( 96 * 0 ), wled [i] );
    EEPROM.write ( i + ( 96 * 1 ), bled [i] );
    EEPROM.write ( i + ( 96 * 2 ), rbled [i] );
    EEPROM.write ( i + ( 96 * 3 ), rled [i] );
    EEPROM.write ( i + ( 96 * 4 ), sled [i] );
    EEPROM.write ( i + ( 96 * 5 ), uvled [i] );
  }
}

void SaveMoonLEDToEEPROM () {
  MIsettings.MI_t = int ( MI );
  EEPROM_writeAnything ( 600, MIsettings );
}

void SaveWaveToEEPROM () {
  WAVEsettings.waveMode = int ( WAVE );
  WAVEsettings.altPump1m = int ( Pump1m );
  WAVEsettings.altPump1s = int ( Pump1s );
  WAVEsettings.altPump2m = int ( Pump2m );
  WAVEsettings.altPump2s = int ( Pump2s );
  WAVEsettings.synchMode = int ( Synch );
  WAVEsettings.synchPumpOnM = int ( OnForTm );
  WAVEsettings.synchPumpOnS = int ( OnForTs );
  WAVEsettings.synchPumpOffM = int ( OffForTm );
  WAVEsettings.synchPumpOffS = int ( OffForTs );
  EEPROM_writeAnything ( 620, WAVEsettings );
}

void SaveTempToEEPROM () {
  tempSettings.tempset = int ( setTempC * 10 );
  tempSettings.tempFset = int ( setTempF * 10 );
  tempSettings.tempoff = int ( offTempC * 10 );
  tempSettings.tempFoff = int ( offTempF * 10 );
  tempSettings.tempalarm = int ( alarmTempC * 10 );
  tempSettings.tempFalarm = int ( alarmTempF * 10 );
  EEPROM_writeAnything ( 640, tempSettings );
}

void SaveGenSetsToEEPROM () {
  GENERALsettings.calendarFormat = int ( setCalendarFormat );
  GENERALsettings.timeFormat = int ( setTimeFormat );
  GENERALsettings.tempScale = int ( setTempScale );
  GENERALsettings.SCREENsaver = int ( setScreensaver );
  GENERALsettings.autoStop = int ( setAutoStop );
  EEPROM_writeAnything ( 660, GENERALsettings );
}

void SaveFeedTimesToEEPROM () {
  FEEDERsettings.feedFish1h = int ( feedFish1H );
  FEEDERsettings.feedFish1m = int ( feedFish1M );
  FEEDERsettings.feedFish2h = int ( feedFish2H );
  FEEDERsettings.feedFish2m = int ( feedFish2M );
  FEEDERsettings.feedFish3h = int ( feedFish3H );
  FEEDERsettings.feedFish3m = int ( feedFish3M );
  FEEDERsettings.feedFish4h = int ( feedFish4H );
  FEEDERsettings.feedFish4m = int ( feedFish4M );
  FEEDERsettings.feedTime1 = int ( FEEDTime1 );
  FEEDERsettings.feedTime2 = int ( FEEDTime2 );
  FEEDERsettings.feedTime3 = int ( FEEDTime3 );
  FEEDERsettings.feedTime4 = int ( FEEDTime4 );
  EEPROM_writeAnything ( 680, FEEDERsettings );
}

void SaveDosingPumpToEEPROM () {
  DOSINGSettings.mlPerTimePump1 = mlPerTimePump1;  // ML per testtime for pump 1. real value is this / 10 (so no float is needed)
  DOSINGSettings.mlPerTimePump2 = mlPerTimePump2;  // ML per testtime for pump 2. real value is this / 10 (so no float is needed)
  DOSINGSettings.mlPerTimePump3 = mlPerTimePump3;  // ML per testtime for pump 3. real value is this / 10 (so no float is needed)
  DOSINGSettings.mlPerTimePump4 = mlPerTimePump4;  // ML per testtime for pump 4. real value is this / 10 (so no float is needed)
  DOSINGSettings.pump1On = pump1On;
  DOSINGSettings.pump2On = pump2On;
  DOSINGSettings.pump3On = pump3On;
  DOSINGSettings.pump4On = pump4On;  // values 0 = deactived;DOSINGSettings. 1 = one pump time;DOSINGSettings. 2 = two pump time;DOSINGSettings. ... till 3
  DOSINGSettings.pump1_1h = pump1_1h;
  DOSINGSettings.pump1_1m = pump1_1m;
  DOSINGSettings.pump1_2h = pump1_2h;
  DOSINGSettings.pump1_2m = pump1_2m;
  DOSINGSettings.pump1_3h = pump1_3h;
  DOSINGSettings.pump1_3m = pump1_3m;  // time values for pump1
  DOSINGSettings.pump2_1h = pump2_1h;
  DOSINGSettings.pump2_1m = pump2_1m;
  DOSINGSettings.pump2_2h = pump2_2h;
  DOSINGSettings.pump2_2m = pump2_2m;
  DOSINGSettings.pump2_3h = pump2_3h;
  DOSINGSettings.pump2_3m = pump2_3m;
  DOSINGSettings.pump3_1h = pump3_1h;
  DOSINGSettings.pump3_1m = pump3_1m;
  DOSINGSettings.pump3_2h = pump3_2h;
  DOSINGSettings.pump3_2m = pump3_2m;
  DOSINGSettings.pump3_3h = pump3_3h;
  DOSINGSettings.pump3_3m = pump3_3m;
  DOSINGSettings.pump4_1h = pump4_1h;
  DOSINGSettings.pump4_1m = pump4_1m;
  DOSINGSettings.pump4_2h = pump4_2h;
  DOSINGSettings.pump4_2m = pump4_2m;
  DOSINGSettings.pump4_3h = pump4_3h;
  DOSINGSettings.pump4_3m = pump4_3m;
  DOSINGSettings.pump1_1ml = pump1_1ml;
  DOSINGSettings.pump1_2ml = pump1_2ml;
  DOSINGSettings.pump1_3ml = pump1_3ml;  // ml values for pump 1 at given times
  DOSINGSettings.pump2_1ml = pump2_1ml;
  DOSINGSettings.pump2_2ml = pump2_2ml;
  DOSINGSettings.pump2_3ml = pump2_3ml;  // ml values for pump 1 at given times
  DOSINGSettings.pump3_1ml = pump3_1ml;
  DOSINGSettings.pump3_2ml = pump3_2ml;
  DOSINGSettings.pump3_3ml = pump3_3ml;  // ml values for pump 1 at given times
  DOSINGSettings.pump4_1ml = pump4_1ml;
  DOSINGSettings.pump4_2ml = pump4_2ml;
  DOSINGSettings.pump4_3ml = pump4_3ml;
  EEPROM_writeAnything ( 700, DOSINGSettings );
}
void ReadFromEEPROM () {
  int k = EEPROM.read ( 0 );
  char tempString [3];

  if ( k == 123 ) {
    for ( int i = 1; i < 97; i++ ) {
      wled [i] = EEPROM.read ( i + ( 96 * 0 ) );
      bled [i] = EEPROM.read ( i + ( 96 * 1 ) );
      rbled [i] = EEPROM.read ( i + ( 96 * 2 ) );
      rled [i] = EEPROM.read ( i + ( 96 * 3 ) );
      sled [i] = EEPROM.read ( i + ( 96 * 4 ) );
      uvled [i] = EEPROM.read ( i + ( 96 * 5 ) );
    }
  }

  EEPROM_readAnything ( 600, MIsettings );
  MI = MIsettings.MI_t;

  EEPROM_readAnything ( 620, WAVEsettings );
  WAVE = WAVEsettings.waveMode;
  Pump1m = WAVEsettings.altPump1m;
  Pump1s = WAVEsettings.altPump1s;
  Pump2m = WAVEsettings.altPump2m;
  Pump2s = WAVEsettings.altPump2s;
  Synch = WAVEsettings.synchMode;
  OnForTm = WAVEsettings.synchPumpOnM;
  OnForTs = WAVEsettings.synchPumpOnS;
  OffForTm = WAVEsettings.synchPumpOffM;
  OffForTs = WAVEsettings.synchPumpOffS;

  EEPROM_readAnything ( 640, tempSettings );
  setTempC = tempSettings.tempset;
  setTempC /= 10;
  setTempF = tempSettings.tempFset;
  setTempF /= 10;
  offTempC = tempSettings.tempoff;
  offTempC /= 10;
  offTempF = tempSettings.tempFoff;
  offTempF /= 10;
  alarmTempC = tempSettings.tempalarm;
  alarmTempC /= 10;
  alarmTempF = tempSettings.tempFalarm;
  alarmTempF /= 10;

  EEPROM_readAnything ( 660, GENERALsettings );
  setCalendarFormat = GENERALsettings.calendarFormat;
  setTimeFormat = GENERALsettings.timeFormat;
  setTempScale = GENERALsettings.tempScale;
  setScreensaver = GENERALsettings.SCREENsaver;
  setAutoStop = GENERALsettings.autoStop;

  EEPROM_readAnything ( 680, FEEDERsettings );
  feedFish1H = FEEDERsettings.feedFish1h;
  feedFish1M = FEEDERsettings.feedFish1m;
  feedFish2H = FEEDERsettings.feedFish2h;
  feedFish2M = FEEDERsettings.feedFish2m;
  feedFish3H = FEEDERsettings.feedFish3h;
  feedFish3M = FEEDERsettings.feedFish3m;
  feedFish4H = FEEDERsettings.feedFish4h;
  feedFish4M = FEEDERsettings.feedFish4m;
  FEEDTime1 = FEEDERsettings.feedTime1;
  FEEDTime2 = FEEDERsettings.feedTime2;
  FEEDTime3 = FEEDERsettings.feedTime3;
  FEEDTime4 = FEEDERsettings.feedTime4;

  EEPROM_readAnything ( 700, DOSINGSettings );
  mlPerTimePump1 = DOSINGSettings.mlPerTimePump1;  // ML per testtime for pump 1. real value is this / 10 (so no float is needed)
  mlPerTimePump2 = DOSINGSettings.mlPerTimePump2;  // ML per testtime for pump 2. real value is this / 10 (so no float is needed)
  mlPerTimePump3 = DOSINGSettings.mlPerTimePump3;  // ML per testtime for pump 3. real value is this / 10 (so no float is needed)
  mlPerTimePump4 = DOSINGSettings.mlPerTimePump4;  // ML per testtime for pump 4. real value is this / 10 (so no float is needed)
  pump1On = DOSINGSettings.pump1On;
  pump2On = DOSINGSettings.pump2On;
  pump3On = DOSINGSettings.pump3On;
  pump4On = DOSINGSettings.pump4On;  // values 0 =DOSINGSettings. deactived; 1 =DOSINGSettings. one pump time; 2 =DOSINGSettings. two pump time; ... till 3
  pump1_1h = DOSINGSettings.pump1_1h;
  pump1_1m = DOSINGSettings.pump1_1m;
  pump1_2h = DOSINGSettings.pump1_2h;
  pump1_2m = DOSINGSettings.pump1_2m;
  pump1_3h = DOSINGSettings.pump1_3h;
  //	Serial.println ( " Dosing pump settings read" );
  pump1_3m = DOSINGSettings.pump1_3m;  // time values for pump1
  pump2_1h = DOSINGSettings.pump2_1h;
  pump2_1m = DOSINGSettings.pump2_1m;
  pump2_2h = DOSINGSettings.pump2_2h;
  pump2_2m = DOSINGSettings.pump2_2m;
  pump2_3h = DOSINGSettings.pump2_3h;
  pump2_3m = DOSINGSettings.pump2_3m;
  pump3_1h = DOSINGSettings.pump3_1h;
  pump3_1m = DOSINGSettings.pump3_1m;
  pump3_2h = DOSINGSettings.pump3_2h;
  pump3_2m = DOSINGSettings.pump3_2m;
  pump3_3h = DOSINGSettings.pump3_3h;
  pump3_3m = DOSINGSettings.pump3_3m;
  pump4_1h = DOSINGSettings.pump4_1h;
  pump4_1m = DOSINGSettings.pump4_1m;
  pump4_2h = DOSINGSettings.pump4_2h;
  pump4_2m = DOSINGSettings.pump4_2m;
  pump4_3h = DOSINGSettings.pump4_3h;
  pump4_3m = DOSINGSettings.pump4_3m;
  pump1_1ml = DOSINGSettings.pump1_1ml;
  pump1_2ml = DOSINGSettings.pump1_2ml;
  pump1_3ml = DOSINGSettings.pump1_3ml;  // ml values for pump 1 at given times
  pump2_1ml = DOSINGSettings.pump2_1ml;
  pump2_2ml = DOSINGSettings.pump2_2ml;
  pump2_3ml = DOSINGSettings.pump2_3ml;  // ml values for pump 1 at given times
  pump3_1ml = DOSINGSettings.pump3_1ml;
  pump3_2ml = DOSINGSettings.pump3_2ml;
  pump3_3ml = DOSINGSettings.pump3_3ml;  // ml values for pump 1 at given times
  pump4_1ml = DOSINGSettings.pump4_1ml;
  pump4_2ml = DOSINGSettings.pump4_2ml;
  pump4_3ml = DOSINGSettings.pump4_3ml;
}
/***************************** END OF EEPROM FUNCTIONS ********************************/

/********************************** RTC FUNCTIONS *************************************/
void SaveRTC () {
  int year = rtcSet [6] - 2000;

  RTC.stop ();            //RTC clock setup
  RTC.set ( DS1307_SEC, 1 );
  RTC.set ( DS1307_MIN, rtcSet [1] );
  RTC.set ( DS1307_HR, rtcSet [2] );
  //RTC.set(DS1307_DOW,1);
  RTC.set ( DS1307_DATE, rtcSet [4] );
  RTC.set ( DS1307_MTH, rtcSet [5] );
  RTC.set ( DS1307_YR, year );
  delay ( 10 );
  RTC.start ();
  delay ( 10 );
  for ( int i = 0; i < 56; i++ ) {
    RTC.set_sram_byte ( 65, i );
  }
  delay ( 50 );
}
/********************************* END OF RTC FUNCTIONS *******************************/

/********************************** TIME AND DATE BAR **********************************/
void TimeDateBar ( boolean refreshAll = false ) {
  char oldVal [16], minute [3], stunde [3], ampm [6], month [5];

  if ( ( rtc [1] >= 0 ) && ( rtc [1] <= 9 ) ) {
    sprintf ( minute, "%i%i", 0, rtc [1] );
    //    rtc1= '0' + String(rtc[1]);
  }               //adds 0 to minutes
  else {
    sprintf ( minute, "%i", rtc [1] );
    //    rtc1= String(rtc[1]);
  }

  if ( setTimeFormat == 1 ) {
    if ( rtc [2] == 0 ) {
      sprintf ( stunde, "%i", 12 );
      //      rtc2=String( 12);
    }                //12 HR Format
    else {
      if ( rtc [2] > 12 ) {
        sprintf ( stunde, "%i", rtc [2] - 12 );
        //        rtc2= String( rtc[2]-12);
      }
      else {
        sprintf ( stunde, "%i", rtc [2] );
        //        rtc2= String(rtc[2]);
      }
    }
  }

  if ( rtc [2] < 12 ) {
    sprintf ( ampm, " AM  " );
    //    ampm=" AM  ";
  }              //Adding the AM/PM sufffix
  else {
    sprintf ( ampm, " PM  " );
    //    ampm= " PM  ";
  }

  sprintf ( oldVal, "%i", time );                                 //refresh time if different
  if ( setTimeFormat == 1 ) {
    sprintf ( time, "%s:%s%s", stunde, minute, ampm );
    //    time= rtc2 + ':' + rtc1 + ampm;
  }
  else {
    sprintf ( time, " %i:%s      ", rtc [2], minute );
    //    time= " " + String(rtc[2]) + ':' + rtc1 +"      ";
  }
  if ( ( oldVal != time ) || refreshAll ) {
    //char bufferT[9];
    //  time.toCharArray(bufferT, 9);               //String to Char array
    setFont ( SMALL, 255, 255, 0, 64, 64, 64 );
    myGLCD.print ( time, 215, 227 );            //Display time
  }

  if ( rtc [5] == 1 ) {
    sprintf ( month, "JAN " );
  }             //Convert the month to its name
  if ( rtc [5] == 2 ) {
    sprintf ( month, "FEB " );
  }
  if ( rtc [5] == 3 ) {
    sprintf ( month, "MAR " );
  }
  if ( rtc [5] == 4 ) {
    sprintf ( month, "APR " );
  }
  if ( rtc [5] == 5 ) {
    sprintf ( month, "MAY " );
  }
  if ( rtc [5] == 6 ) {
    sprintf ( month, "JUN " );
  }
  if ( rtc [5] == 7 ) {
    sprintf ( month, "JLY " );
  }
  if ( rtc [5] == 8 ) {
    sprintf ( month, "AUG " );
  }
  if ( rtc [5] == 9 ) {
    sprintf ( month, "SEP " );
  }
  if ( rtc [5] == 10 ) {
    sprintf ( month, "OCT " );
  }
  if ( rtc [5] == 11 ) {
    sprintf ( month, "NOV " );
  }
  if ( rtc [5] == 12 ) {
    sprintf ( month, "DEC " );
  }

  sprintf ( oldVal, "%s", date );                                 //refresh date if different
  //  date.reserve(24);
  if ( setCalendarFormat == 0 ) {
    sprintf ( date, "  %i/%i/%i   ", rtc [4], rtc [5], rtc [6] );
    //    date= "  " + String(rtc[4]) + "/" + String(rtc[5]) + "/" + String(rtc[6]) + "   ";

  }
  else {
    sprintf ( date, "  %s%i, %i", month, rtc [4], rtc [6] );
    //    date= "  " + month + String(rtc[4]) + ',' + ' ' + String(rtc[6]);
  }
  if ( ( oldVal != date ) || refreshAll ) {
    //    char bufferD[15];
    //    date.toCharArray(bufferD, 15);              //String to Char array
    setFont ( SMALL, 255, 255, 0, 64, 64, 64 );
    myGLCD.print ( date, 20, 227 );             //Display date
  }
}
/****************************** END OF TIME AND DATE BAR ******************************/

/************************************* LED LEVELS *************************************/
void LED_levels_output () {
  int sector, sstep, t1, t2;
  int s_out, b_out, w_out, rb_out, r_out, uv_out, moon_out;

  if ( min_cnt >= 1440 ) {
    min_cnt = 1;
  }   // 24 hours of minutes
  sector = min_cnt / 15;              // divided by gives sector -- 15 minute
  sstep = min_cnt % 15;               // remainder gives add on to sector value

  t1 = sector;
  if ( t1 == 95 ) {
    t2 = 0;
  }
  else {
    t2 = t1 + 1;
  }

  if ( colorLEDtest ) {
    sled_out = scol_out;
    rled_out = rcol_out;
    wled_out = wcol_out;
    bled_out = bcol_out;
    rbled_out = rbcol_out;
    uvled_out = uvcol_out;
    moonled_out = mooncol_out;
  }
  else {
    if ( sstep == 0 ) {
      sled_out = sled [t1];
      bled_out = bled [t1];
      wled_out = wled [t1];
      rbled_out = rbled [t1];
      rled_out = rled [t1];
      uvled_out = uvled [t1];
    }
    else {
      sled_out = check ( &sled [t1], &sled [t2], sstep );
      bled_out = check ( &bled [t1], &bled [t2], sstep );
      wled_out = check ( &wled [t1], &wled [t2], sstep );
      rbled_out = check ( &rbled [t1], &rbled [t2], sstep );
      rled_out = check ( &rled [t1], &rled [t2], sstep );
      uvled_out = check ( &uvled [t1], &uvled [t2], sstep );
    }
    float lunarCycle = moonPhase ( rtc [6], rtc [5], rtc [4] );  //get a value for the lunar cycle
    moonled_out = MoonLow * ( 1 - lunarCycle ) + MI * lunarCycle + 0.5;                  //MaximumIllumination * % of Full Moon (0-100)
  }

  if ( RECOM_RCD ) {
    s_out = sled_out;
    b_out = bled_out;
    w_out = wled_out;
    rb_out = rbled_out;
    r_out = rled_out;
    uv_out = uvled_out;
    moon_out = moonled_out;
  }
  else {
    s_out = 255 - sled_out;
    b_out = 255 - bled_out;
    w_out = 255 - wled_out;
    rb_out = 255 - rbled_out;
    r_out = 255 - rled_out;
    uv_out = 255 - uvled_out;
    moon_out = 255 - moonled_out;
  }

  //  analogWrite(ledPinSump, s_out);
  //  analogWrite(ledPinBlue, b_out);
  //  analogWrite(ledPinWhite, w_out);
  //  analogWrite(ledPinRoyBlue, rb_out);
  //  analogWrite(ledPinRed, r_out);
  //  analogWrite(ledPinUV, uv_out);
  //  analogWrite(ledPinMoon, moon_out);

  setzeDimmWerte ( w_out, b_out );
}

int check ( byte *pt1, byte *pt2, int lstep ) {
  int result;
  float fresult;

  if ( *pt1 == *pt2 ) {
    result = *pt1;
  }     // No change
  else if ( *pt1 < *pt2 )                //Increasing brightness
  {
    fresult = ( ( float ( *pt2 - *pt1 ) / 15.0 ) * float ( lstep ) ) + float ( *pt1 );
    result = int ( fresult );
  }
  //Decreasing brightness
  else {
    fresult = - ( ( float ( *pt1 - *pt2 ) / 15.0 ) * float ( lstep ) ) + float ( *pt1 );
    result = int ( fresult );
  }
  return result;
}
/********************************* END OF LED LEVELS **********************************/

/************************************ WAVE OUTPUT *************************************/
void wave_output () {
  unsigned long currentMillis = millis ();

  if ( WAVE == 1 )                         //Alternating Mode
  {
    if ( waveMakerTest == true ) {
      wPump1 = ( ( Min1 * 60 ) + Sec1 * WaveCorrector );
      wPump1 = wPump1 * 1000;
      wPump2 = ( ( Min2 * 60 ) + Sec2 * WaveCorrector );
      wPump2 = wPump2 * 1000;
    }
    else {
      wPump1 = ( ( Pump1m * 60 ) + Pump1s * WaveCorrector );
      wPump1 = wPump1 * 1000;
      wPump2 = ( ( Pump2m * 60 ) + Pump2s * WaveCorrector );
      wPump2 = wPump2 * 1000;
    }

    if ( currentMillis - previousMillisWave > intervalAlt ) {
      previousMillisWave = currentMillis;

      if ( wPump1 == wPump2 ) {
        wPump2 = wPump2 + 1;
      }

      if ( intervalAlt == wPump1 ) {
        intervalAlt = wPump2;
        PumpTstate = LOW;
        PumpBstate = HIGH;
      }
      else {
        intervalAlt = wPump1;
        PumpTstate = HIGH;
        PumpBstate = LOW;
      }
    }
  }
  if ( ( WAVE == 2 ) && ( Synch == 1 ) )         //Synchronous - Constanly ON
  {
    PumpTstate = HIGH;
    PumpBstate = HIGH;
  }
  else

      if ( ( WAVE == 2 ) && ( Synch == 2 ) )         //Synchronous - Pulsating
    {
      if ( waveMakerTest == true ) {
        wOnForT = ( ( Min1 * 60 ) + Sec1 * WaveCorrector );
        wOnForT = wOnForT * 1000;
        wOffForT = ( ( Min2 * 60 ) + Sec2 * WaveCorrector );
        wOffForT = wOffForT * 1000;
      }
      else {
        wOnForT = ( OnForTm * 60 ) + OnForTs * WaveCorrector;
        wOnForT = wOnForT * 1000;
        wOffForT = ( OffForTm * 60 ) + OffForTs * WaveCorrector;
        wOffForT = wOffForT * 1000;
      }

      if ( currentMillis - previousMillisWave > intervalSynch ) {
        previousMillisWave = currentMillis;

        if ( wOnForT == wOffForT ) {
          wOffForT = wOffForT + 1;
        }

        if ( intervalSynch == wOnForT ) {
          intervalSynch = wOffForT;
          PumpTstate = LOW;
          PumpBstate = LOW;
        }
        else {
          intervalSynch = wOnForT;
          PumpTstate = HIGH;
          PumpBstate = HIGH;
        }
      }
    }
  digitalWrite ( WaveMakerTop, PumpTstate );
  digitalWrite ( WaveMakerBottom, PumpBstate );
}
/********************************* END OF WAVE OUTPUT *********************************/

/******************************** TEMPERATURE FUNCTIONS *******************************/
void checkTempC () {
  if ( checkTemp ) {
    sensors.requestTemperatures ();   // call sensors.requestTemperatures() to issue a global
    // temperature request to all devices on the bus
    tempW = ( sensors.getTempC ( waterThermometer ) );  //read water temperature
    tempH = ( sensors.getTempC ( hoodThermometer ) );   //read hood's heatsink temperature
    tempS = ( sensors.getTempC ( sumpThermometer ) );   //read sump's heatsink temperature

    if ( tempW < ( setTempC + offTempC + alarmTempC ) && tempW > ( setTempC - offTempC - alarmTempC ) ) {
      tempAlarmflag = false;
      digitalWrite ( tempAlarmPin, LOW );                           //turn off alarm
    }
    if ( tempW < ( setTempC + offTempC ) && tempW > ( setTempC - offTempC ) )  //turn off chiller/heater
    {
      tempCoolflag = false;
      tempHeatflag = false;
      digitalWrite ( tempHeatPin, LOW );
      digitalWrite ( tempChillPin, LOW );
    }
    if ( offTempC > 0 ) {
      if ( tempW >= ( setTempC + offTempC ) )            //turn on chiller
      {
        tempCoolflag = true;
        digitalWrite ( tempChillPin, HIGH );
      }
      if ( tempW <= ( setTempC - offTempC ) )             //turn an heater
      {
        tempHeatflag = true;
        digitalWrite ( tempHeatPin, HIGH );
      }
    }
    if ( alarmTempC > 0 )                              //turn on alarm
    {
      if ( ( tempW >= ( setTempC + offTempC + alarmTempC ) ) || ( tempW <= ( setTempC - offTempC - alarmTempC ) ) ) {
        tempAlarmflag = true;
        unsigned long cMillis = millis ();
        if ( cMillis - previousMillisAlarm > intervalAlarm ) {
          previousMillisAlarm = cMillis;
          digitalWrite ( tempAlarmPin, HIGH );
          delay ( 1000 );
          digitalWrite ( tempAlarmPin, LOW );
        }
      }
    }

    //Fan Controller for Hood
    HoodTempInterval = ( tempH - TempToBeginHoodFanInDegC );   //Sets the interval to start from 0
    HoodFanSpeedIncrease = HoodTempInterval * 0.1;   //Fan's speed increases 10% every degree over set temperature
    digitalWrite ( HoodFansTranzPin, HIGH );
    if ( tempH < TempToBeginHoodFanInDegC )          //If Temp's less than defined value, leave fan off
    {
      HoodPWM = 0;
      digitalWrite ( HoodFansTranzPin, LOW );
    }
    if ( ( tempH >= TempToBeginHoodFanInDegC ) && ( HoodFanSpeedIncrease < 1 ) )   //For every degree over defined value, increase by 10%
    {
      HoodPWM = FanOn + HoodFanSpeedIncrease;
    }
    if ( HoodFanSpeedIncrease >= 1 )                 //If the temperature is 10 or more degrees C higher than user
    {
      HoodPWM = 1;
    }                              //defined value to start, leave it at 100%

    //Fan Controller for Sump
    SumpTempInterval = ( tempS - TempToBeginSumpFanInDegC );   //Sets the interval to start from 0
    SumpFanSpeedIncrease = SumpTempInterval * 0.1;   //Fan's speed increases 10% every degree over set temperature
    digitalWrite ( SumpFanTranzPin, HIGH );
    if ( tempS < TempToBeginSumpFanInDegC )          //If Temp's less than defined value, leave fan off
    {
      SumpPWM = 0;
      digitalWrite ( SumpFanTranzPin, LOW );
    }
    if ( ( tempS >= TempToBeginSumpFanInDegC ) && ( SumpFanSpeedIncrease < 1 ) )   //For every degree over defined value, increase by 10%
    {
      SumpPWM = FanOn + SumpFanSpeedIncrease;
    }
    if ( SumpFanSpeedIncrease >= 1 )                 //If the temperature is 10 or more degrees C higher than user
    {
      SumpPWM = 1;
    }                              //defined value to start, leave it at 100%
  }
}
/*************************** END OF TEMPERATURE FUNCTIONS *****************************/

/******************************* LUNAR PHASE FUNCTION *********************************/
float moonPhase ( int moonYear, int moonMonth, int moonDay ) {
  float phase;
  double IP;
  long YY, MM, K1, K2, K3, JulianDay;
  YY = moonYear - floor ( ( 12 - moonMonth ) / 10 );
  MM = moonMonth + 9;
  if ( MM >= 12 ) {
    MM = MM - 12;
  }
  K1 = floor ( 365.25 * ( YY + 4712 ) );
  K2 = floor ( 30.6 * MM + 0.5 );
  K3 = floor ( floor ( ( YY / 100 ) + 49 ) * 0.75 ) - 38;
  JulianDay = K1 + K2 + moonDay + 59;
  if ( JulianDay > 2299160 ) {
    JulianDay = JulianDay - K3;
  }
  IP = MyNormalize ( ( JulianDay - 2451550.1 ) / LC );
  AG = IP * LC;
  phase = 0;

  //Determine the Moon Illumination %
  if ( ( AG >= 0 ) && ( AG <= LC / 2 ) )             //FROM New Moon 0% TO Full Moon 100%
  {
    phase = ( 2 * AG ) / LC;
  }
  if ( ( AG > LC / 2 ) && ( AG <= LC ) )             //FROM Full Moon 100% TO New Moon 0%
  {
    phase = 2 * ( LC - AG ) / LC;
  }

  //Determine the Lunar Phase
  if ( ( AG >= 0 ) && ( AG <= 1.85 ) )             //New Moon; ~0-12.5% illuminated
  {
    sprintf ( LP, "    New Moon   " );
    //    LP = "    New Moon   ";
    MoonPic = New_Moon;
  }
  if ( ( AG > 1.85 ) && ( AG <= 5.54 ) )           //New Crescent; ~12.5-37.5% illuminated
  {
    sprintf ( LP, "Waxing Crescent" );
    //    LP = "Waxing Crescent";
    MoonPic = Waxing_Crescent;
  }
  if ( ( AG > 5.54 ) && ( AG <= 9.23 ) )           //First Quarter; ~37.5-62.5% illuminated
  {
    sprintf ( LP, " First Quarter " );
    //    LP = " First Quarter ";
    MoonPic = First_Quarter;
  }
  if ( ( AG > 9.23 ) && ( AG <= 12.92 ) )          //Waxing Gibbous; ~62.5-87.5% illuminated
  {
    sprintf ( LP, "Waxing Gibbous " );
    //    LP = "Waxing Gibbous ";
    MoonPic = Waxing_Gibbous;
  }
  if ( ( AG > 12.92 ) && ( AG <= 16.61 ) )         //Full Moon; ~87.5-100-87.5% illuminated
  {
    sprintf ( LP, "   Full Moon   " );
    //    LP = "   Full Moon   ";
    MoonPic = Full_Moon;
  }
  if ( ( AG > 16.61 ) && ( AG <= 20.30 ) )         //Waning Gibbous; ~87.5-62.5% illuminated
  {
    sprintf ( LP, "Waning Gibbous " );
    //    LP = "Waning Gibbous ";
    MoonPic = Waning_Gibbous;
  }
  if ( ( AG > 20.30 ) && ( AG <= 23.99 ) )         //Last Quarter; ~62.5-37.5% illuminated
  {
    sprintf ( LP, " Last Quarter  " );
    //    LP = " Last Quarter  ";
    MoonPic = Last_Quarter;
  }
  if ( ( AG > 23.99 ) && ( AG <= 27.68 ) )         //Old Crescent; ~37.5-12.5% illuminated
  {
    sprintf ( LP, "Waning Crescent" );
    //    LP = "Waning Crescent";
    MoonPic = Waning_Crescent;
  }
  if ( ( AG >= 27.68 ) && ( AG <= LC ) )           //New Moon; ~12.5-0% illuminated
  {
    sprintf ( LP, "    New Moon   " );
    //    LP = "    New Moon   ";
    MoonPic = New_Moon;
  }

  return phase;
}

double MyNormalize ( double v ) {
  v = v - floor ( v );
  if ( v < 0 ) v = v + 1;
  return v;
}
/**************************** END OF LUNAR PHASE FUNCTION *****************************/

/********************************* MISC. FUNCTIONS ************************************/
void clearScreen () {
  myGLCD.setColor ( 0, 0, 0 );
  myGLCD.fillRect ( 1, 15, 318, 226 );
}

void printButton ( char* text, int x1, int y1, int x2, int y2, boolean fontsize = false, int buttonColor = 0 ) {
  int stl = strlen ( text );
  int fx, fy;

  switch ( buttonColor ) {
  case BUTTONCOLOR_BLUE :
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.setBackColor ( 0, 0, 255 );
    break;
  case BUTTONCOLOR_RED :
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.setBackColor ( 255, 0, 0 );
    break;
  case BUTTONCOLOR_GREEN :
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.setBackColor ( 0, 255, 0 );
    break;
  default :
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.setBackColor ( 0, 0, 255 );
    break;
  }
  myGLCD.fillRoundRect ( x1, y1, x2, y2 );
  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.drawRoundRect ( x1, y1, x2, y2 );

  if ( fontsize ) {
    myGLCD.setFont ( BigFont );
    fx = x1 + ( ( ( x2 - x1 + 1 ) - ( stl * 16 ) ) / 2 );
    fy = y1 + ( ( ( y2 - y1 + 1 ) - 16 ) / 2 );
    myGLCD.print ( text, fx, fy );
  }
  else {
    myGLCD.setFont ( SmallFont );
    fx = x1 + ( ( ( x2 - x1 ) - ( stl * 8 ) ) / 2 );
    fy = y1 + ( ( ( y2 - y1 - 1 ) - 8 ) / 2 );
    myGLCD.print ( text, fx, fy );
  }
}

void printLedChangerP ( char* text, int x1, int y1, int x2, int y2, boolean fontsize = false ) {
  int stl = strlen ( text );
  int fx, fy;

  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.fillRect ( x1, y1, x2, y2 );
  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.drawRect ( x1, y1, x2, y2 );

  myGLCD.setBackColor ( 255, 255, 255 );
  myGLCD.setColor ( 255, 0, 0 );
  if ( fontsize ) {
    myGLCD.setFont ( BigFont );
    fx = x1 + ( ( ( x2 - x1 + 1 ) - ( stl * 16 ) ) / 2 );
    fy = y1 + ( ( ( y2 - y1 + 1 ) - 16 ) / 2 );
    myGLCD.print ( text, fx, fy );
  }
  else {
    myGLCD.setFont ( SmallFont );
    fx = x1 + ( ( ( x2 - x1 ) - ( stl * 8 ) ) / 2 );
    fy = y1 + ( ( ( y2 - y1 - 1 ) - 8 ) / 2 );
    myGLCD.print ( text, fx, fy );
  }
}

void printLedChangerM ( char* text, int x1, int y1, int x2, int y2, boolean fontsize = false ) {
  int stl = strlen ( text );
  int fx, fy;

  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.fillRect ( x1, y1, x2, y2 );
  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.drawRect ( x1, y1, x2, y2 );

  myGLCD.setBackColor ( 255, 255, 255 );
  myGLCD.setColor ( 0, 0, 255 );
  if ( fontsize ) {
    myGLCD.setFont ( BigFont );
    fx = x1 + ( ( ( x2 - x1 + 1 ) - ( stl * 16 ) ) / 2 );
    fy = y1 + ( ( ( y2 - y1 + 1 ) - 16 ) / 2 );
    myGLCD.print ( text, fx, fy );
  }
  else {
    myGLCD.setFont ( SmallFont );
    fx = x1 + ( ( ( x2 - x1 ) - ( stl * 8 ) ) / 2 );
    fy = y1 + ( ( ( y2 - y1 - 1 ) - 8 ) / 2 );
    myGLCD.print ( text, fx, fy );
  }
}

void printHeader ( char* headline ) {
  setFont ( SMALL, 255, 255, 0, 255, 255, 0 );
  myGLCD.fillRect ( 1, 1, 318, 14 );
  myGLCD.setColor ( 0, 0, 0 );
  myGLCD.print ( headline, CENTER, 1 );
}

void setFont ( boolean font, byte cr, byte cg, byte cb, byte br, byte bg, byte bb ) {
  myGLCD.setBackColor ( br, bg, bb );               //font background black
  myGLCD.setColor ( cr, cg, cb );                   //font color white
  if ( font == LARGE )
    myGLCD.setFont ( BigFont );                 //font size LARGE
  else if ( font == SMALL ) myGLCD.setFont ( SmallFont );
}

void waitForIt ( int x1, int y1, int x2, int y2 )   // Draw a red frame while a button is touched
{
  myGLCD.setColor ( 255, 0, 0 );
  myGLCD.drawRoundRect ( x1, y1, x2, y2 );
  while ( myTouch.dataAvailable () ) {
    myTouch.read ();
  }
  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.drawRoundRect ( x1, y1, x2, y2 );
}

void waitForItSq ( int x1, int y1, int x2, int y2 )  // Draw a red frame while a button is touched
{
  myGLCD.setColor ( 255, 0, 0 );
  myGLCD.drawRect ( x1, y1, x2, y2 );
  while ( myTouch.dataAvailable () ) {
    myTouch.read ();
  }
  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.drawRect ( x1, y1, x2, y2 );
}

int LedToPercent ( int Led_out )                   //returns LED output in %
{
  int result;

  if ( Led_out == 0 ) {
    result = 0;
  }
  else {
    result = map ( Led_out, 1, 255, 1, 100 );
  }

  return result;
}

void drawBarGraph () {
  myGLCD.setColor ( 255, 255, 255 );                //LED Chart
  setFont ( SMALL, 255, 255, 255, 0, 0, 0 );

  myGLCD.drawRect ( 30, 137, 148, 138 );            //x-line
  myGLCD.drawRect ( 30, 137, 31, 36 );              //y-line

  for ( int i = 0; i < 5; i++ )                        //tick-marks
  {
    myGLCD.drawLine ( 31, ( i * 20 ) + 36, 35, ( i * 20 ) + 36 );
  }
  myGLCD.print ( "100", 4, 30 );
  myGLCD.print ( "80", 12, 50 );
  myGLCD.print ( "60", 12, 70 );
  myGLCD.print ( "40", 12, 90 );
  myGLCD.print ( "20", 12, 110 );
  myGLCD.print ( "0", 20, 130 );

  myGLCD.setColor ( 0, 150, 0 );
  myGLCD.drawRect ( 40, 136, 52, 135 );             //SUMP %bar place holder
  myGLCD.setColor ( 255, 0, 0 );
  myGLCD.drawRect ( 57, 136, 69, 135 );             //red %bar place holder
  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.drawRect ( 74, 136, 86, 135 );             //white %bar place holder
  myGLCD.setColor ( 9, 184, 255 );
  myGLCD.drawRect ( 91, 136, 103, 135 );            //blue %bar place holder
  myGLCD.setColor ( 58, 95, 205 );
  myGLCD.drawRect ( 108, 136, 120, 135 );           //royal %bar place holder
  myGLCD.setColor ( 224, 102, 255 );
  myGLCD.drawRect ( 125, 136, 137, 135 );           //UV %bar place holder
}

void drawBarandColorValue () {
  colorled_out = CL_100 + CL_10 + CL_1;
  setFont ( SMALL, Rback, Gback, Bback, Rback, Gback, Bback );
  myGLCD.print ( "    ", xValue, yValue );                  //fill over old
  setFont ( SMALL, Rfont, Gfont, Bfont, Rback, Gback, Bback );
  myGLCD.printNumI ( colorled_out, xValue, yValue );

  yBar = 136 - colorled_out * .39;
  myGLCD.setColor ( 0, 0, 0 );
  myGLCD.fillRect ( x1Bar, yBar, x2Bar, 36 );       //hide end of last bar
  myGLCD.setColor ( Rgad, Ggad, Bgad );
  myGLCD.fillRect ( x1Bar, 136, x2Bar, yBar );      //color percentage bar
  if ( COLOR == SUMP ) {
    scol_out = colorled_out;
  }
  if ( COLOR == RED ) {
    rcol_out = colorled_out;
  }
  if ( COLOR == WHITE ) {
    wcol_out = colorled_out;
  }
  if ( COLOR == BLUE ) {
    bcol_out = colorled_out;
  }
  if ( COLOR == ROYAL ) {
    rbcol_out = colorled_out;
  }
  if ( COLOR == ULTRA ) {
    uvcol_out = colorled_out;
  }
  if ( COLOR == MOON ) {
    mooncol_out = colorled_out;
  }
}

void ledChangerGadget () {
  myGLCD.setColor ( Rgad, Ggad, Bgad );
  myGLCD.fillRoundRect ( 199, 25, 285, 132 );       //Gadget Color
  myGLCD.setColor ( 0, 0, 0 );
  myGLCD.fillRect ( 204, 50, 280, 74 );             //Black Background of Numbers
  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.drawRoundRect ( 199, 25, 285, 132 );       //Outline Gadget
  myGLCD.setColor ( Rline, Gline, Bline );          //Line Color
  myGLCD.drawLine ( 199, 46, 285, 46 );
  setFont ( LARGE, Rfont, Gfont, Bfont, Rback, Gback, Bback );
  if ( COLOR == 0 ) {
    myGLCD.print ( "COLOR", 202, 28 );
  }
  if ( COLOR == 1 ) {
    myGLCD.print ( "WHITE", 202, 28 );
  }
  if ( COLOR == 2 ) {
    myGLCD.print ( "BLUE", 210, 28 );
  }
  if ( COLOR == 3 ) {
    myGLCD.print ( "ROYAL", 202, 28 );
  }
  if ( COLOR == 4 ) {
    myGLCD.print ( "RED", 218, 28 );
  }
  if ( COLOR == 5 ) {
    myGLCD.print ( "ULTRA", 202, 28 );
  }
  if ( COLOR == 6 ) {
    myGLCD.print ( "SUMP", 210, 28 );
  }
  if ( COLOR == 7 ) {
    myGLCD.print ( "LUNAR", 202, 28 );
  }
  for ( int b = 0; b < 3; b++ ) {
    printLedChangerP ( "+", ( b * 27 ) + 204, 78, ( b * 27 ) + 226, 100, LARGE );  //Press Increase Number
    printLedChangerM ( "-", ( b * 27 ) + 204, 105, ( b * 27 ) + 226, 127, LARGE );  //Press Decrease Number
  }
  for ( int c = 0; c < 3; c++ ) {
    myGLCD.setColor ( Rline, Gline, Bline );
    myGLCD.drawRect ( ( c * 27 ) + 204, 78, ( c * 27 ) + 226, 100 );
    myGLCD.drawRect ( ( c * 27 ) + 204, 105, ( c * 27 ) + 226, 127 );
  }
  setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
  myGLCD.printNumI ( cl_100, 214, 54 );
  myGLCD.printNumI ( cl_10, 234, 54 );
  myGLCD.printNumI ( cl_1, 255, 54 );
}

void TimeSaver ( boolean refreshAll = false ) {
  if ( setTimeFormat == 0 )                                 //24HR Format
  {
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.setBackColor ( 0, 0, 0 );
    myGLCD.setFont ( SevenSegNumFont );
    if ( ( rtc [2] >= 0 ) && ( rtc [2] <= 9 ) )                    //Display HOUR
    {
      myGLCD.setColor ( 0, 0, 0 );
      myGLCD.fillRect ( 80, 95, 111, 145 );
      myGLCD.setColor ( 0, 0, 255 );
      myGLCD.printNumI ( rtc [2], 112, 95 );
    }
    else {
      myGLCD.printNumI ( rtc [2], 80, 95 );
    }
  }

  if ( setTimeFormat == 1 )                                 //12HR Format
  {
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.setBackColor ( 0, 0, 0 );
    myGLCD.setFont ( SevenSegNumFont );
    if ( rtc [2] == 0 )                                     //Display HOUR
    {
      myGLCD.print ( "12", 80, 95 );
    }
    if ( ( rtc [2] >= 1 ) && ( rtc [2] <= 9 ) ) {
      myGLCD.setColor ( 0, 0, 0 );
      myGLCD.fillRect ( 80, 95, 111, 145 );
      myGLCD.setColor ( 0, 0, 255 );
      myGLCD.printNumI ( rtc [2], 112, 95 );
    }
    if ( ( rtc [2] >= 10 ) && ( rtc [2] <= 12 ) ) {
      myGLCD.printNumI ( rtc [2], 80, 95 );
    }
    if ( ( rtc [2] >= 13 ) && ( rtc [2] <= 21 ) ) {
      myGLCD.setColor ( 0, 0, 0 );
      myGLCD.fillRect ( 80, 95, 111, 145 );
      myGLCD.setColor ( 0, 0, 255 );
      myGLCD.printNumI ( rtc [2] - 12, 112, 95 );
    }
    if ( rtc [2] >= 22 ) {
      myGLCD.printNumI ( rtc [2] - 12, 80, 95 );
    }

    if ( ( rtc [2] >= 0 ) && ( rtc [2] <= 11 ) )                   //Display AM/PM
    {
      setFont ( LARGE, 0, 0, 255, 0, 0, 0 );
      myGLCD.print ( "AM", 244, 129 );
    }
    else {
      setFont ( LARGE, 0, 0, 255, 0, 0, 0 );
      myGLCD.print ( "PM", 244, 129 );
    }
  }

  myGLCD.setColor ( 0, 0, 255 );
  myGLCD.setBackColor ( 0, 0, 0 );
  myGLCD.setFont ( SevenSegNumFont );
  myGLCD.fillCircle ( 160, 108, 4 );
  myGLCD.fillCircle ( 160, 132, 4 );
  if ( ( rtc [1] >= 0 ) && ( rtc [1] <= 9 ) )                       //Display MINUTES
  {
    myGLCD.print ( "0", 176, 95 );
    myGLCD.printNumI ( rtc [1], 208, 95 );
  }
  else {
    myGLCD.printNumI ( rtc [1], 176, 95 );
  }
}

void screenSaver ()                               //Make the Screen Go Blank after so long
{
  if ( ( setScreensaver == 1 ) && ( tempAlarmflag == false ) ) {
    if ( myTouch.dataAvailable () ) {
      processMyTouch ();
    }
    else {
      screenSaverTimer++;
    }
    if ( screenSaverTimer == setScreenSaverTimer ) {
      dispScreen = 0;
      myGLCD.clrScr ();
    }
    if ( CLOCK_SCREENSAVER == true ) {
      if ( screenSaverTimer > setScreenSaverTimer ) {
        dispScreen = 0;
        TimeSaver ( true );
      }
    }
  }
}

void genSetSelect () {
  if ( setCalendarFormat == 0 )                      //Calendar Format Buttons
  {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 185, 19, 305, 39 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "DD/MM/YYYY", 207, 23 );
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.fillRoundRect ( 185, 45, 305, 65 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 255 );
    myGLCD.print ( "MTH DD, YYYY", 199, 49 );
  }
  else {
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.fillRoundRect ( 185, 19, 305, 39 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 255 );
    myGLCD.print ( "DD/MM/YYYY", 207, 23 );
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 185, 45, 305, 65 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "MTH DD, YYYY", 199, 49 );
  }
  if ( setTimeFormat == 0 )                          //Time Format Buttons
  {
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.fillRoundRect ( 195, 76, 235, 96 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 255 );
    myGLCD.print ( "12HR", 201, 80 );
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 255, 76, 295, 96 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "24HR", 261, 80 );
  }
  else {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 195, 76, 235, 96 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "12HR", 201, 80 );
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.fillRoundRect ( 255, 76, 295, 96 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 255 );
    myGLCD.print ( "24HR", 261, 80 );
  }
  if ( setTempScale == 0 )                           //Temperature Scale Buttons
  {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 195, 107, 235, 127 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "C", 215, 111 );
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.fillRoundRect ( 255, 107, 295, 127 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 255 );
    myGLCD.print ( "F", 275, 111 );
    myGLCD.setColor ( 0, 0, 0 );
    myGLCD.drawCircle ( 210, 113, 1 );
    myGLCD.setColor ( 255, 255, 255 );
    myGLCD.drawCircle ( 270, 113, 1 );
  }
  else {
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.fillRoundRect ( 195, 107, 235, 127 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 255 );
    myGLCD.print ( "C", 215, 111 );
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 255, 107, 295, 127 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "F", 275, 111 );
    myGLCD.setColor ( 255, 255, 255 );
    myGLCD.drawCircle ( 210, 113, 1 );
    myGLCD.setColor ( 0, 0, 0 );
    myGLCD.drawCircle ( 270, 113, 1 );
  }
  if ( setScreensaver == 1 )                         //Screensaver Buttons
  {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 195, 138, 235, 158 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "ON", 209, 142 );
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.fillRoundRect ( 255, 138, 295, 158 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 255 );
    myGLCD.print ( "OFF", 265, 142 );
  }
  else {
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.fillRoundRect ( 195, 138, 235, 158 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 255 );
    myGLCD.print ( "ON", 209, 142 );
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 255, 138, 295, 158 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "OFF", 265, 142 );
  }
  if ( setAutoStop == 1 )                            //Auto-Stop on Feed Buttons
  {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 195, 169, 235, 189 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "ON", 209, 173 );
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.fillRoundRect ( 255, 169, 295, 189 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 255 );
    myGLCD.print ( "OFF", 265, 173 );
  }
  else {
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.fillRoundRect ( 195, 169, 235, 189 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 255 );
    myGLCD.print ( "ON", 209, 173 );
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 255, 169, 295, 189 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "OFF", 265, 173 );
  }
  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.drawRoundRect ( 185, 19, 305, 39 );
  myGLCD.drawRoundRect ( 185, 45, 305, 65 );
  for ( int x = 0; x < 2; x++ ) {
    for ( int y = 0; y < 4; y++ ) {
      myGLCD.drawRoundRect ( ( x * 60 ) + 195, ( y * 31 ) + 76, ( x * 60 ) + 235, ( y * 31 ) + 96 );
    }
  }
}

void feedingTimeOnOff () {
  if ( ( feedTime == 1 ) && ( FEEDTime1 == 1 ) ) {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 70, 150, 250, 170 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "Feeding Time 1 ON", 94, 154 );
  }
  if ( ( feedTime == 1 ) && ( FEEDTime1 == 0 ) ) {
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRoundRect ( 70, 150, 250, 170 );
    setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
    myGLCD.print ( "Feeding Time 1 OFF", 90, 154 );
  }
  if ( ( feedTime == 2 ) && ( FEEDTime2 == 1 ) ) {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 70, 150, 250, 170 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "Feeding Time 2 ON", 94, 154 );
  }
  if ( ( feedTime == 2 ) && ( FEEDTime2 == 0 ) ) {
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRoundRect ( 70, 150, 250, 170 );
    setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
    myGLCD.print ( "Feeding Time 2 OFF", 90, 154 );
  }
  if ( ( feedTime == 3 ) && ( FEEDTime3 == 1 ) ) {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 70, 150, 250, 170 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "Feeding Time 3 ON", 94, 154 );
  }
  if ( ( feedTime == 3 ) && ( FEEDTime3 == 0 ) ) {
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRoundRect ( 70, 150, 250, 170 );
    setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
    myGLCD.print ( "Feeding Time 3 OFF", 90, 154 );
  }
  if ( ( feedTime == 4 ) && ( FEEDTime4 == 1 ) ) {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 70, 150, 250, 170 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "Feeding Time 4 ON", 94, 154 );
  }
  if ( ( feedTime == 4 ) && ( FEEDTime4 == 0 ) ) {
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRoundRect ( 70, 150, 250, 170 );
    setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
    myGLCD.print ( "Feeding Time 4 OFF", 90, 154 );
  }

  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.drawRoundRect ( 70, 150, 250, 170 );
}
/******************************* END OF MISC. FUNCTIONS *******************************/

/*********************** MAIN SCREEN ********** dispScreen = 0 ************************/
void mainScreen ( boolean refreshAll = false ) {
  int ledLevel, t;
  char oldval [16];

  TimeDateBar ( true );
  //	Serial.println ( "mainScreen1" );
  sprintf ( oldval, "%s", day );
  //  oldval = day;                                  //refresh day if different
  sprintf ( day, "%i", rtc [4] );
  //  day = rtc[4];
  if ( ( oldval != day ) || refreshAll ) {
    myGLCD.setColor ( 64, 64, 64 );                //Draw Borders & Dividers in Grey
    myGLCD.drawRect ( 0, 0, 319, 239 );            //Outside Border
    myGLCD.drawRect ( 158, 14, 160, 226 );         //Vertical Divider
    myGLCD.drawRect ( 160, 125, 319, 127 );        //Horizontal Divider
    myGLCD.fillRect ( 0, 0, 319, 14 );             //Top Bar
    myGLCD.setColor ( 0, 0, 0 );
    myGLCD.drawLine ( 159, 126, 161, 126 );        //Horizontal Divider Separator
    setFont ( SMALL, 255, 255, 0, 64, 64, 64 );
    myGLCD.print ( "Jarduino Aquarium Controller v.1.1", CENTER, 1 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
    myGLCD.print ( "LED ARRAY", 52, 20 );
    myGLCD.print ( "LUNAR PHASE", 196, 20 );
    myGLCD.print ( "MONITORS & ALERTS", 174, 133 );

    myGLCD.drawBitmap ( 211, 35, 58, 58, MoonPic, 1 );    //Moon Phase Picture (middle 240,64)
    setFont ( SMALL, 176, 176, 176, 0, 0, 0 );
    //    char bufferLP[16];
    //    LP.toCharArray(bufferLP, 16);
    myGLCD.print ( LP, 180, 96 );            //Print Moon Phase Description to LCD
    float lunarCycle = moonPhase ( rtc [6], rtc [5], rtc [4] );  //get a value for the lunar cycle
    if ( ( lunarCycle * 100 ) < 1 )                   //Print % of Full to LCD
    {
      myGLCD.print ( " 0.0", 188, 108 );
    }
    else {
      myGLCD.printNumF ( lunarCycle * 100, 1, 188, 108 );
    }
    myGLCD.print ( "% of Full", 220, 108 );
  }

  drawBarGraph ();
  //	Serial.println ( "mainScreen 2-b" );
  //  oldval.reserve(6*22);
  if ( ( sumpLed != sled_out ) || refreshAll )         //refresh red led display
  {
    //Serial.println("- 1 -");
    sumpLed = sled_out;
    // Serial.println("- 2 -");
    ledLevel = LedToPercent ( sled_out );
    // Serial.println(ledLevel);
    // Serial.println("- 1 -");
    sprintf ( oldval, "SUMP:   %i%%    ", ledLevel );
    //    oldval = "SUMP:   " + String(ledLevel) + "%    ";
    //oldval = "SUMP:   xx%   s ";
    //  Serial.println("- 3 -");
    //    char bufferS[13];
    //Serial.println("- 1 -");
    //    oldval.toCharArray(bufferS, 13);
    // Serial.println("- 4 -");
    t = 136 - sled_out * .39;
    // Serial.println("- 1 -");
    myGLCD.setColor ( 0, 0, 0 );
    // Serial.println("- 5 -");
    myGLCD.fillRect ( 40, t, 52, 36 );              //hide end of last bar
    // Serial.println("- 1 -");
    myGLCD.setColor ( 0, 150, 0 );
    // Serial.println("- 6 -");
    setFont ( SMALL, 0, 150, 0, 0, 0, 0 );
    // Serial.println("- 1 -");
    myGLCD.print ( oldval, 45, 147 );              //display SUMP LEDs % output
    //Serial.println("- 7 -");
    myGLCD.drawRect ( 40, 136, 52, 135 );           //SUMP %bar place holder
    // Serial.println("- 1 -");
    myGLCD.fillRect ( 40, 136, 52, t );             //SUMP percentage bar
  }
  //	Serial.println ( "mainScreen 2-c" );
  if ( ( redLed != rled_out ) || refreshAll )          //refresh red led display
  {
    redLed = rled_out;
    ledLevel = LedToPercent ( rled_out );
    //		Serial.println ( "mump 2" );
    sprintf ( oldval, "Red:    %i%%    ", ledLevel );
    //    oldval = "Red:    " + String(ledLevel) + "%  " + "  ";
    //    char bufferR[13];
    //		Serial.println ( "mump 2" );
    //    oldval.toCharArray(oldval, 13);
    t = 136 - rled_out * .39;

    myGLCD.setColor ( 0, 0, 0 );
    myGLCD.fillRect ( 57, t, 69, 36 );              //hide end of last bar
    myGLCD.setColor ( 255, 0, 0 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( oldval, 45, 159 );              //display red LEDs % output
    myGLCD.drawRect ( 57, 136, 69, 135 );           //red %bar place holder
    myGLCD.fillRect ( 57, 136, 69, t );             //red percentage bar
  }
  //	Serial.println ( "mainScreen 22" );
  if ( ( whiteLed != wled_out ) || refreshAll )        //refresh white led display
  {
    whiteLed = wled_out;
    ledLevel = LedToPercent ( wled_out );
    sprintf ( oldval, "White:  %i%%    ", ledLevel );
    //    oldval = "White:  " + String(ledLevel) + "%  " + "  ";
    //    char bufferW[13];
    //    oldval.toCharArray(bufferW, 13);
    t = 136 - wled_out * .39;

    myGLCD.setColor ( 0, 0, 0 );
    myGLCD.fillRect ( 74, t, 86, 36 );              //hide end of last bar
    myGLCD.setColor ( 255, 255, 255 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
    myGLCD.print ( oldval, 45, 171 );              //display white LEDs % output
    myGLCD.drawRect ( 74, 136, 86, 135 );           //white %bar place holder
    myGLCD.fillRect ( 74, 136, 86, t );             //white percentage bar
  }
  //	Serial.println ( "mainScreen 2-d" );
  if ( ( blueLed != bled_out ) || refreshAll )         //refresh blue led displays
  {
    blueLed = bled_out;
    ledLevel = LedToPercent ( bled_out );
    sprintf ( oldval, "Blue:   %i%%    ", ledLevel );
    //    oldval = "Blue:   " + String(ledLevel) + "%" + "  ";
    //    char bufferB[13];
    //    oldval.toCharArray(bufferB, 13);
    t = 136 - bled_out * .39;

    myGLCD.setColor ( 0, 0, 0 );
    myGLCD.fillRect ( 91, t, 103, 36 );             //hide end of last bar
    myGLCD.setColor ( 9, 184, 255 );
    setFont ( SMALL, 99, 184, 255, 0, 0, 0 );
    myGLCD.print ( oldval, 45, 183 );              //display blue LEDs % output
    myGLCD.drawRect ( 91, 136, 103, 135 );          //blue %bar place holder
    myGLCD.fillRect ( 91, 136, 103, t );            //blue percentage bar
  }

  if ( ( rblueLed != rbled_out ) || refreshAll )       //refresh royal blue led display
  {
    rblueLed = rbled_out;
    ledLevel = LedToPercent ( rbled_out );
    sprintf ( oldval, "Royal:  %i%%    ", ledLevel );
    //    oldval = "Royal:  " + String(ledLevel) + "%  " + "  ";
    //    char bufferRB[13];
    //    oldval.toCharArray(bufferRB, 13);
    t = 136 - rbled_out * .39;

    myGLCD.setColor ( 0, 0, 0 );
    myGLCD.fillRect ( 108, t, 120, 36 );            //hide end of last bar
    myGLCD.setColor ( 58, 95, 205 );
    setFont ( SMALL, 58, 95, 205, 0, 0, 0 );
    myGLCD.print ( oldval, 45, 195 );             //display royal blue LEDs % output
    myGLCD.drawRect ( 108, 136, 120, 135 );         //royal %bar place holder
    myGLCD.fillRect ( 108, 136, 120, t );           //royal percentage bar
  }

  if ( ( uvLed != uvled_out ) || refreshAll )          //refresh UV led display
  {
    uvLed = uvled_out;
    ledLevel = LedToPercent ( uvled_out );
    sprintf ( oldval, "Ultra:  %i%%    ", ledLevel );
    //    oldval = "Ultra:  " + String(ledLevel) + "%  " + "  ";
    //    char bufferUV[13];
    //    oldval.toCharArray(bufferUV, 13);
    t = 136 - uvled_out * .39;

    myGLCD.setColor ( 0, 0, 0 );
    myGLCD.fillRect ( 125, t, 137, 36 );            //hide end of last bar
    myGLCD.setColor ( 224, 102, 255 );
    setFont ( SMALL, 224, 102, 255, 0, 0, 0 );
    myGLCD.print ( oldval, 45, 207 );             //display UV LEDs % output
    myGLCD.drawRect ( 125, 136, 137, 135 );         //UV %bar place holder
    myGLCD.fillRect ( 125, 136, 137, t );           //UV percentage bar
  }
  //	Serial.println ( "mainScreen 3" );
  if ( setTempScale == 1 ) {
    sprintf ( degC_F, "F" );
  }               //Print deg C or deg F
  else {
    sprintf ( degC_F, "C" );
  }

  //char bufferDeg[2];
  // degC_F.toCharArray(bufferDeg,2);

  if ( refreshAll )                                //draw static elements
  {
    setFont ( SMALL, 0, 255, 0, 0, 0, 0 );
    myGLCD.print ( "Water Temp:", 169, 148 );
    myGLCD.drawCircle ( 304, 150, 1 );
    myGLCD.print ( degC_F, 309, 148 );

    myGLCD.print ( "Hood Temp:", 169, 161 );
    myGLCD.drawCircle ( 304, 163, 1 );
    myGLCD.print ( degC_F, 309, 161 );

    myGLCD.print ( "Sump Temp:", 169, 174 );
    myGLCD.drawCircle ( 304, 176, 1 );
    myGLCD.print ( degC_F, 309, 174 );
  }
  //	Serial.println ( "mainScreen 5" );
  myGLCD.setColor ( 0, 0, 0 );                      //clear cooler/heater & alarm notices
  myGLCD.fillRect ( 200, 189, 303, 203 );
  myGLCD.fillRect ( 182, 203, 315, 221 );
  //	Serial.println ( "mainScreen 4" );
  if ( ( tempW > 50 ) || ( tempW < 10 ) )                  //range in deg C no matter what
  {
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "Error", 260, 148 );
  }
  else {
    if ( setTempScale == 1 ) {
      tempW = ( ( tempW * 1.8 ) + 32.05 );
    }         //C to F with rounding
    if ( tempCoolflag == true )                     //Water temperature too HIGH
    {
      setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
      myGLCD.printNumF ( tempW, 1, 260, 148 );
      setFont ( SMALL, 255, 255, 0, 0, 0, 0 );
      myGLCD.print ( "Chiller ON", 200, 191 );
    }
    else if ( tempHeatflag == true )                      //Water temperature too LOW
    {
      setFont ( SMALL, 0, 0, 255, 0, 0, 0 );
      myGLCD.printNumF ( tempW, 1, 260, 148 );
      setFont ( SMALL, 255, 255, 0, 0, 0, 0 );
      myGLCD.print ( "Heater ON", 203, 191 );
    }
    else {
      setFont ( SMALL, 0, 255, 0, 0, 0, 0 );
      myGLCD.printNumF ( tempW, 1, 260, 148 );
    }
    if ( ( tempW < 100 ) && ( tempW >= 0 ) ) {
      myGLCD.setColor ( 0, 0, 0 );
      myGLCD.fillRect ( 292, 148, 300, 160 );
    }
  }

  if ( ( tempH > 50 ) || ( tempH < 10 ) ) {
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "Error", 260, 161 );
  }
  else {
    if ( setTempScale == 1 ) {
      tempH = ( ( tempH * 1.8 ) + 32.05 );
    }
    setFont ( SMALL, 0, 255, 0, 0, 0, 0 );
    myGLCD.printNumF ( tempH, 1, 260, 161 );            //Hood temperature (No Flags)
    if ( ( tempH < 100 ) && ( tempH >= 0 ) ) {
      myGLCD.setColor ( 0, 0, 0 );
      myGLCD.fillRect ( 292, 161, 300, 173 );
    }
  }
  //	Serial.println ( "mainScreen 6" );
  if ( ( tempS > 50 ) || ( tempS < 10 ) ) {
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "Error", 260, 174 );
  }
  else {
    if ( setTempScale == 1 ) {
      tempS = ( ( tempS * 1.8 ) + 32.05 );
    }
    setFont ( SMALL, 0, 255, 0, 0, 0, 0 );
    myGLCD.printNumF ( tempS, 1, 260, 174 );            //Sump temperature (No Flags)
    if ( ( tempS < 100 ) && ( tempS >= 0 ) ) {
      myGLCD.setColor ( 0, 0, 0 );
      myGLCD.fillRect ( 292, 174, 300, 186 );
    }
  }

  if ( ( tempAlarmflag == true ) && ( tempHeatflag == true ) )     //Alarm: H20 temp Below offsets
  {
    setFont ( LARGE, 0, 0, 255, 0, 0, 0 );
    myGLCD.print ( "ALARM!!", 185, 204 );
  }
  if ( ( tempAlarmflag == true ) && ( tempCoolflag == true ) )     //Alarm: H20 temp Above offsets
  {
    setFont ( LARGE, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "ALARM!!", 185, 204 );
  }
}

void screenReturn ()                                    //Auto Return to MainScreen()
{
  if ( SCREEN_RETURN == true ) {
    if ( dispScreen != 0 ) {
      if ( myTouch.dataAvailable () ) {
        processMyTouch ();
      }
      else {
        returnTimer++;
      }
      if ( returnTimer > setReturnTimer ) {
        returnTimer = 0;
        LEDtestTick = false;
        colorLEDtest = false;
        ReadFromEEPROM ();
        dispScreen = 0;
        clearScreen ();
        mainScreen ( true );
      }
    }
  }
}
/******************************** END OF MAIN SCREEN **********************************/

/*********************** MENU SCREEN ********** dispScreen = 1 ************************/
void menuScreen () {
  printHeader ( "Choose Option 1/2" );

  myGLCD.setColor ( 64, 64, 64 );
  myGLCD.drawRect ( 0, 196, 319, 194 );
  printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );
  printButton ( "NEXT", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );

  printButton ( "Time and Date", tanD [0], tanD [1], tanD [2], tanD [3] );
  printButton ( "H2O Temp Control", temC [0], temC [1], temC [2], temC [3] );
  printButton ( "WaveMaker", wave [0], wave [1], wave [2], wave [3] );
  printButton ( "General Settings", gSet [0], gSet [1], gSet [2], gSet [3] );
  printButton ( "LED Testing", tesT [0], tesT [1], tesT [2], tesT [3] );
  printButton ( "Change LED Values", ledChM [0], ledChM [1], ledChM [2], ledChM [3] );
  printButton ( "Automatic Feeder", aFeed [0], aFeed [1], aFeed [2], aFeed [3] );
  printButton ( "About", about [0], about [1], about [2], about [3] );

}
/********************************* END OF MENU1 SCREEN *********************************/
/*********************** MENU SCREEN ********** dispScreen = 15 ************************/
void menuScreen2 () {
  printHeader ( "Choose Option 2/2" );

  myGLCD.setColor ( 64, 64, 64 );
  myGLCD.drawRect ( 0, 196, 319, 194 );
  printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );
  printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );

  printButton ( "Dosing Pump", dosingPumpButton [0], dosingPumpButton [1], dosingPumpButton [2], dosingPumpButton [3] );

}
/********************************* END OF MENU1 SCREEN *********************************/

/************** TIME and DATE SCREEN ********** dispScreen = 2 ************************/
void clockScreen ( boolean refreshAll = true ) {
  if ( refreshAll ) {
    for ( int i = 0; i < 7; i++ ) {
      rtcSet [i] = rtc [i];
    }

    printHeader ( "Time and Date Settings" );

    myGLCD.setColor ( 64, 64, 64 );                   //Draw Dividers in Grey
    myGLCD.drawRect ( 0, 196, 319, 194 );             //Bottom Horizontal Divider
    myGLCD.drawLine ( 0, 104, 319, 104 );             //Middle Horizontal Divider
    printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
    printButton ( "SAVE", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );
    printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

    printButton ( "+", houU [0], houU [1], houU [2], houU [3], true );     //hour up
    printButton ( "+", minU [0], minU [1], minU [2], minU [3], true );     //min up
    printButton ( "-", houD [0], houD [1], houD [2], houD [3], true );     //hour down
    printButton ( "-", minD [0], minD [1], minD [2], minD [3], true );     //min down
    if ( setTimeFormat == 1 ) {
      printButton ( "+", ampmU [0], ampmU [1], ampmU [2], ampmU [3], true );  //AM/PM up
      printButton ( "-", ampmD [0], ampmD [1], ampmD [2], ampmD [3], true );
    }  //AM/PM down

    printButton ( "+", monU [0], monU [1], monU [2], monU [3], true );     //month up
    printButton ( "+", dayU [0], dayU [1], dayU [2], dayU [3], true );     //day up
    printButton ( "+", yeaU [0], yeaU [1], yeaU [2], yeaU [3], true );     //year up
    printButton ( "-", monD [0], monD [1], monD [2], monD [3], true );     //month down
    printButton ( "-", dayD [0], dayD [1], dayD [2], dayD [3], true );     //day down
    printButton ( "-", yeaD [0], yeaD [1], yeaD [2], yeaD [3], true );     //year down
  }

  ReadFromEEPROM ();
  timeDispH = rtcSet [2];
  timeDispM = rtcSet [1];
  xTimeH = 107;
  yTime = 52;
  xColon = xTimeH + 42;
  xTimeM10 = xTimeH + 70;
  xTimeM1 = xTimeH + 86;
  xTimeAMPM = xTimeH + 155;
  timeChange ();

  setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
  myGLCD.print ( "Date", 20, 142 );
  myGLCD.print ( "/", 149, 142 );
  myGLCD.print ( "/", 219, 142 );
  if ( setCalendarFormat == 0 )                             //DD/MM/YYYY Format
  {
    setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
    myGLCD.print ( "(DD/MM/YYYY)", 5, 160 );
    setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
    if ( ( rtcSet [4] >= 0 ) && ( rtcSet [4] <= 9 ) )              //Set DAY
    {
      myGLCD.print ( "0", 107, 142 );
      myGLCD.printNumI ( rtcSet [4], 123, 142 );
    }
    else {
      myGLCD.printNumI ( rtcSet [4], 107, 142 );
    }
    if ( ( rtcSet [5] >= 0 ) && ( rtcSet [5] <= 9 ) )              //Set MONTH
    {
      myGLCD.print ( "0", 177, 142 );
      myGLCD.printNumI ( rtcSet [5], 193, 142 );
    }
    else {
      myGLCD.printNumI ( rtcSet [5], 177, 142 );
    }
  }
  else if ( setCalendarFormat == 1 )                             //MM/DD/YYYY Format
  {
    setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
    myGLCD.print ( "(MM/DD/YYYY)", 5, 160 );
    setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
    if ( ( rtcSet [5] >= 0 ) && ( rtcSet [5] <= 9 ) )              //Set MONTH
    {
      myGLCD.print ( "0", 107, 142 );
      myGLCD.printNumI ( rtcSet [5], 123, 142 );
    }
    else {
      myGLCD.printNumI ( rtcSet [5], 107, 142 );
    }
    if ( ( rtcSet [4] >= 0 ) && ( rtcSet [4] <= 9 ) )              //Set DAY
    {
      myGLCD.print ( "0", 177, 142 );
      myGLCD.printNumI ( rtcSet [4], 193, 142 );
    }
    else {
      myGLCD.printNumI ( rtcSet [4], 177, 142 );
    }
  }
  myGLCD.printNumI ( rtcSet [6], 247, 142 );                //Set YEAR
}

void timeChange () {
  setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
  myGLCD.print ( "Time", 20, yTime );

  if ( setTimeFormat == 0 )                                 //24HR Format
  {
    setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
    myGLCD.print ( "(24HR)", 29, yTime + 18 );
  }

  if ( setTimeFormat == 1 )                                 //12HR Format
  {
    setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
    myGLCD.print ( "(12HR)", 29, yTime + 18 );
  }

  timeCorrectFormat ();
}

void buildCorrectTime () {
  char minute [3], stunde [3], ampm [4];
  //	char oldVal [16], rtc1 [3], rtc2 [3], ampm [4], month [5];

  //		if ( ( rtc [1] >= 0 ) && ( rtc [1] <= 9 ) ) {
  //	sprintf ( minute, "%i%i", 0, rtc [1] );
  //		}               //adds 0 to minutes
  //		else {
  //			sprintf ( rtc1, "%i", rtc [1] );
  //		}
  if ( ( timeDispM >= 0 ) && ( timeDispM <= 9 ) ) {
    sprintf ( minute, "%i%i", 0, timeDispM );
    //			sprintf ( rtc1, "%i%i", 0, rtc [1] );
  }               //adds 0 to minutes
  else {
    sprintf ( minute, "%i", timeDispM );
  }
  //
  //		if ( setTimeFormat == 1 ) {
  //			if ( rtc [2] == 0 ) {
  //				sprintf ( rtc2, "%i", 12 );
  //			}                //12 HR Format
  //			else {
  //				if ( rtc [2] > 12 ) {
  //					sprintf ( rtc2, "%i", rtc [2] - 12 );
  //				}
  //				else {
  //					sprintf ( rtc2, "%i", rtc [2] );
  //				}
  //			}
  //		}
  //
  //		if ( rtc [2] < 12 ) {
  //			sprintf ( ampm, " AM  " );
  //		}              //Adding the AM/PM sufffix
  //		else {
  //			sprintf ( ampm, " PM  " );
  //		}
  //
  //		sprintf ( oldVal, "%i", time );                                 //refresh time if different
  //		if ( setTimeFormat == 1 ) {
  //			sprintf ( time, "%s:%s%s", rtc2, rtc1, ampm );
  //		}
  //		else {
  //			sprintf ( time, " %i:%s      ", rtc [2], rtc1 );
  //		}

  //	timeDispM
  if ( setTimeFormat == 1 ) {
    if ( timeDispH == 0 ) {
      sprintf ( stunde, "%i", 12 );
    }                //12 HR Format
    else {
      if ( timeDispH > 12 ) {
        sprintf ( stunde, "%i", timeDispH - 12 );
      }
      else {
        sprintf ( stunde, "%i", timeDispH );
      }
    }
  }

  if ( timeDispH < 12 ) {
    sprintf ( ampm, " AM" );
  }              //Adding the AM/PM sufffix
  else {
    sprintf ( ampm, " PM" );
  }

  if ( setTimeFormat == 1 ) {
    sprintf ( time, "%s:%s%s", stunde, minute, ampm );
  }
  else {
    sprintf ( time, "%i:%s", timeDispH, minute );
  }
}

void timeCorrectFormat () {
  //	if ( bigClock ) {
  setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
  //	}
  //	else {

  //		setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
  //
  //	}
  buildCorrectTime ();

  myGLCD.print ( time, xTimeH, yTime );            //Display time

    //	myGLCD.print ( ":", xColon, yTime );
  //	if ( setTimeFormat == 0 )                                 //24HR Format
  //			{
  ////		setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
  //		if ( ( timeDispH >= 0 ) && ( timeDispH <= 9 ) )              //Set HOUR
  //				{
  //			myGLCD.print ( "0", xTimeH, yTime );
  //			myGLCD.printNumI ( timeDispH, xTimeH + 16, yTime );
  //		}
  //		else {
  //			myGLCD.printNumI ( timeDispH, xTimeH, yTime );
  //		}
  //	}
  //	if ( setTimeFormat == 1 )                                 //12HR Format
  //			{
  ////		setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
  //		if ( timeDispH == 0 )                                  //Set HOUR
  //				{
  //			myGLCD.print ( "12", xTimeH, yTime );
  //		}
  //		if ( ( timeDispH >= 1 ) && ( timeDispH <= 9 ) ) {
  //			myGLCD.print ( "0", xTimeH, yTime );
  //			myGLCD.printNumI ( timeDispH, xTimeH + 16, yTime );
  //		}
  //		if ( ( timeDispH >= 10 ) && ( timeDispH <= 12 ) ) {
  //			myGLCD.printNumI ( timeDispH, xTimeH, yTime );
  //		}
  //		if ( ( timeDispH >= 13 ) && ( timeDispH <= 21 ) ) {
  //			myGLCD.print ( "0", xTimeH, yTime );
  //			myGLCD.printNumI ( timeDispH - 12, xTimeH + 16, yTime );
  //		}
  //		if ( timeDispH >= 22 ) {
  //			myGLCD.printNumI ( timeDispH - 12, xTimeH, yTime );
  //		}
  //
  //		if ( AM_PM == 1 ) {
  //			myGLCD.print ( "AM", xTimeAMPM, yTime );
  //		}
  //		if ( AM_PM == 2 ) {
  //			myGLCD.print ( "PM", xTimeAMPM, yTime );
  //		}
  //	}
  //	if ( ( timeDispM >= 0 ) && ( timeDispM <= 9 ) )                 //Set MINUTES
  //			{
  //		myGLCD.print ( "0", xTimeM10, yTime );
  //		myGLCD.printNumI ( timeDispM, xTimeM1, yTime );
  //	}
  //	else {
  //		myGLCD.printNumI ( timeDispM, xTimeM10, yTime );
  //	}
}
/**************************** END OF TIME and DATE SCREEN *****************************/

/*********** H2O TEMP CONTROL SCREEN ********** dispScreen = 3 ************************/
void tempScreen ( boolean refreshAll = false ) {
  //  char deg[2];
  if ( refreshAll ) {
    if ( ( setTempC == 0 ) && ( setTempScale == 0 ) ) {
      setTempC = 26.1;
    }                         //change to 26.1 deg C
    if ( ( ( setTempF == 0 ) || ( setTempF == setTempC ) ) && ( setTempScale == 1 ) ) {
      setTempF = 79.0;
    }                         //change to 79.0 deg F

    if ( setTempScale == 1 ) {
      temp2beS = setTempF;
      temp2beO = offTempF;
      temp2beA = alarmTempF;
    }
    else {
      temp2beS = setTempC;
      temp2beO = offTempC;
      temp2beA = alarmTempC;
    }

    printHeader ( "H2O Temperature Control Settings" );

    myGLCD.setColor ( 64, 64, 64 );                    //Draw Dividers in Grey
    myGLCD.drawRect ( 0, 196, 319, 194 );              //Bottom Horizontal Divider
    printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
    printButton ( "SAVE", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );
    printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

    if ( setTempScale == 1 ) {
      //      deg ="F";
      sprintf ( degC_F, "F" );
    }                //Print deg C or deg F
    else {
      //      deg = "C";
      sprintf ( degC_F, "C" );
    }
    sprintf ( degC_F, "C" );
    //    degC_F=deg;
    // char bufferDeg[2];
    //    degC_F.toCharArray(bufferDeg,2);

    setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
    myGLCD.print ( "Desired Temperature in", 60, 34 );
    myGLCD.drawCircle ( 245, 36, 1 );
    myGLCD.print ( degC_F, 250, 34 );
    myGLCD.print ( ":", 258, 34 );
    myGLCD.print ( "Temperature Offset:", CENTER, 84 );
    myGLCD.print ( "Alarm Offset:", CENTER, 134 );

    printButton ( "-", temM [0], temM [1], temM [2], temM [3], true );      //temp minus
    printButton ( "+", temP [0], temP [1], temP [2], temP [3], true );      //temp plus
    printButton ( "-", offM [0], offM [1], offM [2], offM [3], true );      //offset minus
    printButton ( "+", offP [0], offP [1], offP [2], offP [3], true );      //offset plus
    printButton ( "-", almM [0], almM [1], almM [2], almM [3], true );      //alarm minus
    printButton ( "+", almP [0], almP [1], almP [2], almP [3], true );      //alarm plus
  }

  setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
  myGLCD.printNumF ( temp2beS, 1, CENTER, 54 );
  myGLCD.printNumF ( temp2beO, 1, CENTER, 104 );
  myGLCD.printNumF ( temp2beA, 1, CENTER, 154 );
}
/************************** END of H20 TEMP CONTROL SCREEN ****************************/

/********** LED TESTING OPTIONS SCREEN ******** dispScreen = 4 ************************/
void ledTestOptionsScreen () {
  printHeader ( "LED Testing Options" );

  myGLCD.setColor ( 64, 64, 64 );                       //Draw Dividers in Grey
  myGLCD.drawRect ( 0, 196, 319, 194 );                 //Bottom Horizontal Divider
  printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
  printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

  printButton ( "Test LED Array Output", tstLA [0], tstLA [1], tstLA [2], tstLA [3] );
  printButton ( "Control Individual Leds", cntIL [0], cntIL [1], cntIL [2], cntIL [3] );
}
/*************************** END OF LED TEST OPTIONS SCREEN ***************************/

/********** TEST LED ARRAY SCREEN ************* dispScreen = 5 ************************/
void testArrayScreen ( boolean refreshAll = false ) {
  if ( refreshAll ) {
    printHeader ( "Test LED Array Output Settings" );
    myGLCD.fillRect ( 1, 15, 318, 37 );       //clear "Test in Progress" Banner

    myGLCD.setColor ( 64, 64, 64 );            //Draw Dividers in Grey
    myGLCD.drawRect ( 0, 196, 319, 194 );      //Bottom Horizontal Divider
    printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
    printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

    printButton ( "", stsT [0], stsT [1], stsT [2], stsT [3], true );      //start/stop
    printButton ( "-10s", tenM [0], tenM [1], tenM [2], tenM [3], true );  //-10s
    printButton ( "+10s", tenP [0], tenP [1], tenP [2], tenP [3], true );  //+10s
    myGLCD.print ( "START", stsT [0] + 6, stsT [1] + 15 );
    myGLCD.print ( "TEST", stsT [0] + 15, stsT [1] + 40 );
  }
  else {
    min_cnt = 0;

    myGLCD.setColor ( 0, 0, 0 );
    myGLCD.fillRect ( 1, 15, 318, 99 );       //clear test results if any
    myGLCD.fillRect ( 1, 187, 318, 227 );     //clear the "Back" and "Cancel" Buttons

    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.fillRect ( stsT [0] + 5, stsT [1] + 5, stsT [2] - 5, stsT [3] - 40 );    //clear 'start'
    setFont ( LARGE, 255, 255, 255, 0, 0, 255 );
    myGLCD.print ( "STOP", stsT [0] + 15, stsT [1] + 15 );

    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRect ( 1, 15, 318, 37 );
    myGLCD.drawRoundRect ( stsT [0], stsT [1], stsT [2], stsT [3] );  //red button during test
    setFont ( LARGE, 255, 255, 255, 255, 0, 0 );
    myGLCD.print ( "Test in Progress", CENTER, 16 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
    myGLCD.print ( "TIME:", 52, 40 );
    myGLCD.print ( "LIGHT OUTPUT (0--255):", 140, 40 );

    while ( LEDtestTick )               //test LED and speed up time
    {
      unsigned long currentMillis = millis ();

      if ( myTouch.dataAvailable () ) {
        processMyTouch ();
      }

      if ( currentMillis - previousMillisLED > 500 )    //change time every 0.5s
      {
        previousMillisLED = currentMillis;

        min_cnt++;
        char oldvalue [9], twelveHR [9], hrs [12], hrsPM [3], Minutes [3], AMPM [2];
        int hours = min_cnt / 60;
        int minut = min_cnt % 60;

        if ( hours < 12 ) {
          sprintf ( AMPM, "AM" );
          //          AMPM="AM";
        }                              //Adding the AM/PM suffix
        else {
          sprintf ( AMPM, "PM" );
          //          AMPM="PM";
        }
        //        snprintf (HOURS,3,"%i", hours);
        //        HOURS=String( hours);
        sprintf ( hrsPM, "%i", hours - 12 );
        //        hrsPM=String( hours-12);
        if ( hours == 0 ) {

          sprintf ( hrs, "12" );
        }
        else {
          if ( ( hours >= 1 ) && ( hours <= 9 ) ) {
            sprintf ( hrs, " %i", hours );
            //            hrs=" "+HOURS;
          }      //keep hours in place
          else {
            if ( ( hours >= 13 ) && ( hours <= 21 ) ) {
              sprintf ( hrs, " %i", hrsPM );
              //              hrs=" " +hrsPM;
            }  //convert to 12HR
            else {
              if ( ( hours >= 22 ) && ( hours < 24 ) ) {
                //            	  snprintf (hrs,3,"%i", hrsPM);
                sprintf ( hrs, "%s", hrsPM );
              }
              else {
                if ( hours == 24 ) {
                  sprintf ( hrs, "12" );
                }
                else {
                  sprintf ( hrs, "%i", hours );
                  //                  hrs=HOURS;
                }
              }
            }
          }
        }
        //        snprintf (mins,3,"%i", minut);
        //        mins=String( minut);                                            //add zero to minutes
        if ( ( minut >= 0 ) && ( minut <= 9 ) ) {
          sprintf ( Minutes, "0%i", minut );
          //          Minutes="0"+mins;
        }
        else {
          sprintf ( Minutes, "%i", minut );
          //          Minutes=mins;
        }
        sprintf ( oldvalue, "%s", twelveHR );
        //        oldvalue=twelveHR;
        sprintf ( twelveHR, "%s:%s", hrs, Minutes );
        //        twelveHR=hrs+':'+Minutes;
        if ( ( oldvalue != twelveHR ) || refreshAll ) {
          //          char bufferCount[9];
          //          twelveHR.toCharArray(bufferCount,9);
          setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
          myGLCD.print ( twelveHR, 7, 55 );
          //          char bufferAMPM[3];
          //          AMPM.toCharArray(bufferAMPM,3);
          myGLCD.print ( AMPM, 90, 55 );
        }

        char sled [13], rled [13], wled [13], bled [13], rbled [13], uvled [13];
        setFont ( SMALL, 0, 150, 0, 0, 0, 0 );
        sprintf ( sled, "SUMP:  %i   ", sled_out );
        //        String sled = "SUMP:  " + String(sled_out) + " " + " ";
        //        char bufferS[11];
        //        sled.toCharArray(bufferS, 11);
        myGLCD.print ( sled, 145, 55 );

        setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
        sprintf ( rled, "Red:   %i   ", rled_out );
        //        String rled = "Red:   " + String(rled_out) + "  " + " ";
        //        char bufferR[11];
        //        rled.toCharArray(bufferR, 11);
        myGLCD.print ( rled, 145, 67 );

        setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
        sprintf ( wled, "White: %i   ", wled_out );
        //        String wled = "White: " + String(wled_out) + " " + " ";
        //        char bufferW[11];
        //        wled.toCharArray(bufferW, 11);
        myGLCD.print ( wled, 145, 79 );

        setFont ( SMALL, 9, 184, 255, 0, 0, 0 );
        sprintf ( bled, "Blue:  %i   ", bled_out );
        //        String bled = "Blue:  " + String(bled_out) + " " + " ";
        //        char bufferB[11];
        //        bled.toCharArray(bufferB, 11);
        myGLCD.print ( bled, 235, 55 );

        setFont ( SMALL, 58, 95, 205, 0, 0, 0 );
        sprintf ( rbled, "Royal: %i   ", rbled_out );
        //        String rbled = "Royal: " + String(rbled_out) + "  " + " ";
        //        char bufferRB[11];
        //        rbled.toCharArray(bufferRB, 11);
        myGLCD.print ( rbled, 235, 67 );

        setFont ( SMALL, 224, 102, 255, 0, 0, 0 );
        sprintf ( uvled, "Ultra: %i   ", uvled_out );
        //        String uvled = "Ultra: " + String(uvled_out) + "  " + " ";
        //        char bufferUV[11];
        //        uvled.toCharArray(bufferUV, 11);
        myGLCD.print ( uvled, 235, 79 );

        LED_levels_output ();
        checkTempC ();
        TimeDateBar ();
      }
    }
  }
}
/*************************** END OF TEST LED ARRAY SCREEN *****************************/

/********* TEST INDIVIDUAL LED(S) SCREEN ****** dispScreen = 6 ************************/
void testIndLedScreen () {
  printHeader ( "Test Individual LED Values (0--255)" );

  setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
  myGLCD.print ( "LED OUTPUT %", 52, 20 );

  drawBarGraph ();
  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.drawRect ( 30, 137, 165, 138 );               //x-plane extended to include moon%
  myGLCD.setColor ( 176, 176, 176 );
  myGLCD.drawRect ( 142, 136, 154, 135 );              //MOON %bar place holder

  myGLCD.setColor ( 0, 150, 0 );
  myGLCD.fillRoundRect ( 5, 148, 105, 168 );
  setFont ( SMALL, 255, 255, 255, 0, 150, 0 );
  myGLCD.print ( "Sump:", 15, 152 );                   //display SUMP LEDs output
  myGLCD.print ( "0", 67, 152 );

  myGLCD.setColor ( 255, 0, 0 );
  myGLCD.fillRoundRect ( 5, 174, 105, 194 );
  setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
  myGLCD.print ( "Red:", 15, 178 );                    //display Red LEDs output
  myGLCD.print ( "0", 67, 178 );

  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.fillRoundRect ( 5, 200, 105, 220 );
  setFont ( SMALL, 0, 0, 0, 255, 255, 255 );
  myGLCD.print ( "White:", 15, 204 );                  //display White LEDs output
  myGLCD.print ( "0", 67, 204 );

  myGLCD.setColor ( 9, 184, 255 );
  myGLCD.fillRoundRect ( 110, 148, 210, 168 );
  setFont ( SMALL, 255, 255, 255, 9, 184, 255 );
  myGLCD.print ( "Blue:", 120, 152 );                  //display Blue LEDs output
  myGLCD.print ( "0", 172, 152 );

  myGLCD.setColor ( 58, 95, 205 );
  myGLCD.fillRoundRect ( 110, 174, 210, 194 );
  setFont ( SMALL, 255, 255, 255, 58, 95, 205 );
  myGLCD.print ( "Royal:", 120, 178 );                 //display Royal Blue LEDs output
  myGLCD.print ( "0", 172, 178 );

  myGLCD.setColor ( 224, 102, 255 );
  myGLCD.fillRoundRect ( 110, 200, 210, 220 );
  setFont ( SMALL, 255, 255, 255, 224, 102, 255 );
  myGLCD.print ( "Ultra:", 120, 204 );                 //display UltraViolet LEDs output
  myGLCD.print ( "0", 172, 204 );

  myGLCD.setColor ( 176, 176, 176 );
  myGLCD.fillRoundRect ( 215, 148, 315, 168 );
  setFont ( SMALL, 0, 0, 0, 176, 176, 176 );
  myGLCD.print ( "Lunar:", 225, 152 );                 //display Lunar LEDs output
  myGLCD.print ( "0", 277, 152 );

  myGLCD.setColor ( 0, 0, 0 );
  myGLCD.drawRoundRect ( 215, 174, 315, 194 );         //blank button

  myGLCD.drawRoundRect ( 215, 200, 315, 220 );
  setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
  myGLCD.print ( "<< BACK >>", 227, 204 );             //display "BACK" button

  myGLCD.setColor ( 255, 255, 255 );                   //white Border Around Color Buttons
  for ( int x = 0; x < 3; x++ ) {
    for ( int y = 0; y < 3; y++ ) {
      myGLCD.drawRoundRect ( ( x * 105 ) + 5, ( y * 26 ) + 148, ( x * 105 ) + 105, ( y * 26 ) + 168 );
    }
  }

  Rgad = 0, Ggad = 0, Bgad = 0;
  Rfont = 255, Gfont = 255, Bfont = 255, Rback = 0, Gback = 0, Bback = 0, Rline = 255, Gline = 255, Bline = 255;
  COLOR = 0;
  ledChangerGadget ();
  myGLCD.setColor ( 0, 0, 0 );
  myGLCD.fillRect ( 204, 50, 280, 74 );
  setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
  myGLCD.print ( "SELECT", 220, 50 );
  myGLCD.print ( "BELOW", 224, 62 );
}
/************************ END OF TEST INDIVIDUAL LED(S) SCREEN ************************/

/******** SHOW LED COLOR CHOICES SCREEN ******* dispScreen = 7 ************************/
void ledColorViewScreen () {
  printHeader ( "Individual LED Outputs: Color Choices" );

  myGLCD.setColor ( 64, 64, 64 );                      //Draw Dividers in Grey
  myGLCD.drawRect ( 0, 196, 319, 194 );                //Bottom Horizontal Divider
  printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
  printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.fillRoundRect ( 10, 20, 150, 50 );
  setFont ( SMALL, 0, 0, 0, 255, 255, 255 );
  myGLCD.print ( "White", 60, 29 );

  myGLCD.setColor ( 58, 95, 205 );
  myGLCD.fillRoundRect ( 10, 60, 150, 90 );
  setFont ( SMALL, 255, 255, 255, 58, 95, 205 );
  myGLCD.print ( "Royal Blue", 40, 69 );

  myGLCD.setColor ( 255, 0, 0 );
  myGLCD.fillRoundRect ( 10, 100, 150, 130 );
  setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
  myGLCD.print ( "Red", 68, 109 );

  myGLCD.setColor ( 176, 176, 176 );
  myGLCD.fillRoundRect ( 90, 140, 230, 170 );
  setFont ( SMALL, 0, 0, 0, 176, 176, 176 );
  myGLCD.print ( "Lunar", 140, 149 );

  myGLCD.setColor ( 0, 150, 0 );
  myGLCD.fillRoundRect ( 170, 100, 310, 130 );
  setFont ( SMALL, 255, 255, 255, 0, 150, 0 );
  myGLCD.print ( "Sump", 224, 109 );

  myGLCD.setColor ( 224, 102, 255 );
  myGLCD.fillRoundRect ( 170, 60, 310, 90 );
  setFont ( SMALL, 255, 255, 255, 224, 102, 255 );
  myGLCD.print ( "Ultraviolet", 196, 69 );

  myGLCD.setColor ( 9, 184, 255 );
  myGLCD.fillRoundRect ( 170, 20, 310, 50 );
  setFont ( SMALL, 255, 255, 255, 9, 184, 255 );
  myGLCD.print ( "Blue", 224, 29 );

  myGLCD.setColor ( 255, 255, 255 );
  for ( int x = 0; x < 2; x++ ) {
    for ( int y = 0; y < 3; y++ ) {
      myGLCD.drawRoundRect ( ( x * 160 ) + 10, ( y * 40 ) + 20, ( x * 160 ) + 150, ( y * 40 ) + 50 );
    }
  }
  myGLCD.drawRoundRect ( 90, 140, 230, 170 );
}
/************************** END OF LED COLOR CHOICES SCREEN ***************************/

/****** SHOW LED VALUES FOR CHOICE SCREEN ********** dispScreen = 8 *******************/
void ledValuesScreen () {
  int a;

  if ( COLOR == 1 ) {
    for ( int i; i < 96; i++ )
      tled [i] = wled [i];
    printHeader ( "White LED Output Values" );
  }
  if ( COLOR == 2 ) {
    for ( int i; i < 96; i++ )
      tled [i] = bled [i];
    printHeader ( "Blue LED Output Values" );
  }
  if ( COLOR == 3 ) {
    for ( int i; i < 96; i++ )
      tled [i] = rbled [i];
    printHeader ( "Roayl Blue LED Output Values" );
  }
  if ( COLOR == 4 ) {
    for ( int i; i < 96; i++ )
      tled [i] = rled [i];
    printHeader ( "Red LED Output Values" );
  }
  if ( COLOR == 5 ) {
    for ( int i; i < 96; i++ )
      tled [i] = uvled [i];
    printHeader ( "Ultraviolet LED Output Values" );
  }
  if ( COLOR == 6 ) {
    for ( int i; i < 96; i++ )
      tled [i] = sled [i];
    printHeader ( "Sump LED Output Values" );
  }
  if ( COLOR == 7 ) {
    tMI = MI;
    printHeader ( "View/Change Moon LED Max Output" );
    setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
    myGLCD.print ( "Max Illumination", CENTER, 45 );
    myGLCD.print ( "at Full Moon", CENTER, 65 );
    myGLCD.printNumI ( tMI, CENTER, 120 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
    myGLCD.print ( "(0--255)", CENTER, 90 );
    myGLCD.print ( "-1", 95, 145 );
    myGLCD.print ( "+5", 210, 145 );

    printButton ( "-", miM [0], miM [1], miM [2], miM [3], true );      //Max Illum. minus
    printButton ( "+", miP [0], miP [1], miP [2], miP [3], true );      //Max Illum. plus
    myGLCD.setColor ( 64, 64, 64 );
    myGLCD.drawRect ( 0, 196, 319, 194 );
    printButton ( "MORE COLORS", back [0], back [1], back [2], back [3], SMALL );
    printButton ( "SAVE", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );
    printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );
  }

  if ( COLOR != 7 ) {
    setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
    for ( int i = 0; i < 12; i++ ) {
      myGLCD.setColor ( 0, 255, 255 );
      myGLCD.printNumI ( ( i * 2 ), ( i * 26 ) + 13, 14 );
      myGLCD.printNumI ( ( ( i * 2 ) + 1 ), ( i * 26 ) + 13, 24 );
      for ( int j = 0; j < 8; j++ ) {
        a = ( i * 8 ) + j;
        myGLCD.setColor ( 255, 255, 255 );
        myGLCD.printNumI ( tled [a], ( i * 26 ) + 7, ( j * 18 ) + 39 );
        myGLCD.setColor ( 100, 100, 100 );
        myGLCD.drawRect ( ( i * 26 ) + 4, ( j * 18 ) + 35, ( i * 26 ) + 30, ( j * 18 ) + 53 );
      }
    }
    myGLCD.setColor ( 64, 64, 64 );
    myGLCD.drawRect ( 0, 196, 319, 194 );
    printButton ( "MORE COLORS", back [0], back [1], back [2], back [3], SMALL );
    printButton ( "CHANGE", ledChV [0], ledChV [1], ledChV [2], ledChV [3], SMALL );
    printButton ( "SAVE", eeprom [0], eeprom [1], eeprom [2], eeprom [3] );
  }
}
/*************************** END OF SHOW LED VALUES SCREEN ****************************/

/********** CHANGE LED VALUES SCREEN ********** dispScreen = 9 ************************/
void ledChangeScreen () {
  if ( COLOR == 1 ) {
    printHeader ( "Change White LED Output Values" );
  }
  if ( COLOR == 2 ) {
    printHeader ( "Change Blue LED Output Values" );
  }
  if ( COLOR == 3 ) {
    printHeader ( "Change Royal Blue LED Output Values" );
  }
  if ( COLOR == 4 ) {
    printHeader ( "Change Red LED Output Values" );
  }
  if ( COLOR == 5 ) {
    printHeader ( "Change Ultraviolet LED Output Values" );
  }
  if ( COLOR == 6 ) {
    printHeader ( "Change Sump LED Output Values" );
  }

  myGLCD.setColor ( 64, 64, 64 );
  myGLCD.drawRect ( 0, 196, 319, 194 );
  printButton ( "MENU", back [0], back [1], back [2], back [3], SMALL );
  printButton ( "OK", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );
  printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

  setFont ( SMALL, 0, 255, 255, 0, 0, 0 );
  for ( int i = 0; i < 12; i++ ) {
    myGLCD.setColor ( 0, 255, 255 );
    myGLCD.printNumI ( ( i * 2 ), ( i * 26 ) + 10, 22 );
    myGLCD.printNumI ( ( ( i * 2 ) + 1 ), ( i * 26 ) + 10, 33 );
    myGLCD.setColor ( 100, 100, 100 );
    myGLCD.drawRect ( ( i * 26 ) + 4, 20, ( i * 26 ) + 30, 45 );
  }

  for ( int i = 0; i < 8; i++ ) {
    printButton ( "+", ( i * 38 ) + 10, 70, ( i * 38 ) + 35, 95, LARGE );
    printButton ( "-", ( i * 38 ) + 10, 125, ( i * 38 ) + 35, 150, LARGE );
  }
}
/*************************** END OF CHANGE LED VALUES SCREEN **************************/

/************ WAVEMAKER SCREEN **************** dispScreen = 10 ***********************/
void WaveMakerButtons () {
  myGLCD.setColor ( 255, 0, 0 );
  myGLCD.fillRoundRect ( 5, 20, 155, 40 );        //Alternating Mode Button
  myGLCD.fillRoundRect ( 165, 20, 315, 40 );      //Synchronous Mode Button
  myGLCD.fillRoundRect ( 5, 46, 155, 66 );        //Feeding Mode Button
  myGLCD.fillRoundRect ( 165, 46, 315, 66 );      //Turn Pumps ON/OFF Button
  setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
  myGLCD.print ( "Alternating Mode", 16, 24 );
  myGLCD.print ( "Synchronous Mode", 176, 24 );
  myGLCD.print ( "Feeding Mode", 32, 50 );
  myGLCD.print ( "Turn Pumps OFF", 184, 50 );
}

void WaveMakerScreen () {
  printHeader ( "View/Change Wavemaker Settings" );

  myGLCD.setColor ( 64, 64, 64 );                 //Draw Dividers in Grey
  myGLCD.drawRect ( 0, 70, 319, 72 );             //Top Horizontal Divider
  myGLCD.drawRect ( 0, 196, 319, 194 );           //Bottom Horizontal Divider
  printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
  printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

  setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
  myGLCD.print ( "Current Status:", CENTER, 80 );
}

void viewWaveTimes () {
  setFont ( SMALL, 0, 255, 0, 0, 0, 0 );
  int dynamicXm = 135;
  if ( ( MIN1 <= 99 ) && ( MIN1 >= 10 ) ) {
    dynamicXm += 8;
  }
  if ( MIN1 <= 9 ) {
    dynamicXm += 16;
  }
  myGLCD.printNumI ( MIN1, dynamicXm, minY1 );
  int dynamicXs = 231;
  if ( SEC1 <= 9 ) {
    dynamicXs += 8;
  }
  myGLCD.printNumI ( SEC1, dynamicXs, minY1 );
  dynamicXm = 135;
  if ( ( MIN2 <= 99 ) && ( MIN2 >= 10 ) ) {
    dynamicXm += 8;
  }
  if ( MIN2 <= 9 ) {
    dynamicXm += 16;
  }
  myGLCD.printNumI ( MIN2, dynamicXm, minY2 );
  dynamicXs = 231;
  if ( SEC2 <= 9 ) {
    dynamicXs += 8;
  }
  myGLCD.printNumI ( SEC2, dynamicXs, minY2 );
}

void WaveMakerStatusScreen () {
  myGLCD.setColor ( 0, 0, 0 );                    //Clear Status Area
  myGLCD.fillRect ( 1, 96, 318, 193 );

  WaveMakerButtons ();

  if ( WAVE == 0 ) {
    setFont ( LARGE, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "No Mode Saved!", CENTER, 100 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "Make a selection above. Choose", CENTER, 130 );
    myGLCD.print ( "either Alternating or Synchronous", CENTER, 142 );
    myGLCD.print ( "then save your settings.", CENTER, 154 );
  }
  else

      if ( WAVE == 1 ) {
      myGLCD.setColor ( 0, 255, 0 );
      myGLCD.fillRoundRect ( 5, 20, 155, 40 );
      setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
      myGLCD.print ( "Alternating Mode", 16, 24 );
      myGLCD.setColor ( 255, 255, 255 );
      myGLCD.drawRoundRect ( 5, 20, 155, 40 );

      setFont ( LARGE, 0, 255, 0, 0, 0, 0 );
      myGLCD.print ( "Alternating Flow", CENTER, 100 );

      myGLCD.print ( "Pump1->", 15, 140 );
      myGLCD.print ( "min", 167, 140 );
      myGLCD.print ( "sec", 255, 140 );
      myGLCD.print ( "Pump2->", 15, 160 );
      myGLCD.print ( "min", 167, 160 );
      myGLCD.print ( "sec", 255, 160 );

      MIN1 = Pump1m, SEC1 = Pump1s, MIN2 = Pump2m, SEC2 = Pump2s;
      minY1 = 144, minY2 = 164;
      viewWaveTimes ();
    }
    else

        if ( WAVE == 2 ) {
        myGLCD.setColor ( 0, 255, 0 );
        myGLCD.fillRoundRect ( 165, 20, 315, 40 );
        setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
        myGLCD.print ( "Synchronous Mode", 176, 24 );
        myGLCD.setColor ( 255, 255, 255 );
        myGLCD.drawRoundRect ( 165, 20, 315, 40 );

        setFont ( LARGE, 0, 255, 0, 0, 0, 0 );
        myGLCD.print ( "Synchronous Flow", CENTER, 100 );

        if ( Synch == 1 ) {
          myGLCD.print ( "Constantly ON", CENTER, 116 );
          setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
          myGLCD.print ( "Both powerheads are ON constantly.", CENTER, 144 );
          myGLCD.print ( "To change to Pulsating Mode, push", CENTER, 156 );
          myGLCD.print ( "the Synchronous Mode button above", CENTER, 168 );
        }
        if ( Synch == 2 ) {
          myGLCD.print ( "Pulsating Mode", CENTER, 116 );

          myGLCD.print ( "ON for", 15, 146 );
          myGLCD.print ( "min", 167, 146 );
          myGLCD.print ( "sec", 255, 146 );
          myGLCD.print ( "OFF for", 15, 166 );
          myGLCD.print ( "min", 167, 166 );
          myGLCD.print ( "sec", 255, 166 );

          MIN1 = OnForTm, SEC1 = OnForTs, MIN2 = OffForTm, SEC2 = OffForTs;
          minY1 = 150, minY2 = 170;
          viewWaveTimes ();
        }
      }
      else

          if ( WAVE == 3 ) {
          myGLCD.setColor ( 0, 255, 0 );
          myGLCD.fillRoundRect ( 165, 46, 315, 66 );
          setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
          myGLCD.print ( "Turn Pumps ON", 188, 50 );
          myGLCD.setColor ( 255, 255, 255 );
          myGLCD.drawRoundRect ( 165, 46, 315, 66 );

          setFont ( LARGE, 0, 255, 0, 0, 0, 0 );
          myGLCD.print ( "Powerheads are OFF", CENTER, 100 );
          setFont ( SMALL, 0, 255, 0, 0, 0, 0 );
          myGLCD.print ( "To turn the powerheads back ON", CENTER, 125 );
          if ( MODE == 1 ) {
            myGLCD.print ( "and resume in Alternating Mode,", CENTER, 137 );
          }
          else if ( MODE == 2 ) {
            myGLCD.print ( "and resume in Synchronous Mode,", CENTER, 137 );
          }
          myGLCD.print ( "push the Turn Pumps ON button above", CENTER, 149 );

          waveMakerOff = true;
          digitalWrite ( WaveMakerTop, LOW );
          digitalWrite ( WaveMakerBottom, LOW );
        }

  myGLCD.setColor ( 255, 255, 255 );
  for ( int x = 0; x < 2; x++ ) {
    for ( int y = 0; y < 2; y++ ) {
      myGLCD.drawRoundRect ( ( x * 160 ) + 5, ( y * 26 ) + 20, ( x * 160 ) + 155, ( y * 26 ) + 40 );
    }
  }

  if ( WAVE == 4 ) {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 5, 46, 155, 66 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "Feeding Mode", 32, 50 );
    myGLCD.setColor ( 255, 255, 255 );
    for ( int x = 0; x < 2; x++ ) {
      for ( int y = 0; y < 2; y++ ) {
        myGLCD.drawRoundRect ( ( x * 160 ) + 5, ( y * 26 ) + 20, ( x * 160 ) + 155, ( y * 26 ) + 40 );
      }
    }

    setFont ( LARGE, 0, 255, 0, 0, 0, 0 );
    myGLCD.print ( "Feeding", CENTER, 100 );
    setFont ( SMALL, 0, 255, 0, 0, 0, 0 );
    myGLCD.print ( "Powerheads will remain OFF", CENTER, 125 );
    myGLCD.print ( "for 5 minutes", CENTER, 137 );
    if ( MODE == 1 ) {
      myGLCD.print ( "Alternating Mode will resume in:", CENTER, 155 );
    }
    else if ( MODE == 2 ) {
      myGLCD.print ( "Synchronous Mode will resume in:", CENTER, 155 );
    }

    waveMakerOff = true;
    digitalWrite ( WaveMakerTop, LOW );
    digitalWrite ( WaveMakerBottom, LOW );
  }
  while ( WAVE == 4 ) {
    unsigned long currentMillis = millis ();

    if ( myTouch.dataAvailable () ) {
      processMyTouch ();
    }

    if ( currentMillis - previousMillisCt >= intervalCt ) {
      previousMillisCt = currentMillis;
      if ( countDown >= 0 ) {
        if ( countDown <= 30 ) {
          myGLCD.setColor ( 0, 0, 0 );
          myGLCD.fillRect ( 128, 173, 192, 189 );
        }
        MIN_O = ( countDown / 60 ) % 10;
        SEC_T = ( countDown % 60 ) / 10;
        SEC_O = ( countDown % 60 ) % 10;
        setFont ( LARGE, 255, 0, 0, 0, 0, 0 );
        myGLCD.printNumI ( MIN_O, 128, 173 );
        myGLCD.print ( ":", 144, 173 );
        myGLCD.printNumI ( SEC_T, 160, 173 );
        myGLCD.printNumI ( SEC_O, 176, 173 );
        if ( countDown == 0 ) {
          WAVE = MODE;
          WaveMakerStatusScreen ();
        }
        countDown--;
        checkTempC ();
        TimeDateBar ();
      }
      waveMakerOff = false;
    }
  }
}
/******************************* END OF WAVEMAKER SCREEN ******************************/

/******** WAVEMAKER SETTINGS SCREEN ************** dispScreen = 11 ********************/
void WaveMakerSettingsScreen () {
  myGLCD.setColor ( 64, 64, 64 );                 //Draw Dividers in Grey
  myGLCD.drawRect ( 0, 196, 319, 194 );           //Bottom Horizontal Divider

  myGLCD.setColor ( 0, 0, 255 );
  myGLCD.fillRoundRect ( 5, 200, 105, 220 );      //BACK Button
  myGLCD.fillRoundRect ( 110, 200, 210, 220 );    //TEST Button
  myGLCD.fillRoundRect ( 215, 200, 315, 220 );    //SAVE Button

  setFont ( SMALL, 255, 255, 255, 0, 0, 255 );
  myGLCD.print ( "<< BACK >>", 15, 204 );
  myGLCD.print ( "TEST", 144, 204 );
  myGLCD.print ( "SAVE", 241, 204 );

  myGLCD.setColor ( 255, 255, 255 );
  for ( int x = 0; x < 3; x++ ) {
    myGLCD.drawRoundRect ( ( x * 105 ) + 5, 200, ( x * 105 ) + 105, 220 );
  }

  setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
  if ( WAVE == 1 ) {
    printHeader ( "Alternating Flow Mode Settings" );
    myGLCD.setColor ( 64, 64, 64 );                 //Draw Dividers in Grey
    myGLCD.drawRect ( 0, 38, 319, 40 );             //Top Horizontal Divider
    myGLCD.drawRect ( 0, 115, 319, 117 );           //Middle Horizontal Divider
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "Set how long each powerhead will be ON", CENTER, 20 );
    setFont ( SMALL, 0, 255, 0, 0, 0, 0 );
    myGLCD.print ( "POWERHEAD 1", CENTER, 50 );
    myGLCD.print ( "POWERHEAD 2", CENTER, 127 );
    setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
    myGLCD.print ( "Minutes", 57, 100 );
    myGLCD.print ( "Seconds", 211, 100 );
    myGLCD.print ( "Minutes", 57, 177 );
    myGLCD.print ( "Seconds", 211, 177 );

    Min1 = Pump1m, Sec1 = Pump1s, Min2 = Pump2m, Sec2 = Pump2s;
    viewWaveTimesPage ();
  }

  if ( WAVE == 2 ) {
    printHeader ( "Synchronous Flow Mode Settings" );
    myGLCD.setColor ( 64, 64, 64 );                 //Draw Dividers in Grey
    myGLCD.drawRect ( 0, 44, 319, 46 );             //Top Horizontal Divider
  }
}

void synchronousSynch () {
  myGLCD.setColor ( 0, 0, 0 );                       //Clear Status Area
  myGLCD.fillRect ( 1, 47, 318, 193 );
  myGLCD.setColor ( 255, 0, 0 );
  myGLCD.fillRoundRect ( 5, 20, 155, 40 );           //Constantly ON Mode Button
  myGLCD.fillRoundRect ( 165, 20, 315, 40 );         //Pulsating Mode Mode Button
  setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
  myGLCD.print ( "Constantly ON", 28, 24 );
  myGLCD.print ( "Pulsating Mode", 184, 24 );
  for ( int x = 0; x < 2; x++ ) {
    myGLCD.drawRoundRect ( ( x * 160 ) + 5, 20, ( x * 160 ) + 155, 40 );
  }

  if ( Synch == 0 ) {
    setFont ( LARGE, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "NO Synchronous Mode", CENTER, 80 );
    myGLCD.print ( "has been selected!", CENTER, 100 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "Choose either Constantly ON or", CENTER, 130 );
    myGLCD.print ( "Pulsating Mode. Afterwards, you", CENTER, 142 );
    myGLCD.print ( "can alter the settings.", CENTER, 154 );
  }
  else

      if ( Synch == 1 ) {
      myGLCD.setColor ( 0, 255, 0 );
      myGLCD.fillRoundRect ( 5, 20, 155, 40 );
      setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
      myGLCD.print ( "Constantly ON", 28, 24 );
      myGLCD.setColor ( 255, 255, 255 );
      myGLCD.drawRoundRect ( 5, 20, 155, 40 );

      setFont ( LARGE, 0, 255, 0, 0, 0, 0 );
      myGLCD.print ( "Constantly ON Mode", CENTER, 80 );
      myGLCD.print ( "has been selected!", CENTER, 100 );
      setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
      myGLCD.print ( "Both powerheads will remain", CENTER, 130 );
      myGLCD.print ( "ON constantly. To change to", CENTER, 142 );
      myGLCD.print ( "Pulsating Mode select it above.", CENTER, 154 );
    }
    else

        if ( Synch == 2 ) {
        myGLCD.setColor ( 0, 255, 0 );
        myGLCD.fillRoundRect ( 165, 20, 315, 40 );
        setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
        myGLCD.print ( "Pulsating Mode", 184, 24 );
        myGLCD.setColor ( 255, 255, 255 );
        myGLCD.drawRoundRect ( 165, 20, 315, 40 );

        myGLCD.setColor ( 64, 64, 64 );                 //Draw Dividers in Grey
        myGLCD.drawRect ( 0, 120, 319, 122 );           //Middle Horizontal Divider
        setFont ( SMALL, 0, 255, 0, 0, 0, 0 );
        myGLCD.print ( "POWERHEADS WIll RUN FOR", CENTER, 50 );
        myGLCD.print ( "POWERHEADS WILL STOP FOR", CENTER, 127 );
        setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
        myGLCD.print ( "Minutes", 57, 100 );
        myGLCD.print ( "Seconds", 211, 100 );
        myGLCD.print ( "Minutes", 57, 177 );
        myGLCD.print ( "Seconds", 211, 177 );

        Min1 = OnForTm, Sec1 = OnForTs, Min2 = OffForTm, Sec2 = OffForTs;
        viewWaveTimesPage ();
      }
}

void viewWaveTimesPage () {
  setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
  if ( Min1 == 0 ) {
    myGLCD.print ( "000", 59, 75 );
  }
  if ( ( Min1 >= 1 ) && ( Min1 <= 9 ) ) {
    myGLCD.print ( "00", 59, 75 );
    myGLCD.printNumI ( Min1, 91, 75 );
  }
  if ( ( Min1 >= 10 ) && ( Min1 <= 99 ) ) {
    myGLCD.print ( "0", 59, 75 );
    myGLCD.printNumI ( Min1, 75, 75 );
  }
  if ( Min1 >= 100 ) {
    myGLCD.printNumI ( Min1, 59, 75 );
  }

  if ( Sec1 == 0 ) {
    myGLCD.print ( "00", 221, 75 );
  }
  if ( ( Sec1 >= 1 ) && ( Sec1 <= 9 ) ) {
    myGLCD.print ( "0", 221, 75 );
    myGLCD.printNumI ( Sec1, 237, 75 );
  }
  if ( Sec1 >= 10 ) {
    myGLCD.printNumI ( Sec1, 221, 75 );
  }

  if ( Min2 == 0 ) {
    myGLCD.print ( "000", 59, 152 );
  }
  if ( ( Min2 >= 1 ) && ( Min2 <= 9 ) ) {
    myGLCD.print ( "00", 59, 152 );
    myGLCD.printNumI ( Min2, 91, 152 );
  }
  if ( ( Min2 >= 10 ) && ( Min2 <= 99 ) ) {
    myGLCD.print ( "0", 59, 152 );
    myGLCD.printNumI ( Min2, 75, 152 );
  }
  if ( Min2 >= 100 ) {
    myGLCD.printNumI ( Min2, 59, 152 );
  }

  if ( Sec2 == 0 ) {
    myGLCD.print ( "00", 221, 152 );
  }
  if ( ( Sec2 >= 1 ) && ( Sec2 <= 9 ) ) {
    myGLCD.print ( "0", 221, 152 );
    myGLCD.printNumI ( Sec2, 237, 152 );
  }
  if ( Sec2 >= 10 ) {
    myGLCD.printNumI ( Sec2, 221, 152 );
  }

  printButton ( "-", pump1Mm [0], pump1Mm [1], pump1Mm [2], pump1Mm [3], LARGE );
  printButton ( "+", pump1Mp [0], pump1Mp [1], pump1Mp [2], pump1Mp [3], LARGE );
  printButton ( "-", pump1Sm [0], pump1Sm [1], pump1Sm [2], pump1Sm [3], LARGE );
  printButton ( "+", pump1Sp [0], pump1Sp [1], pump1Sp [2], pump1Sp [3], LARGE );
  printButton ( "-", pump2Mm [0], pump2Mm [1], pump2Mm [2], pump2Mm [3], LARGE );
  printButton ( "+", pump2Mp [0], pump2Mp [1], pump2Mp [2], pump2Mp [3], LARGE );
  printButton ( "-", pump2Sm [0], pump2Sm [1], pump2Sm [2], pump2Sm [3], LARGE );
  printButton ( "+", pump2Sp [0], pump2Sp [1], pump2Sp [2], pump2Sp [3], LARGE );
}

void waveModePlusMinus () {
  setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
  if ( ( y >= 70 ) && ( y <= 95 ) )                        //First Row
  {
    if ( ( x >= 21 ) && ( x <= 46 ) )                     //Pump 1 Minute Minus
    {
      waitForIt ( 21, 70, 46, 95 );
      Min1 -= 1;
      if ( ( min1X >= 10 ) && ( Min1 <= 99 ) ) {
        min1X = 75;
        myGLCD.print ( "0", 59, 75 );
      }
      if ( ( Min1 <= 9 ) && ( Min1 > 0 ) ) {
        min1X = 91;
        myGLCD.print ( "00", 59, 75 );
      }
      if ( Min1 <= 0 ) {
        Min1 = 0;
        myGLCD.print ( "0", 91, 75 );
      }
      setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
      myGLCD.printNumI ( Min1, min1X, 75 );
    }
    if ( ( x >= 120 ) && ( x <= 145 ) )                   //Pump 1 Minute Plus
    {
      waitForIt ( 120, 70, 145, 95 );
      Min1 += 1;
      if ( ( Min1 >= 0 ) && ( Min1 <= 9 ) ) {
        min1X = 91;
      }
      if ( Min1 > 9 ) {
        min1X = 75;
      }
      if ( Min1 > 99 ) {
        min1X = 59;
      }
      if ( Min1 >= 999 ) {
        Min1 = 999;
      }
      setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
      myGLCD.printNumI ( Min1, min1X, 75 );
    }
    if ( ( x >= 175 ) && ( x <= 200 ) )                   //Pump 1 Second Minus
    {
      waitForIt ( 175, 70, 200, 95 );
      Sec1 -= 1;
      if ( ( sec1X >= 10 ) && ( Sec1 <= 59 ) ) {
        sec1X = 221;
      }
      if ( ( Sec1 <= 9 ) && ( Sec1 > 0 ) ) {
        sec1X = 237;
        myGLCD.print ( "0", 221, 75 );
      }
      if ( Sec1 <= 0 ) {
        Sec1 = 0;
        myGLCD.print ( "0", 237, 75 );
      }
      setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
      myGLCD.printNumI ( Sec1, sec1X, 75 );
    }
    if ( ( x >= 274 ) && ( x <= 299 ) )                   //Pump 1 Second Plus
    {
      waitForIt ( 274, 70, 299, 95 );
      Sec1 += 1;
      if ( ( Sec1 >= 0 ) && ( Sec1 <= 9 ) ) {
        sec1X = 237;
      }
      if ( Sec1 > 9 ) {
        sec1X = 221;
      }
      if ( Sec1 >= 59 ) {
        Sec1 = 59;
      }
      setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
      myGLCD.printNumI ( Sec1, sec1X, 75 );
    }
  }

  if ( ( y >= 147 ) && ( y <= 172 ) )                      //Second Row
  {
    if ( ( x >= 21 ) && ( x <= 46 ) )                        //Pump 2 Minute Minus
    {
      waitForIt ( 21, 147, 46, 172 );
      Min2 -= 1;
      if ( ( min2X >= 10 ) && ( Min2 <= 99 ) ) {
        min2X = 75;
        myGLCD.print ( "0", 59, 152 );
      }
      if ( ( Min2 <= 9 ) && ( Min2 > 0 ) ) {
        min2X = 91;
        myGLCD.print ( "00", 59, 152 );
      }
      if ( Min2 <= 0 ) {
        Min2 = 0;
        myGLCD.print ( "0", 91, 152 );
      }
      setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
      myGLCD.printNumI ( Min2, min2X, 152 );
    }
    if ( ( x >= 120 ) && ( x <= 145 ) )                      //Pump 2 Minute Plus
    {
      waitForIt ( 120, 147, 145, 172 );
      Min2 += 1;
      if ( ( Min2 >= 0 ) && ( Min2 <= 9 ) ) {
        min2X = 91;
      }
      if ( Min2 > 9 ) {
        min2X = 75;
      }
      if ( Min2 > 99 ) {
        min2X = 59;
      }
      if ( Min2 >= 999 ) {
        Min2 = 999;
      }
      setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
      myGLCD.printNumI ( Min2, min2X, 152 );
    }
    if ( ( x >= 175 ) && ( x <= 200 ) )                      //Pump 2 Second Minus
    {
      waitForIt ( 175, 147, 200, 172 );
      Sec2 -= 1;
      if ( ( sec2X >= 10 ) && ( Sec2 <= 59 ) ) {
        sec2X = 221;
      }
      if ( ( Sec2 <= 9 ) && ( Sec2 > 0 ) ) {
        sec2X = 237;
        myGLCD.print ( "0", 221, 152 );
      }
      if ( Sec2 <= 0 ) {
        Sec2 = 0;
        myGLCD.print ( "0", 237, 152 );
      }
      setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
      myGLCD.printNumI ( Sec2, sec2X, 152 );
    }
    if ( ( x >= 274 ) && ( x <= 299 ) )                      //Pump 2 Second Plus
    {
      waitForIt ( 274, 147, 299, 172 );
      Sec2 += 1;
      if ( ( Sec2 >= 0 ) && ( Sec2 <= 9 ) ) {
        sec2X = 237;
      }
      if ( Sec2 > 9 ) {
        sec2X = 221;
      }
      if ( Sec2 >= 59 ) {
        Sec2 = 59;
      }
      setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
      myGLCD.printNumI ( Sec2, sec2X, 152 );
    }
  }
}
/************************** END OF WAVEMAKER SETTINGS SCREEN **************************/

/******** GENERAL SETTINGS SCREEN ************* dispScreen = 12 ***********************/
void generalSettingsScreen () {
  printHeader ( "View/Change General Settings" );

  myGLCD.setColor ( 64, 64, 64 );
  myGLCD.drawRect ( 0, 196, 319, 194 );
  for ( int x = 0; x < 4; x++ ) {
    myGLCD.drawLine ( 0, ( x * 31 ) + 70, 319, ( x * 31 ) + 70 );
  }

  printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
  printButton ( "SAVE", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );
  printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

  setFont ( SMALL, 0, 255, 0, 0, 0, 0 );
  myGLCD.print ( "Calendar Format", 25, 36 );
  myGLCD.print ( "Time Format", 25, 80 );
  myGLCD.print ( "Temperature Scale", 25, 111 );
  myGLCD.print ( "Screensaver", 25, 142 );
  myGLCD.print ( "Auto-Stop on Feed", 25, 173 );

  genSetSelect ();
}
/*********************** END OF GENERAL SETTINGS SETTINGS SCREEN **********************/

/******** AUTOMATIC FEEDER SCREEN ************* dispScreen = 13 ***********************/
void autoFeederScreen () {
  printHeader ( "Automatic Fish Feeder Page" );

  myGLCD.setColor ( 64, 64, 64 );
  myGLCD.drawRect ( 0, 196, 319, 194 );
  printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
  printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );
  myGLCD.setColor ( 64, 64, 64 );
  myGLCD.drawRect ( 159, 194, 161, 121 );
  myGLCD.drawRoundRect ( 78, 87, 242, 121 );
  myGLCD.drawRoundRect ( 80, 89, 240, 119 );
  myGLCD.drawRect ( 0, 103, 78, 105 );
  myGLCD.drawRect ( 242, 103, 319, 105 );
  myGLCD.drawLine ( 159, 87, 159, 14 );
  myGLCD.drawLine ( 161, 87, 161, 14 );
  myGLCD.setColor ( 0, 0, 0 );
  myGLCD.drawLine ( 160, 195, 160, 193 );
  myGLCD.drawLine ( 160, 122, 160, 120 );
  myGLCD.drawLine ( 77, 104, 79, 104 );
  myGLCD.drawLine ( 241, 104, 243, 104 );
  myGLCD.drawLine ( 160, 88, 160, 86 );

  myGLCD.setColor ( 153, 0, 102 );
  myGLCD.fillRoundRect ( 85, 94, 235, 114 );           //Feed Fish Now Button
  myGLCD.setColor ( 255, 255, 255 );
  myGLCD.drawRoundRect ( 85, 94, 235, 114 );
  setFont ( SMALL, 255, 255, 255, 153, 0, 102 );
  myGLCD.print ( "Feed Fish Now!", 106, 98 );
  //TODO: Zeiten aufschreiben
  if ( FEEDTime1 == 0 )                                 //Feeding Time 1 Button
  {
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRoundRect ( 5, 20, 155, 40 );
    setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
    myGLCD.print ( "Feeding Time 1", 24, 24 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "This time has not", 12, 52 );
    myGLCD.print ( "been scheduled", 24, 65 );
  }
  else {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 5, 20, 155, 40 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "Feeding Time 1", 24, 24 );
    timeDispH = feedFish1H;
    timeDispM = feedFish1M;
    if ( setTimeFormat == 0 ) {
      xTimeH = 40;
    }
    if ( setTimeFormat == 1 ) {
      xTimeH = 16;
    }
    ReadFromEEPROM ();
    if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) {
      AM_PM = 1;
    }
    else {
      AM_PM = 2;
    }
    yTime = 56;
    xColon = xTimeH + 32;
    xTimeM10 = xTimeH + 48;
    xTimeM1 = xTimeH + 64;
    xTimeAMPM = xTimeH + 96;
    timeCorrectFormat ();
  }
  if ( FEEDTime2 == 0 )                                 //Feeding Time 2 Button
  {
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRoundRect ( 165, 20, 315, 40 );
    setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
    myGLCD.print ( "Feeding Time 2", 184, 24 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "This time has not", 172, 52 );
    myGLCD.print ( "been scheduled", 184, 65 );
  }
  else {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 165, 20, 315, 40 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "Feeding Time 2", 184, 24 );
    timeDispH = feedFish2H;
    timeDispM = feedFish2M;
    if ( setTimeFormat == 0 ) {
      xTimeH = 200;
    }
    if ( setTimeFormat == 1 ) {
      xTimeH = 176;
    }
    ReadFromEEPROM ();
    if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) {
      AM_PM = 1;
    }
    else {
      AM_PM = 2;
    }
    yTime = 56;
    xColon = xTimeH + 32;
    xTimeM10 = xTimeH + 48;
    xTimeM1 = xTimeH + 64;
    xTimeAMPM = xTimeH + 96;
    timeCorrectFormat ();
  }
  if ( FEEDTime3 == 0 )                                 //Feeding Time 3 Button
  {
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRoundRect ( 5, 168, 155, 188 );
    setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
    myGLCD.print ( "Feeding Time 3", 24, 172 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "This time has not", 12, 133 );
    myGLCD.print ( "been scheduled", 24, 146 );
  }
  else {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 5, 168, 155, 188 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "Feeding Time 3", 24, 172 );
    timeDispH = feedFish3H;
    timeDispM = feedFish3M;
    if ( setTimeFormat == 0 ) {
      xTimeH = 40;
    }
    if ( setTimeFormat == 1 ) {
      xTimeH = 16;
    }
    ReadFromEEPROM ();
    if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) {
      AM_PM = 1;
    }
    else {
      AM_PM = 2;
    }
    yTime = 137;
    xColon = xTimeH + 32;
    xTimeM10 = xTimeH + 48;
    xTimeM1 = xTimeH + 64;
    xTimeAMPM = xTimeH + 96;
    timeCorrectFormat ();
  }
  if ( FEEDTime4 == 0 )                                 //Feeding Time 4 Button
  {
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRoundRect ( 165, 168, 315, 188 );
    setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
    myGLCD.print ( "Feeding Time 4", 184, 172 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "This time has not", 172, 133 );
    myGLCD.print ( "been scheduled", 184, 146 );
  }
  else {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 165, 168, 315, 188 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "Feeding Time 4", 184, 172 );
    timeDispH = feedFish4H;
    timeDispM = feedFish4M;
    if ( setTimeFormat == 0 ) {
      xTimeH = 200;
    }
    if ( setTimeFormat == 1 ) {
      xTimeH = 176;
    }
    ReadFromEEPROM ();
    if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) {
      AM_PM = 1;
    }
    else {
      AM_PM = 2;
    }
    yTime = 137;
    xColon = xTimeH + 32;
    xTimeM10 = xTimeH + 48;
    xTimeM1 = xTimeH + 64;
    xTimeAMPM = xTimeH + 96;
    timeCorrectFormat ();
  }

  myGLCD.setColor ( 255, 255, 255 );
  for ( int x = 0; x < 2; x++ ) {
    for ( int y = 0; y < 2; y++ ) {
      myGLCD.drawRoundRect ( ( x * 160 ) + 5, ( y * 148 ) + 20, ( x * 160 ) + 155, ( y * 148 ) + 40 );
    }
  }
}

void feedingTimeOutput () {
  if ( ( FEEDTime1 == 1 ) && ( feedFish1H == rtc [2] ) && ( feedFish1M == rtc [1] ) && ( rtc [0] <= 5 ) ) {
    if ( setAutoStop == 1 ) {
      waveMakerOff = true;
      digitalWrite ( WaveMakerTop, LOW );
      digitalWrite ( WaveMakerBottom, LOW );
      fiveTillBackOn1 = 0;
      FeedWaveCtrl_1 = true;
    }
    digitalWrite ( autoFeeder, HIGH );
    delay ( 6000 );
    RTC.get ( rtc, true );
    digitalWrite ( autoFeeder, LOW );
  }
  if ( FeedWaveCtrl_1 == true ) {
    fiveTillBackOn1++;
    if ( fiveTillBackOn1 > 60 )                       //60 is 5 minutes (60/12=5)
    {
      waveMakerOff = false;
      FeedWaveCtrl_1 = false;
    }
  }

  if ( ( FEEDTime2 == 1 ) && ( feedFish2H == rtc [2] ) && ( feedFish2M == rtc [1] ) && ( rtc [0] <= 5 ) ) {
    if ( setAutoStop == 1 ) {
      waveMakerOff = true;
      digitalWrite ( WaveMakerTop, LOW );
      digitalWrite ( WaveMakerBottom, LOW );
      fiveTillBackOn2 = 0;
      FeedWaveCtrl_2 = true;
    }
    digitalWrite ( autoFeeder, HIGH );
    delay ( 6000 );
    RTC.get ( rtc, true );
    digitalWrite ( autoFeeder, LOW );
  }
  if ( FeedWaveCtrl_2 == true ) {
    fiveTillBackOn2++;
    if ( fiveTillBackOn2 > 60 ) {
      waveMakerOff = false;
      FeedWaveCtrl_2 = false;
    }
  }

  if ( ( FEEDTime3 == 1 ) && ( feedFish3H == rtc [2] ) && ( feedFish3M == rtc [1] ) && ( rtc [0] <= 5 ) ) {
    if ( setAutoStop == 1 ) {
      waveMakerOff = true;
      digitalWrite ( WaveMakerTop, LOW );
      digitalWrite ( WaveMakerBottom, LOW );
      fiveTillBackOn3 = 0;
      FeedWaveCtrl_3 = true;
    }
    digitalWrite ( autoFeeder, HIGH );
    delay ( 6000 );
    RTC.get ( rtc, true );
    digitalWrite ( autoFeeder, LOW );
  }
  if ( FeedWaveCtrl_3 == true ) {
    fiveTillBackOn3++;
    if ( fiveTillBackOn3 > 60 ) {
      waveMakerOff = false;
      FeedWaveCtrl_3 = false;
    }
  }

  if ( ( FEEDTime4 == 1 ) && ( feedFish4H == rtc [2] ) && ( feedFish4M == rtc [1] ) && ( rtc [0] <= 5 ) ) {
    if ( setAutoStop == 1 ) {
      waveMakerOff = true;
      digitalWrite ( WaveMakerTop, LOW );
      digitalWrite ( WaveMakerBottom, LOW );
      fiveTillBackOn4 = 0;
      FeedWaveCtrl_4 = true;
    }
    digitalWrite ( autoFeeder, HIGH );
    delay ( 6000 );
    RTC.get ( rtc, true );
    digitalWrite ( autoFeeder, LOW );
  }
  if ( FeedWaveCtrl_4 == true ) {
    fiveTillBackOn4++;
    if ( fiveTillBackOn4 > 60 ) {
      waveMakerOff = false;
      FeedWaveCtrl_4 = false;
    }
  }
}
/*********************** END OF AUTOMATIC FEEDER SETTINGS SCREEN **********************/

/***** SET AUTOMATIC FEEDER TIMES SCREEN ********** dispScreen = 14 *******************/
void setFeederTimesScreen ( boolean refreshAll = true ) {
  if ( feedTime == 1 ) {
    printHeader ( "Set Feeding Time 1" );
  }
  if ( feedTime == 2 ) {
    printHeader ( "Set Feeding Time 2" );
  }
  if ( feedTime == 3 ) {
    printHeader ( "Set Feeding Time 3" );
  }
  if ( feedTime == 4 ) {
    printHeader ( "Set Feeding Time 4" );
  }

  if ( refreshAll ) {
    for ( int i = 0; i < 7; i++ ) {
      rtcSet [i] = rtc [i];
    }

    myGLCD.setColor ( 64, 64, 64 );
    myGLCD.drawRect ( 0, 196, 319, 194 );
    printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
    printButton ( "SAVE", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );
    printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

    feedingTimeOnOff ();

    printButton ( "+", houP [0], houP [1], houP [2], houP [3], true );     //hour up
    printButton ( "+", minP [0], minP [1], minP [2], minP [3], true );     //min up
    printButton ( "-", houM [0], houM [1], houM [2], houM [3], true );     //hour down
    printButton ( "-", minM [0], minM [1], minM [2], minM [3], true );     //min down
    if ( setTimeFormat == 1 ) {
      printButton ( "+", ampmP [0], ampmP [1], ampmP [2], ampmP [3], true );  //AM/PM up
      printButton ( "-", ampmM [0], ampmM [1], ampmM [2], ampmM [3], true );
    }  //AM/PM down
  }

  timeDispH = rtcSet [2];
  timeDispM = rtcSet [1];
  xTimeH = 107;
  yTime = 68;
  xColon = xTimeH + 42;
  xTimeM10 = xTimeH + 70;
  xTimeM1 = xTimeH + 86;
  xTimeAMPM = xTimeH + 155;
  timeChange ();
}
/********************** END OF SET AUTOMATIC FEEDER TIMES SCREEN **********************/

/************** ABOUT SCREEN ****************** dispScreen = 15 ***********************/
void AboutScreen () {
  printHeader ( "AquaDino Aquarium Controller v.1.0" );

  setFont ( SMALL, 0, 255, 0, 0, 0, 0 );
  myGLCD.print ( "Release Date: April 2013", CENTER, 20 );
  myGLCD.print ( "Written by Helge Stasch", CENTER, 32 );

  myGLCD.print ( "Main code based on Jarduino and Stilo", 5, 52 );
  myGLCD.print ( "http://code.google.com/p/stilo/", 5, 64 );

  myGLCD.print ( "LED controlling algorithm based on", 5, 84 );
  myGLCD.print ( "Krusduino by Hugh Dangerfield", 5, 96 );
  myGLCD.print ( "and Dave Rosser", 5, 108 );
  myGLCD.print ( "http://code.google.com/p/dangerduino/", 5, 120 );

  myGLCD.print ( "Special Thanks: Hugh & Dave, Kev Tench,", 5, 140 );
  myGLCD.print ( "Mark Chester, Ned Simpson, Stilo, Neil", 5, 152 );
  myGLCD.print ( "Williams", 5, 164 );

  myGLCD.setColor ( 64, 64, 64 );
  myGLCD.drawRect ( 0, 196, 319, 194 );
  printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
  printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );
}
/********************************* END OF ABOUT SCREEN ********************************/

/********************************* END OF Menu two Screen ********************************/

/********************************* Dosing Pump main Options Screen *********************/
void dosingPumpMainMenu () {
  int mlPump, mlrestpump;
  printHeader ( "Dosing Pump Page" );
  //	Serial.println ( "DosingMainMenu triggerd" );
  myGLCD.setColor ( 64, 64, 64 );
  myGLCD.drawRect ( 0, 196, 319, 194 );
  printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
  printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );
  myGLCD.setColor ( 64, 64, 64 );
  myGLCD.drawRect ( 159, 194, 161, 14 );  // -> unten mitte / bis mitte
  //	myGLCD.drawRoundRect ( 78, 87, 242, 121 );-> mittlered dingen
  //	myGLCD.drawRoundRect ( 80, 89, 240, 119 );-> mittlered dingen
  myGLCD.drawRect ( 0, 103, 319, 105 );  // rechts lkinks
  //	myGLCD.drawRect ( 242, 103, 319, 105 );
  //	myGLCD.drawLine ( 159, 87, 159, 14 );
  //	myGLCD.drawLine ( 161, 87, 161, 14 );
  //	myGLCD.setColor ( 0, 0, 0 );
  //	myGLCD.drawLine ( 160, 195, 160, 193 );
  //	myGLCD.drawLine ( 160, 122, 160, 120 );
  //	myGLCD.drawLine ( 77, 104, 79, 104 );
  //	myGLCD.drawLine ( 241, 104, 243, 104 );
  //	myGLCD.drawLine ( 160, 88, 160, 86 );

  myGLCD.setColor ( 153, 0, 102 );
  ReadFromEEPROM ();

  if ( pump1On == 0 )                                 //Feeding Time 1 Button
  {
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRoundRect ( 5, 20, 155, 40 );
    setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
    myGLCD.print ( "Dosing Pump 1", 24, 24 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "Dosing Pump 1", 12, 52 );
    myGLCD.print ( "is not scheduled", 24, 65 );
  }
  else {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 5, 20, 155, 40 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "Dosing Pump 1", 24, 24 );
    //		myGLCD.setColor ( 255, 0, 0 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    timeDispH = pump1_1h;
    timeDispM = pump1_1m;
    buildCorrectTime ();
    mlPump = pump1_1ml / 10;
    mlrestpump = ( pump1_1ml % 10 );
    sprintf ( otherData, "1: %6s %3i.%iml", time, mlPump, mlrestpump );
    myGLCD.print ( otherData, 5, 44 );

    timeDispH = pump1_2h;
    timeDispM = pump1_2m;
    buildCorrectTime ();
    mlPump = pump1_2ml / 10;
    mlrestpump = ( pump1_2ml % 10 );
    sprintf ( otherData, "2: %6s %3i.%iml", time, mlPump, mlrestpump );
    myGLCD.print ( otherData, 5, 63 );

    timeDispH = pump1_3h;
    timeDispM = pump1_3m;
    buildCorrectTime ();
    mlPump = pump1_3ml / 10;
    mlrestpump = ( pump1_3ml % 10 );
    sprintf ( otherData, "3: %6s %3i.%iml", time, mlPump, mlrestpump );
    myGLCD.print ( otherData, 5, 82 );
  }
  if ( pump2On == 0 )                                 //Feeding Time 2 Button
  {
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRoundRect ( 165, 20, 315, 40 );
    setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
    myGLCD.print ( "Dosing Pump 2", 184, 24 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "Dosing Pump 2", 172, 52 );
    myGLCD.print ( "is not scheduled", 184, 65 );
  }
  else {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 165, 20, 315, 40 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "Dosing Pump 2", 184, 24 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    timeDispH = pump2_1h;
    timeDispM = pump2_1m;
    buildCorrectTime ();
    mlPump = pump2_1ml / 10;
    mlrestpump = ( pump2_1ml % 10 );
    sprintf ( otherData, "1: %6s %3i.%iml", time, mlPump, mlrestpump );
    myGLCD.print ( otherData, 170, 44 );

    timeDispH = pump2_2h;
    timeDispM = pump2_2m;
    buildCorrectTime ();
    mlPump = pump2_2ml / 10;
    mlrestpump = ( pump2_2ml % 10 );
    sprintf ( otherData, "2: %6s %3i.%iml", time, mlPump, mlrestpump );
    myGLCD.print ( otherData, 170, 63 );

    timeDispH = pump2_3h;
    timeDispM = pump2_3m;
    buildCorrectTime ();
    mlPump = pump2_3ml / 10;
    mlrestpump = ( pump2_3ml % 10 );
    sprintf ( otherData, "3: %6s %3i.%iml", time, mlPump, mlrestpump );
    myGLCD.print ( otherData, 170, 82 );
  }
  if ( pump3On == 0 )                                 //Feeding Time 3 Button
  {
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRoundRect ( 5, 168, 155, 188 );
    setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
    myGLCD.print ( "Dosing Pump 3", 24, 172 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "Dosing Pump 3", 12, 133 );
    myGLCD.print ( "is not scheduled", 24, 146 );
  }
  else {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 5, 168, 155, 188 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "Dosing Pump 3", 24, 172 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    timeDispH = pump3_1h;
    timeDispM = pump3_1m;
    buildCorrectTime ();
    mlPump = pump3_1ml / 10;
    mlrestpump = ( pump3_1ml % 10 );
    sprintf ( otherData, "1: %6s %3i.%iml", time, mlPump, mlrestpump );
    myGLCD.print ( otherData, 5, 113 );

    timeDispH = pump3_2h;
    timeDispM = pump3_2m;
    buildCorrectTime ();
    mlPump = pump3_2ml / 10;
    mlrestpump = ( pump3_2ml % 10 );
    sprintf ( otherData, "2: %6s %3i.%iml", time, mlPump, mlrestpump );
    myGLCD.print ( otherData, 5, 132 );

    timeDispH = pump3_3h;
    timeDispM = pump3_3m;
    buildCorrectTime ();
    mlPump = pump3_3ml / 10;
    mlrestpump = ( pump3_3ml % 10 );
    sprintf ( otherData, "3: %6s %3i.%iml", time, mlPump, mlrestpump );
    myGLCD.print ( otherData, 5, 153 );
  }
  if ( pump4On == 0 )                                 //Feeding Time 4 Button
  {
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRoundRect ( 165, 168, 315, 188 );
    setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
    myGLCD.print ( "Dosing Pump 4", 184, 172 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    myGLCD.print ( "Dosing Pump 4", 172, 133 );
    myGLCD.print ( "is not scheduled", 184, 146 );
  }
  else {
    myGLCD.setColor ( 0, 255, 0 );
    myGLCD.fillRoundRect ( 165, 168, 315, 188 );
    setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
    myGLCD.print ( "Dosing Pump 4", 184, 172 );
    setFont ( SMALL, 255, 0, 0, 0, 0, 0 );
    timeDispH = pump4_1h;
    timeDispM = pump4_1m;
    buildCorrectTime ();
    mlPump = pump4_1ml / 10;
    mlrestpump = ( pump4_1ml % 10 );
    sprintf ( otherData, "1: %6s %3i.%iml", time, mlPump, mlrestpump );
    myGLCD.print ( otherData, 170, 113 );

    timeDispH = pump4_2h;
    timeDispM = pump4_2m;
    //		timeDispH = 13;
    //		timeDispM = 23;
    buildCorrectTime ();
    mlPump = pump4_2ml / 10;
    mlrestpump = ( pump4_2ml % 10 );
    sprintf ( otherData, "2: %6s %3i.%iml", time, mlPump, mlrestpump );
    myGLCD.print ( otherData, 170, 132 );

    timeDispH = pump4_3h;
    timeDispM = pump4_3m;
    //		timeDispH = 13;
    //		timeDispM = 23;
    buildCorrectTime ();
    mlPump = pump4_3ml / 10;
    mlrestpump = ( pump4_3ml % 10 );
    sprintf ( otherData, "3: %6s %3i.%iml", time, mlPump, mlrestpump );
    myGLCD.print ( otherData, 170, 153 );
  }

  myGLCD.setColor ( 255, 255, 255 );
  for ( int x = 0; x < 2; x++ ) {
    for ( int y = 0; y < 2; y++ ) {
      myGLCD.drawRoundRect ( ( x * 160 ) + 5, ( y * 148 ) + 20, ( x * 160 ) + 155, ( y * 148 ) + 40 );
    }
  }
}

/************************************Dosing Pump Pump settings*************************/

void dosingPumpSubmenu () {
  //	Serial.println ( "DosingSubMenu triggerd" );
  //	Serial.print ( "Pumpe: " );
  //	Serial.println ( dosingPumpSelected );
  //	Serial.print ( "Pumpe4_1h: " );
  //	Serial.println ( pump4_1h );

  char header [18];
  sprintf ( header, "Set Dosing Pump %i", dosingPumpSelected );
  printHeader ( header );

  myGLCD.setColor ( 64, 64, 64 );
  printButton ( "Calibrate Dosing Pump", calibrateDosingPump [0], calibrateDosingPump [1], calibrateDosingPump [2], calibrateDosingPump [3], SMALL );

  myGLCD.drawRect ( 0, 196, 319, 194 );
  printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
  printButton ( "SAVE", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );
  printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

  int mlPump, mlrestpump;
  //			int time1h, time2h, time3h, time1m, time2m, time3m;

  timeDispH = ( * ( pumph [dosingPumpSelected - 1] [0] ) );
  timeDispM = ( * ( pumpm [dosingPumpSelected - 1] [0] ) );
  buildCorrectTime ();
  mlPump = ( * ( pumpml [dosingPumpSelected - 1] [0] ) ) / 10;
  mlrestpump = ( ( * ( pumpml [dosingPumpSelected - 1] [0] ) ) % 10 );
  printButton ( "+", dosingHour1up [0], dosingHour1up [1], dosingHour1up [2], dosingHour1up [3] );
  printButton ( "-", dosingHour1down [0], dosingHour1down [1], dosingHour1down [2], dosingHour1down [3] );
  setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
  myGLCD.print ( "1:", 5, 64 );
  myGLCD.print ( time, 60, 64 );
  sprintf ( otherData, "%3i.%iml", mlPump, mlrestpump );
  myGLCD.print ( otherData, 165, 64 );
  printButton ( "+", dosingMinute1up [0], dosingMinute1up [1], dosingMinute1up [2], dosingMinute1up [3] );
  printButton ( "-", dosingMinute1down [0], dosingMinute1down [1], dosingMinute1down [2], dosingMinute1down [3] );

  printButton ( "+", dosingML1up [0], dosingML1up [1], dosingML1up [2], dosingML1up [3] );
  printButton ( "-", dosingML1down [0], dosingML1down [1], dosingML1down [2], dosingML1down [3] );

  printButton ( "+", dosingSubML1up [0], dosingSubML1up [1], dosingSubML1up [2], dosingSubML1up [3] );
  printButton ( "-", dosingSubML1down [0], dosingSubML1down [1], dosingSubML1down [2], dosingSubML1down [3] );

  printButton ( "RESET", dosingPump1Off [0], dosingPump1Off [1], dosingPump1Off [2], dosingPump1Off [3] );

  //						myGLCD.print ( otherData, 5, 44 );
  // minus 105 - 130
  // plus 135 - 160
  timeDispH = ( * ( pumph [dosingPumpSelected - 1] [1] ) );
  timeDispM = ( * ( pumpm [dosingPumpSelected - 1] [1] ) );
  buildCorrectTime ();
  mlPump = ( * ( pumpml [dosingPumpSelected - 1] [1] ) ) / 10;
  mlrestpump = ( ( * ( pumpml [dosingPumpSelected - 1] [1] ) ) % 10 );
  printButton ( "+", dosingHour2up [0], dosingHour2up [1], dosingHour2up [2], dosingHour2up [3] );
  printButton ( "-", dosingHour2down [0], dosingHour2down [1], dosingHour2down [2], dosingHour2down [3] );
  setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
  myGLCD.print ( "2:", 5, 119 );
  myGLCD.print ( time, 60, 119 );
  sprintf ( otherData, "%3i.%iml", mlPump, mlrestpump );
  myGLCD.print ( otherData, 165, 119 );
  printButton ( "+", dosingMinute2up [0], dosingMinute2up [1], dosingMinute2up [2], dosingMinute2up [3] );
  printButton ( "-", dosingMinute2down [0], dosingMinute2down [1], dosingMinute2down [2], dosingMinute2down [3] );

  printButton ( "+", dosingML2up [0], dosingML2up [1], dosingML2up [2], dosingML2up [3] );
  printButton ( "-", dosingML2down [0], dosingML2down [1], dosingML2down [2], dosingML2down [3] );

  printButton ( "+", dosingSubML2up [0], dosingSubML2up [1], dosingSubML2up [2], dosingSubML2up [3] );
  printButton ( "-", dosingSubML2down [0], dosingSubML2down [1], dosingSubML2down [2], dosingSubML2down [3] );

  printButton ( "RESET", dosingPump2Off [0], dosingPump2Off [1], dosingPump2Off [2], dosingPump2Off [3] );
  //		Serial.print ( "Pump: " );
  //		Serial.print ( dosingPumpSelected );
  //		Serial.print ( " showing: " );
  //		Serial.println ( pump1_3h );
  timeDispH = ( * ( pumph [dosingPumpSelected - 1] [2] ) );
  timeDispM = ( * ( pumpm [dosingPumpSelected - 1] [2] ) );
  buildCorrectTime ();
  mlPump = ( * ( pumpml [dosingPumpSelected - 1] [2] ) ) / 10;
  mlrestpump = ( ( * ( pumpml [dosingPumpSelected - 1] [2] ) ) % 10 );
  printButton ( "+", dosingHour3up [0], dosingHour3up [1], dosingHour3up [2], dosingHour3up [3] );
  printButton ( "-", dosingHour3down [0], dosingHour3down [1], dosingHour3down [2], dosingHour3down [3] );
  setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
  myGLCD.print ( "3:", 5, 160 );
  myGLCD.print ( time, 60, 160 );
  sprintf ( otherData, "%3i.%iml", mlPump, mlrestpump );
  myGLCD.print ( otherData, 165, 160 );
  printButton ( "+", dosingMinute3up [0], dosingMinute3up [1], dosingMinute3up [2], dosingMinute3up [3] );
  printButton ( "-", dosingMinute3down [0], dosingMinute3down [1], dosingMinute3down [2], dosingMinute3down [3] );

  printButton ( "+", dosingML3up [0], dosingML3up [1], dosingML3up [2], dosingML3up [3] );
  printButton ( "-", dosingML3down [0], dosingML3down [1], dosingML3down [2], dosingML3down [3] );

  printButton ( "+", dosingSubML3up [0], dosingSubML3up [1], dosingSubML3up [2], dosingSubML3up [3] );
  printButton ( "-", dosingSubML3down [0], dosingSubML3down [1], dosingSubML3down [2], dosingSubML3down [3] );

  printButton ( "RESET", dosingPump3Off [0], dosingPump3Off [1], dosingPump3Off [2], dosingPump3Off [3] );

  // platz bis 220 etwa,
  // 6 x buttonleiste: 150; 3 x text: 60
}

void dosingPumpCalibration ( boolean updateAll = true ) {
  char header [24];
  int mlPump, mlrestpump;
  if ( updateAll ) {
    sprintf ( header, "Calibrate Dosing Pump %i", dosingPumpSelected );
    printHeader ( header );
  }

  if ( dosingPumpCalibrationActive ) {
    printButton ( "STOP", dosingCalibrationStart [0], dosingCalibrationStart [1], dosingCalibrationStart [2], dosingCalibrationStart [3], SMALL,
    BUTTONCOLOR_RED );

  }
  else {
    myGLCD.setColor ( 64, 64, 64 );
    printButton ( "START", dosingCalibrationStart [0], dosingCalibrationStart [1], dosingCalibrationStart [2], dosingCalibrationStart [3], SMALL );
  }

  if ( updateAll ) {
    myGLCD.drawRect ( 0, 196, 319, 194 );
    printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
    printButton ( "SAVE", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );
    printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

    setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
    myGLCD.print ( "Press START to start calibration.", 5, 65 );
    myGLCD.print ( "Enter pumped Vol (10 sec.) below.", 5, 77 );

    //		Serial.println ( "Dosing pump calibrate - writing ml: " );
    //		Serial.print ( "origValue is: " );
    Serial.println ( * ( pumpCalibrate [dosingPumpSelected - 1] ) );

    mlPump = ( * ( pumpCalibrate [dosingPumpSelected - 1] ) ) / 10;
    mlrestpump = ( ( * ( pumpCalibrate [dosingPumpSelected - 1] ) ) % 10 );

    //		Serial.print ( "ml Value is: " );
    //				Serial.println (mlPump );
    //
    //		Serial.print ( "subml Value is: " );
    //				Serial.println (mlrestpump );

    sprintf ( otherData, "%3i.%iml", mlPump, mlrestpump );
    myGLCD.print ( otherData, 165, 160 );
    printButton ( "+", dosingCalibrationMLup [0], dosingCalibrationMLup [1], dosingCalibrationMLup [2], dosingCalibrationMLup [3], SMALL );
    printButton ( "-", dosingCalibrationMLdown [0], dosingCalibrationMLdown [1], dosingCalibrationMLdown [2], dosingCalibrationMLdown [3], SMALL );

    printButton ( "+", dosingCalibrationSubMLup [0], dosingCalibrationSubMLup [1], dosingCalibrationSubMLup [2], dosingCalibrationSubMLup [3], SMALL );
    printButton ( "-", dosingCalibrationSubMLdown [0], dosingCalibrationSubMLdown [1], dosingCalibrationSubMLdown [2], dosingCalibrationSubMLdown [3],
    SMALL );
  }
}

void checkDosingPumpOnOff () {

  for ( int i = 0; i < 4; i++ ) {
    if ( dosingPumpOffTimes [i] > millis () ) {
      if (!pumpRunning[i]){
        // dosing pump should be on
        digitalWrite ( *pumpPins [i], HIGH );
        
        Serial.print(i+1);
        Serial.print(" Pump on: ");
        Serial.println(millis());
        pumpRunning[i] = true;
      }
    }
    else {
      if (pumpRunning[i]){
        digitalWrite ( *pumpPins [i], LOW );
        dosingPumpCalibrationActive = false;
       
        Serial.print(i+1);
        Serial.print(" Pump off: ");
        Serial.println(millis());
         pumpRunning[i] = false;
      }
    }
  }

  if ( !dosingPumpSettingsChanged ) {
    //check programmed times
    for ( int i = 0; i < 4; i++ ) { // pump
      for ( int j = 0; j < 3; j++ ) { // program
        // pump active?
        if ( pumpml [i] [j] > 0 ) {
          //does hour fit?
          if ( ( * ( pumph [i] [j] ) == rtc [2] ) && ( * ( pumpm [i] [j] ) == rtc [1] ) && ( rtc [0] <= 3 )
            && ( dosingPumpOffTimes [i] < millis () && !pumpRunning [i] ) ) {
            //activate pump for dosing of j ml
            // first: pumpml says 172 for pumping 17.2 ml. pumpcalibration says 53 for 5.3 ml per second.
            // to get time in milis: pumpml *1000L / pumpcal = time in ms
            Serial.print("Timer triggered for Pump ");
            Serial.print(i+1);
            Serial.print("-");
            Serial.print(j);
            Serial.print("; should be on for ms ");
            long pumptime = ( (long) (*(pumpml [i] [j])) * 1000L ) / ( *pumpCalibrate [i] ) * testtime;
            Serial.println(pumptime);
            Serial.print(millis());
            Serial.print(" -> ");

            dosingPumpOffTimes [i] = millis () + pumptime;
            Serial.println(dosingPumpOffTimes [i]);
          }
        }

      }
    }
  }
}

/************************************ TOUCH SCREEN ************************************/
void processMyTouch () {
  myTouch.read ();
  x = myTouch.getX ();
  y = myTouch.getY ();

  returnTimer = 0;
  screenSaverTimer = 0;

  if ( ( x >= canC [0] ) && ( x <= canC [2] ) && ( y >= canC [1] ) && ( y <= canC [3] )  //press cancel
  && ( dispScreen != 0 ) && ( dispScreen != 5 ) && ( dispScreen != 6 ) && ( dispScreen != 8 ) && ( dispScreen != 11 ) ) {
    waitForIt ( canC [0], canC [1], canC [2], canC [3] );
    LEDtestTick = false;
    waveMakerOff = false;
    dosingPumpCalibrationActive = false;

    dosingPumpOffTimes [dosingPumpSelected] = 0;

    dosingPumpSettingsChanged = false;
    ReadFromEEPROM ();
    dispScreen = 0;
    clearScreen ();
    mainScreen ( true );
  }
  else if ( ( x >= back [0] ) && ( x <= back [2] ) && ( y >= back [1] )
    && ( y <= back [3] )  //press back
  && ( dispScreen != 0 ) && ( dispScreen != 1 ) && ( dispScreen != 5 ) && ( dispScreen != 6 ) && ( dispScreen != 8 ) && ( dispScreen != 11 )
    && ( dispScreen != 14 ) && ( dispScreen != 18 ) && ( dispScreen != 19 ) ) {
    waitForIt ( back [0], back [1], back [2], back [3] );
    LEDtestTick = false;
    waveMakerOff = false;
    dosingPumpSettingsChanged = false;
    ReadFromEEPROM ();
    if ( dispScreen == DOSINGPUMP_MENU ) {
      dispScreen = MENUSCREEN_TWO;
      clearScreen ();
      menuScreen2 ();

    }
    else {
      dispScreen = MENUSCREEN_ONE;
      clearScreen ();
      menuScreen ();
    }
  }
  else {
    switch ( dispScreen ) {
    case 0 :     //--------------- MAIN SCREEN (Press Any Key) ---------------
      dispScreen = MENUSCREEN_ONE;
      clearScreen ();
      menuScreen ();
      break;

    case 1 :     //--------------------- MENU SCREEN -------------------------
      if ( ( x >= prSAVE [0] ) && ( x <= prSAVE [2] ) && ( y >= prSAVE [1] ) && ( y <= prSAVE [3] ) )  //press SAVE / NEXT
      {
        waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
        dispScreen = MENUSCREEN_TWO;
        clearScreen ();
        menuScreen2 ();
      }
      if ( ( x >= tanD [0] ) && ( x <= tanD [2] ) )                      //first column
      {
        if ( ( y >= tanD [1] ) && ( y <= tanD [3] ) )                   //press Date & Clock Screen
        {
          waitForIt ( tanD [0], tanD [1], tanD [2], tanD [3] );
          if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) {
            AM_PM = 1;
          }
          else {
            AM_PM = 2;
          }
          dispScreen = 2;
          clearScreen ();
          clockScreen ();
        }
        if ( ( y >= temC [1] ) && ( y <= temC [3] ) )                   //press H2O Temp Control
        {
          waitForIt ( temC [0], temC [1], temC [2], temC [3] );
          ReadFromEEPROM ();
          dispScreen = 3;
          clearScreen ();
          tempScreen ( true );
        }
        if ( ( y >= wave [1] ) && ( y <= wave [3] ) )                   //press Wavemaker Screen
        {
          waitForIt ( wave [0], wave [1], wave [2], wave [3] );
          dispScreen = 10;
          clearScreen ();
          WaveMakerScreen ();
          WaveMakerStatusScreen ();
        }
        if ( ( y >= gSet [1] ) && ( y <= gSet [3] ) )                   //press General Settings
        {
          waitForIt ( gSet [0], gSet [1], gSet [2], gSet [3] );
          dispScreen = 12;
          clearScreen ();
          generalSettingsScreen ();
        }
      }
      if ( ( x >= tesT [0] ) && ( x <= tesT [2] ) )                      //second column
      {
        if ( ( y >= tesT [1] ) && ( y <= tesT [3] ) )                  //press LED Testing page
        {
          waitForIt ( tesT [0], tesT [1], tesT [2], tesT [3] );
          dispScreen = 4;
          clearScreen ();
          ledTestOptionsScreen ();
        }
        if ( ( y >= ledChM [1] ) && ( y <= ledChM [3] ) )             //press Change LED values
        {
          waitForIt ( ledChM [0], ledChM [1], ledChM [2], ledChM [3] );
          dispScreen = 7;
          clearScreen ();
          ledColorViewScreen ();
        }
        if ( ( y >= aFeed [1] ) && ( y <= aFeed [3] ) )               //press Automatic Feeder screen
        {
          waitForIt ( aFeed [0], aFeed [1], aFeed [2], aFeed [3] );
          dispScreen = 13;
          clearScreen ();
          autoFeederScreen ();
        }
        if ( ( y >= about [1] ) && ( y <= about [3] ) )               //press About sketch
        {
          waitForIt ( about [0], about [1], about [2], about [3] );
          dispScreen = 15;
          clearScreen ();
          AboutScreen ();
        }
      }

      break;

    case 2 :     //--------------- CLOCK & DATE SETUP SCREEN -----------------
      if ( ( x >= prSAVE [0] ) && ( x <= prSAVE [2] ) && ( y >= prSAVE [1] ) && ( y <= prSAVE [3] ) )  //press SAVE
      {
        waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
        if ( setTimeFormat == 1 ) {
          if ( ( rtcSet [2] == 0 ) && ( AM_PM == 2 ) ) {
            rtcSet [2] += 12;
          }
          if ( ( ( rtcSet [2] >= 1 ) && ( rtcSet [2] <= 11 ) ) && ( AM_PM == 2 ) ) {
            rtcSet [2] += 12;
          }
          if ( ( ( rtcSet [2] >= 12 ) && ( rtcSet [2] <= 23 ) ) && ( AM_PM == 1 ) ) {
            rtcSet [2] -= 12;
          }
        }
        SaveRTC ();
        dispScreen = 0;
        clearScreen ();
        mainScreen ( true );
      }
      else {
        if ( ( y >= houU [1] ) && ( y <= houU [3] ) )                    //FIRST ROW
        {
          if ( ( x >= houU [0] ) && ( x <= houU [2] ) )                 //press hour up
          {
            waitForIt ( houU [0], houU [1], houU [2], houU [3] );
            rtcSet [2]++;
            if ( rtcSet [2] >= 24 ) {
              rtcSet [2] = 0;
            }
          }
          if ( ( x >= minU [0] ) && ( x <= minU [2] ) )                 //press min up
          {
            waitForIt ( minU [0], minU [1], minU [2], minU [3] );
            rtcSet [1]++;
            if ( rtcSet [1] >= 60 ) {
              rtcSet [1] = 0;
            }
          }
          if ( ( x >= ampmU [0] ) && ( x <= ampmU [2] ) )               //press AMPM up
          {
            waitForIt ( ampmU [0], ampmU [1], ampmU [2], ampmU [3] );
            if ( AM_PM == 1 ) {
              AM_PM = 2;
            }
            else {
              AM_PM = 1;
            }
          }
        }
        if ( ( y >= houD [1] ) && ( y <= houD [3] ) )                    //SECOND ROW
        {
          if ( ( x >= houD [0] ) && ( x <= houD [2] ) )                 //press hour down
          {
            waitForIt ( houD [0], houD [1], houD [2], houD [3] );
            rtcSet [2]--;
            if ( rtcSet [2] < 0 ) {
              rtcSet [2] = 23;
            }
          }
          if ( ( x >= minD [0] ) && ( x <= minD [2] ) )                 //press min down
          {
            waitForIt ( minD [0], minD [1], minD [2], minD [3] );
            rtcSet [1]--;
            if ( rtcSet [1] < 0 ) {
              rtcSet [1] = 59;
            }
          }
          if ( ( x >= ampmD [0] ) && ( x <= ampmD [2] ) )               //press AMPM down
          {
            waitForIt ( ampmD [0], ampmD [1], ampmD [2], ampmD [3] );
            if ( AM_PM == 1 ) {
              AM_PM = 2;
            }
            else {
              AM_PM = 1;
            }
          }
        }
        if ( ( y >= dayU [1] ) && ( y <= dayU [3] ) )                    //THIRD ROW
        {
          if ( setCalendarFormat == 0 )                         //DD/MM/YYYY Format
          {
            if ( ( x >= dayU [0] ) && ( x <= dayU [2] ) )              //press day up
            {
              waitForIt ( dayU [0], dayU [1], dayU [2], dayU [3] );
              rtcSet [4]++;
              if ( rtcSet [4] > 31 ) {
                rtcSet [4] = 1;
              }
            }
            if ( ( x >= monU [0] ) && ( x <= monU [2] ) )              //press month up
            {
              waitForIt ( monU [0], monU [1], monU [2], monU [3] );
              rtcSet [5]++;
              if ( rtcSet [5] > 12 ) {
                rtcSet [5] = 1;
              }
            }
          }
          else {
            if ( setCalendarFormat == 1 )                         //MM/DD/YYYY Format
            {
              if ( ( x >= dayU [0] ) && ( x <= dayU [2] ) )              //press month up
              {
                waitForIt ( dayU [0], dayU [1], dayU [2], dayU [3] );
                rtcSet [5]++;
                if ( rtcSet [5] > 12 ) {
                  rtcSet [5] = 1;
                }
              }
              if ( ( x >= monU [0] ) && ( x <= monU [2] ) )              //press day up
              {
                waitForIt ( monU [0], monU [1], monU [2], monU [3] );
                rtcSet [4]++;
                if ( rtcSet [4] > 31 ) {
                  rtcSet [4] = 1;
                }
              }
            }
          }
          if ( ( x >= yeaU [0] ) && ( x <= yeaU [2] ) )                 //press year up
          {
            waitForIt ( yeaU [0], yeaU [1], yeaU [2], yeaU [3] );
            rtcSet [6]++;
            if ( rtcSet [6] > 2100 ) {
              rtcSet [6] = 2000;
            }
          }
        }
        if ( ( y >= dayD [1] ) && ( y <= dayD [3] ) )                    //FOURTH ROW
        {
          if ( setCalendarFormat == 0 )                         //DD/MM/YYYY Format
          {
            if ( ( x >= dayD [0] ) && ( x <= dayD [2] ) )              //press day down
            {
              waitForIt ( dayD [0], dayD [1], dayD [2], dayD [3] );
              rtcSet [4]--;
              if ( rtcSet [4] < 1 ) {
                rtcSet [4] = 31;
              }
            }
            if ( ( x >= monD [0] ) && ( x <= monD [2] ) )              //press month down
            {
              waitForIt ( monD [0], monD [1], monD [2], monD [3] );
              rtcSet [5]--;
              if ( rtcSet [5] < 1 ) {
                rtcSet [5] = 12;
              }
            }
          }
          else {
            if ( setCalendarFormat == 1 )                         //MM/DD/YYYY Format
            {
              if ( ( x >= dayD [0] ) && ( x <= dayD [2] ) )              //press month down
              {
                waitForIt ( dayD [0], dayD [1], dayD [2], dayD [3] );
                rtcSet [5]--;
                if ( rtcSet [5] < 1 ) {
                  rtcSet [5] = 12;
                }
              }
              if ( ( x >= monD [0] ) && ( x <= monD [2] ) )              //press day down
              {
                waitForIt ( monD [0], monD [1], monD [2], monD [3] );
                rtcSet [4]--;
                if ( rtcSet [4] < 1 ) {
                  rtcSet [4] = 31;
                }
              }
            }
          }
          if ( ( x >= yeaD [0] ) && ( x <= yeaD [2] ) )                 //press year down
          {
            waitForIt ( yeaD [0], yeaD [1], yeaD [2], yeaD [3] );
            rtcSet [6]--;
            if ( rtcSet [6] < 2000 ) {
              rtcSet [6] = 2100;
            }
          }
        }
        clockScreen ( false );
      }
      break;

    case 3 :     //------------------ H20 TEMPERATURE CONTROL ---------------
      if ( ( x >= prSAVE [0] ) && ( x <= prSAVE [2] ) && ( y >= prSAVE [1] ) && ( y <= prSAVE [3] ) )  //press SAVE
      {
        waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
        setTempC = temp2beS;
        setTempF = temp2beS;
        offTempC = temp2beO;
        offTempF = temp2beO;
        alarmTempC = temp2beA;
        alarmTempF = temp2beA;
        if ( setTempScale == 0 )                      //Celsius to Farenheit (Consistency Conversion)
        {
          setTempF = ( ( 1.8 * setTempC ) + 32.05 );
          offTempF = ( ( 1.8 * offTempC ) + 0.05 );
          alarmTempF = ( ( 1.8 * alarmTempC ) + 0.05 );
        }
        if ( setTempScale == 1 )                      //Farenheit to Celsius (Consistency Conversion)
        {
          setTempC = ( ( .55556 * ( setTempF - 32 ) ) + .05 );
          offTempC = ( .55556 ) * offTempF + .05;
          alarmTempC = ( .55556 ) * alarmTempF + .05;
        }
        dispScreen = 0;
        SaveTempToEEPROM ();
        clearScreen ();
        mainScreen ( true );
      }
      else
        setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
      {
        if ( ( x >= temM [0] ) && ( x <= temM [2] ) )                         //first column
        {
          if ( ( y >= temM [1] ) && ( y <= temM [3] ) )                      //press temp minus
          {
            waitForIt ( temM [0], temM [1], temM [2], temM [3] );
            temp2beS -= 0.1;
            if ( ( setTempScale == 1 ) && ( temp2beS <= 50 ) ) {
              temp2beS = 50;
            }
            if ( ( setTempScale == 0 ) && ( temp2beS <= 10 ) ) {
              temp2beS = 10;
            }
            tempScreen ();
          }
          if ( ( y >= offM [1] ) && ( y <= offM [3] ) )                      //press offset minus
          {
            waitForIt ( offM [0], offM [1], offM [2], offM [3] );
            temp2beO -= 0.1;
            if ( temp2beO < 0.1 ) {
              temp2beO = 0.0;
            }
            tempScreen ();
          }
          if ( ( y >= almM [1] ) && ( y <= almM [3] ) )                      //press alarm minus
          {
            waitForIt ( almM [0], almM [1], almM [2], almM [3] );
            temp2beA -= 0.1;
            if ( temp2beA < 0.1 ) {
              temp2beA = 0.0;
            }
            tempScreen ();
          }
        }
        if ( ( x >= temP [0] ) && ( x <= temP [2] ) )                         //second column
        {
          if ( ( y >= temP [1] ) && ( y <= temP [3] ) )                      //press temp plus
          {
            waitForIt ( temP [0], temP [1], temP [2], temP [3] );
            temp2beS += 0.1;
            if ( ( setTempScale == 1 ) && ( temp2beS >= 104 ) ) {
              temp2beS = 104;
            }
            if ( ( setTempScale == 0 ) && ( temp2beS >= 40 ) ) {
              temp2beS = 40;
            }
            tempScreen ();
          }
          if ( ( y >= offP [1] ) && ( y <= offP [3] ) )                      //press offset plus
          {
            waitForIt ( offP [0], offP [1], offP [2], offP [3] );
            temp2beO += 0.1;
            if ( temp2beO >= 10 ) {
              temp2beO = 9.9;
            }
            tempScreen ();
          }
          if ( ( y >= almP [1] ) && ( y <= almP [3] ) )                      //press alarm plus
          {
            waitForIt ( almP [0], almP [1], almP [2], almP [3] );
            temp2beA += 0.1;
            if ( temp2beA >= 10 ) {
              temp2beA = 9.9;
            }
            tempScreen ();
          }
        }
      }
      break;

    case 4 :     // -------------- LED TEST OPTIONS SCREEN -----------------
      if ( ( x >= tstLA [0] ) && ( x <= tstLA [2] ) && ( y >= tstLA [1] ) && ( y <= tstLA [3] ) )   //Test LED Array Output
      {
        waitForIt ( tstLA [0], tstLA [1], tstLA [2], tstLA [3] );
        dispScreen = 5;
        clearScreen ();
        testArrayScreen ( true );
      }
      if ( ( x >= cntIL [0] ) && ( x <= cntIL [2] ) && ( y >= cntIL [1] ) && ( y <= cntIL [3] ) )   //Test Individual LEDs
      {
        waitForIt ( cntIL [0], cntIL [1], cntIL [2], cntIL [3] );
        dispScreen = 6;
        clearScreen ();
        testIndLedScreen ();
        colorLEDtest = true;
      }
      break;

    case 5 :     //---------------- TEST LED ARRAY SCREEN ------------------
      if ( ( x >= back [0] ) && ( x <= back [2] ) && ( y >= back [1] ) && ( y <= back [3] )   //press back
      && ( LEDtestTick == false ) ) {
        waitForIt ( back [0], back [1], back [2], back [3] );
        LEDtestTick = false;
        ReadFromEEPROM ();
        dispScreen = 4;
        clearScreen ();
        ledTestOptionsScreen ();
      }
      if ( ( x >= canC [0] ) && ( x <= canC [2] ) && ( y >= canC [1] ) && ( y <= canC [3] )   //press CANCEL
      && ( LEDtestTick == false ) ) {
        waitForIt ( canC [0], canC [1], canC [2], canC [3] );
        LEDtestTick = false;
        ReadFromEEPROM ();
        dispScreen = 0;
        clearScreen ();
        mainScreen ( true );
      }
      if ( ( x >= stsT [0] ) && ( x <= stsT [2] ) && ( y >= stsT [1] ) && ( y <= stsT [3] ) )  //press start/stop test
      {
        waitForIt ( stsT [0], stsT [1], stsT [2], stsT [3] );

        if ( LEDtestTick ) {
          LEDtestTick = false;
          testArrayScreen ( true );
        }
        else {
          LEDtestTick = true;
          testArrayScreen ();
        }
      }
      else {
        if ( ( x >= tenM [0] ) && ( x <= tenM [2] ) && ( y >= tenM [1] ) && ( y <= tenM [3] ) )   //press -10s
        {
          min_cnt -= 10;
          if ( min_cnt < 0 ) {
            min_cnt = 0;
          }
          delay ( 50 );
        }
        if ( ( x >= tenP [0] ) && ( x <= tenP [2] ) && ( y >= tenP [1] ) && ( y <= tenP [2] ) )   //press +10s
        {
          min_cnt += 10;
          if ( min_cnt > 1440 ) {
            min_cnt = 1440;
          }
          delay ( 50 );
        }
      }
      break;

    case 6 :     // --------------- TEST INDIVIDUAL LED SCREEN --------------
      int CL_check, CL_check2;
      if ( ( x >= 215 ) && ( x <= 315 ) && ( y >= 200 ) && ( y <= 220 ) )          //press back
      {
        waitForIt ( 215, 200, 315, 220 );
        LEDtestTick = false;
        dispScreen = 4;
        clearScreen ();
        ledTestOptionsScreen ();
        colorLEDtest = false;
      }
      if ( ( x >= 5 ) && ( x <= 105 ) )                                    //First Column of Colors
      {
        if ( ( y >= 148 ) && ( y <= 168 ) )                               //Press SUMP
        {
          waitForIt ( 5, 148, 105, 168 );
          COLOR = 0;
          Rgad = 0, Ggad = 150, Bgad = 0, Rfont = 255, Gfont = 255, Bfont = 255, Rback = 0, Gback = 150, Bback = 0, Rline = 255, Gline = 255, Bline =
            255;
          xValue = 67, yValue = 152, x1Bar = 40, x2Bar = x1Bar + 12;
          COLOR = SUMP;
          ledChangerGadget ();
        }
        if ( ( y >= 174 ) && ( y <= 194 ) )                               //press Red
        {
          waitForIt ( 5, 174, 105, 194 );
          COLOR = 0;
          Rgad = 255, Ggad = 0, Bgad = 0, Rfont = 255, Gfont = 255, Bfont = 255, Rback = 255, Gback = 0, Bback = 0, Rline = 255, Gline = 255, Bline =
            255;
          xValue = 67, yValue = 178, x1Bar = 57, x2Bar = x1Bar + 12;
          COLOR = RED;
          ledChangerGadget ();
        }
        if ( ( y >= 200 ) && ( y <= 220 ) )                               //press White
        {
          waitForIt ( 5, 200, 105, 220 );
          COLOR = 0;
          Rgad = 255, Ggad = 255, Bgad = 255, Rfont = 0, Gfont = 0, Bfont = 0, Rback = 255, Gback = 255, Bback = 255, Rline = 0, Gline = 0, Bline =
            0;
          xValue = 67, yValue = 204, x1Bar = 74, x2Bar = x1Bar + 12;
          COLOR = WHITE;
          ledChangerGadget ();
        }
      }

      if ( ( x >= 110 ) && ( x <= 210 ) )                                  //Second Column of Colors
      {
        if ( ( y >= 148 ) && ( y <= 168 ) )                               //Press Blue
        {
          waitForIt ( 110, 148, 210, 168 );
          COLOR = 0;
          Rgad = 9, Ggad = 184, Bgad = 255, Rfont = 255, Gfont = 255, Bfont = 255, Rback = 9, Gback = 184, Bback = 255, Rline = 255, Gline = 255, Bline =
            255;
          xValue = 172, yValue = 152, x1Bar = 91, x2Bar = x1Bar + 12;
          COLOR = BLUE;
          ledChangerGadget ();
        }
        if ( ( y >= 174 ) && ( y <= 194 ) )                               //press Royal Blue
        {
          waitForIt ( 110, 174, 210, 194 );
          COLOR = 0;
          Rgad = 58, Ggad = 95, Bgad = 205, Rfont = 255, Gfont = 255, Bfont = 255, Rback = 58, Gback = 95, Bback = 205, Rline = 255, Gline = 255, Bline =
            255;
          xValue = 172, yValue = 178, x1Bar = 108, x2Bar = x1Bar + 12;
          COLOR = ROYAL;
          ledChangerGadget ();
        }
        if ( ( y >= 200 ) && ( y <= 220 ) )                               //press Ultraviolet
        {
          waitForIt ( 110, 200, 210, 220 );
          COLOR = 0;
          Rgad = 224, Ggad = 102, Bgad = 255, Rfont = 255, Gfont = 255, Bfont = 255, Rback = 224, Gback = 102, Bback = 255, Rline = 255, Gline =
            255, Bline = 255;
          xValue = 172, yValue = 204, x1Bar = 125, x2Bar = x1Bar + 12;
          COLOR = ULTRA;
          ledChangerGadget ();
        }
      }

      if ( ( x >= 215 ) && ( x <= 315 ) && ( y >= 148 ) && ( y <= 168 ) )          //Press Moon
      {
        waitForIt ( 215, 148, 315, 168 );
        COLOR = 0;
        Rgad = 176, Ggad = 176, Bgad = 176, Rfont = 0, Gfont = 0, Bfont = 0, Rback = 176, Gback = 176, Bback = 176, Rline = 0, Gline = 0, Bline = 0;
        xValue = 277, yValue = 152, x1Bar = 142, x2Bar = x1Bar + 12;
        COLOR = MOON;
        ledChangerGadget ();
      }

      if ( ( y >= 78 ) && ( y <= 100 ) )                                   //Plus buttons were touched
      {
        for ( int i = 0; i < 3; i++ ) {
          if ( ( x >= ( i * 27 ) + 204 ) && ( x <= ( i * 27 ) + 226 ) && ( COLOR != 0 ) ) {
            waitForItSq ( ( i * 27 ) + 204, 78, ( i * 27 ) + 226, 100 );
            if ( i == 0 ) {
              cl_100 += 1;
            }
            if ( i == 1 ) {
              cl_10 += 1;
            }
            if ( i == 2 ) {
              cl_1 += 1;
            }
            CL_check = ( cl_10 * 10 ) + cl_1;
            if ( ( cl_100 > 2 ) && ( i == 0 ) ) {
              cl_100 = 0;
            }
            if ( ( cl_100 < 2 ) && ( cl_10 > 9 ) && ( i == 1 ) ) {
              cl_10 = 0;
            }
            if ( ( cl_100 >= 2 ) && ( cl_10 >= 5 ) && ( i == 1 ) ) {
              cl_10 = 5;
            }
            if ( ( cl_100 < 2 ) && ( cl_1 > 9 ) && ( i == 2 ) ) {
              cl_1 = 0;
            }
            if ( ( cl_100 >= 2 ) && ( cl_10 < 5 ) && ( cl_1 > 9 ) && ( i == 2 ) ) {
              cl_1 = 0;
            }
            if ( ( cl_100 >= 2 ) && ( cl_10 >= 5 ) && ( i == 2 ) ) {
              cl_1 = 5;
            }
            if ( ( CL_check >= 55 ) && ( ( i == 0 ) || ( i == 1 ) ) && ( cl_100 > 1 ) ) {
              cl_10 = 5;
              cl_1 = 5;
              setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
              myGLCD.printNumI ( cl_10, 234, 54 );
              myGLCD.printNumI ( cl_1, 255, 54 );
            }
            setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
            if ( i == 0 ) {
              myGLCD.printNumI ( cl_100, 214, 54 );
            }
            if ( i == 1 ) {
              myGLCD.printNumI ( cl_10, 234, 54 );
            }
            if ( i == 2 ) {
              myGLCD.printNumI ( cl_1, 255, 54 );
            }
            CL_100 = cl_100 * 100;
            CL_10 = cl_10 * 10;
            CL_1 = cl_1;
            for ( int c = 0; c < 3; c++ ) {
              myGLCD.setColor ( Rline, Gline, Bline );
              myGLCD.drawRect ( ( c * 27 ) + 204, 78, ( c * 27 ) + 226, 100 );
              myGLCD.drawRect ( ( c * 27 ) + 204, 105, ( c * 27 ) + 226, 127 );
            }
            drawBarandColorValue ();
            LED_levels_output ();
            checkTempC ();
          }
        }
      }
      else if ( ( y >= 105 ) && ( y <= 127 ) )                             //Minus buttons were touched
      {
        for ( int i = 0; i < 3; i++ ) {
          if ( ( x >= ( i * 27 ) + 204 ) && ( x <= ( i * 27 ) + 226 ) && ( COLOR != 0 ) ) {
            waitForItSq ( ( i * 27 ) + 204, 105, ( i * 27 ) + 226, 127 );
            if ( i == 0 ) {
              cl_100 -= 1;
            }
            if ( i == 1 ) {
              cl_10 -= 1;
            }
            if ( i == 2 ) {
              cl_1 -= 1;
            }
            CL_check = ( cl_10 * 10 ) + cl_1;
            CL_check2 = ( cl_100 * 100 ) + cl_1;
            if ( ( cl_100 < 0 ) && ( i == 0 ) ) {
              cl_100 = 2;
            }
            if ( ( cl_10 < 0 ) && ( i == 1 ) && ( cl_100 < 2 ) ) {
              cl_10 = 9;
            }
            if ( ( cl_10 < 0 ) && ( i == 1 ) && ( cl_100 == 2 ) ) {
              cl_10 = 5;
            }
            if ( ( cl_1 < 0 ) && ( i == 2 ) && ( cl_100 < 2 ) ) {
              cl_1 = 9;
            }
            if ( ( cl_1 < 0 ) && ( i == 2 ) && ( cl_100 == 2 ) && ( cl_10 < 5 ) ) {
              cl_1 = 9;
            }
            if ( ( cl_1 < 0 ) && ( i == 2 ) && ( cl_100 == 2 ) && ( cl_10 >= 5 ) ) {
              cl_1 = 5;
            }
            if ( ( CL_check >= 55 ) && ( ( i == 0 ) || ( i == 1 ) ) && ( cl_100 > 1 ) ) {
              cl_10 = 5;
              cl_1 = 5;
              setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
              myGLCD.printNumI ( cl_10, 234, 54 );
              myGLCD.printNumI ( cl_1, 255, 54 );
            }
            if ( ( CL_check2 > 205 ) && ( i == 1 ) && ( cl_10 < 1 ) ) {
              cl_10 = 5;
              cl_1 = 5;
              setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
              myGLCD.printNumI ( cl_10, 234, 54 );
              myGLCD.printNumI ( cl_1, 255, 54 );
            }
            setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
            if ( i == 0 ) {
              myGLCD.printNumI ( cl_100, 214, 54 );
            }
            if ( i == 1 ) {
              myGLCD.printNumI ( cl_10, 234, 54 );
            }
            if ( i == 2 ) {
              myGLCD.printNumI ( cl_1, 255, 54 );
            }
            CL_100 = cl_100 * 100;
            CL_10 = cl_10 * 10;
            CL_1 = cl_1;
            for ( int c = 0; c < 3; c++ ) {
              myGLCD.setColor ( Rline, Gline, Bline );
              myGLCD.drawRect ( ( c * 27 ) + 204, 78, ( c * 27 ) + 226, 100 );
              myGLCD.drawRect ( ( c * 27 ) + 204, 105, ( c * 27 ) + 226, 127 );
            }
            drawBarandColorValue ();
            LED_levels_output ();
            checkTempC ();
          }
        }
      }
      break;

    case 7 :     // ----------- VIEW INDIVIDUAL LED COLORS SCREEN -----------
      if ( ( x >= 10 ) && ( x <= 150 ) )                                //first column
      {
        if ( ( y >= 20 ) && ( y <= 50 ) )                              //View White LEDs Array
        {
          waitForIt ( 10, 20, 150, 50 );
          dispScreen = 8;
          COLOR = WHITE;
          clearScreen ();
          ledValuesScreen ();
        }
        if ( ( y >= 60 ) && ( y <= 90 ) )                              //View Royal Blue LEDs Array
        {
          waitForIt ( 10, 60, 150, 90 );
          dispScreen = 8;
          COLOR = ROYAL;
          clearScreen ();
          ledValuesScreen ();
        }
        if ( ( y >= 100 ) && ( y <= 130 ) )                            //View Red LEDs Array
        {
          waitForIt ( 10, 100, 150, 130 );
          dispScreen = 8;
          COLOR = RED;
          clearScreen ();
          ledValuesScreen ();
        }
      }
      if ( ( x >= 170 ) && ( x <= 310 ) )                               //second column
      {
        if ( ( y >= 20 ) && ( y <= 50 ) )                              //View Blue LEDs Array
        {
          waitForIt ( 170, 20, 310, 50 );
          dispScreen = 8;
          COLOR = BLUE;
          clearScreen ();
          ledValuesScreen ();
        }
        if ( ( y >= 60 ) && ( y <= 90 ) )                              //View Ultra LEDs Array
        {
          waitForIt ( 170, 60, 310, 90 );
          dispScreen = 8;
          COLOR = ULTRA;
          clearScreen ();
          ledValuesScreen ();
        }
        if ( ( y >= 100 ) && ( y <= 130 ) )                            //View Sump LEDs Array
        {
          waitForIt ( 170, 100, 310, 130 );
          dispScreen = 8;
          COLOR = SUMP;
          clearScreen ();
          ledValuesScreen ();
        }
      }
      if ( ( x >= 90 ) && ( x <= 230 ) && ( y >= 140 ) && ( y <= 170 ) )        //View Moon LEDs MI
      {
        waitForIt ( 90, 140, 230, 170 );
        ReadFromEEPROM ();
        dispScreen = 8;
        COLOR = MOON;
        clearScreen ();
        ledValuesScreen ();
      }
      break;

    case 8 :     // -------------- SHOW LED VALUES TABLE SCREEN -------------
      if ( ( x >= back [0] ) && ( x <= back [2] ) && ( y >= back [1] ) && ( y <= back [3] ) )   //press MORE COLORS
      {
        waitForIt ( back [0], back [1], back [2], back [3] );
        ReadFromEEPROM ();
        dispScreen = 7;
        clearScreen ();
        ledColorViewScreen ();
      }
      else if ( ( x >= ledChV [0] ) && ( x <= ledChV [2] ) && ( y >= ledChV [1] ) && ( y <= ledChV [3] ) && ( COLOR != 7 ) )   //press CHANGE
      {
        waitForIt ( ledChV [0], ledChV [1], ledChV [2], ledChV [3] );
        ReadFromEEPROM ();
        dispScreen = 9;
        clearScreen ();
        ledChangeScreen ();
      }
      else if ( ( x >= eeprom [0] ) && ( x <= eeprom [2] ) && ( y >= eeprom [1] ) && ( y <= eeprom [3] ) && ( COLOR != 7 ) )  //press SAVE
      {
        waitForIt ( eeprom [0], eeprom [1], eeprom [2], eeprom [3] );
        SaveLEDToEEPROM ();
        dispScreen = 7;
        clearScreen ();
        ledColorViewScreen ();
      }
      else if ( ( x >= prSAVE [0] ) && ( x <= prSAVE [2] ) && ( y >= prSAVE [1] ) && ( y <= prSAVE [3] ) )  //press SAVE
      {
        waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
        SaveMoonLEDToEEPROM ();
        dispScreen = 7;
        clearScreen ();
        ledColorViewScreen ();
      }
      else if ( ( x >= canC [0] ) && ( x <= canC [2] ) && ( y >= canC [1] ) && ( y <= canC [3] ) )         //press CANCEL
      {
        waitForIt ( canC [0], canC [1], canC [2], canC [3] );
        ReadFromEEPROM ();
        LEDtestTick = false;
        dispScreen = 0;
        clearScreen ();
        mainScreen ( true );
      }
      else

          if ( ( x >= miM [0] ) && ( x <= miM [2] ) && ( y >= miM [1] ) && ( y <= miM [3] ) )   //press MI minus
        {
          waitForIt ( miM [0], miM [1], miM [2], miM [3] );
          tMI -= 1;
          if ( tMI <= 0 ) {
            tMI = 0;
          }
          MI = tMI;
          setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
          myGLCD.print ( "   ", CENTER, 120 );
          myGLCD.printNumI ( tMI, CENTER, 120 );
        }
        else if ( ( x >= miP [0] ) && ( x <= miP [2] ) && ( y >= miP [1] ) && ( y <= miP [3] ) )   //press MI plus
        {
          waitForIt ( miP [0], miP [1], miP [2], miP [3] );
          tMI += 5;
          if ( tMI > 255 ) {
            tMI = 255;
          }
          MI = tMI;
          setFont ( LARGE, 255, 255, 255, 0, 0, 0 );
          myGLCD.print ( "   ", CENTER, 120 );
          myGLCD.printNumI ( tMI, CENTER, 120 );
        }
      break;

    case 9 :     //---------------- CHANGE LED VALUES SCREEN ---------------
      if ( ( x >= prSAVE [0] ) && ( x <= prSAVE [2] ) && ( y >= prSAVE [1] ) && ( y <= prSAVE [3] ) )  //press SAVE
      {
        waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
        dispScreen = 8;
        if ( COLOR == 1 ) {
          for ( int i; i < 96; i++ ) {
            wled [i] = tled [i];
          }
        }
        if ( COLOR == 2 ) {
          for ( int i; i < 96; i++ ) {
            bled [i] = tled [i];
          }
        }
        if ( COLOR == 3 ) {
          for ( int i; i < 96; i++ ) {
            rbled [i] = tled [i];
          }
        }
        if ( COLOR == 4 ) {
          for ( int i; i < 96; i++ ) {
            rled [i] = tled [i];
          }
        }
        if ( COLOR == 5 ) {
          for ( int i; i < 96; i++ ) {
            uvled [i] = tled [i];
          }
        }
        if ( COLOR == 6 ) {
          for ( int i; i < 96; i++ ) {
            sled [i] = tled [i];
          }
        }
        clearScreen ();
        ledValuesScreen ();
      }

      else if ( ( y >= 15 ) && ( y <= 40 ) )        //top row with times was touched
      {
        if ( ( x >= 4 ) && ( x <= 316 ) ) {
          int oldLCT = LedChangTime;
          LedChangTime = map ( x, 3, 320, 0, 12 );

          if ( oldLCT != LedChangTime )   //highlight touched time
          {
            myGLCD.setColor ( 0, 0, 0 );
            myGLCD.fillRect ( ( oldLCT * 26 ) + 5, 21, ( oldLCT * 26 ) + 29, 45 );
            setFont ( SMALL, 0, 255, 255, 0, 0, 0 );
            myGLCD.printNumI ( ( oldLCT * 2 ), ( oldLCT * 26 ) + 10, 22 );
            myGLCD.printNumI ( ( ( oldLCT * 2 ) + 1 ), ( oldLCT * 26 ) + 10, 33 );
            myGLCD.setColor ( 255, 0, 0 );
            myGLCD.fillRect ( ( LedChangTime * 26 ) + 5, 21, ( LedChangTime * 26 ) + 29, 45 );
            setFont ( SMALL, 255, 255, 255, 255, 0, 0 );
            myGLCD.printNumI ( ( LedChangTime * 2 ), ( LedChangTime * 26 ) + 10, 22 );
            myGLCD.printNumI ( ( ( LedChangTime * 2 ) + 1 ), ( LedChangTime * 26 ) + 10, 33 );

            for ( int i = 0; i < 8; i++ )    //print led values for highlighted time
            {
              int k = ( LedChangTime * 8 ) + i;
              setFont ( SMALL, 255, 255, 255, 0, 0, 0 );
              myGLCD.print ( "   ", ( i * 38 ) + 12, 105 );
              myGLCD.printNumI ( tled [k], ( i * 38 ) + 12, 105 );
            }
          }
        }
      }
      else if ( ( y >= 70 ) && ( y <= 95 ) )        //plus buttons were touched
      {
        for ( int i = 0; i < 8; i++ ) {
          if ( ( x >= ( i * 38 ) + 10 ) && ( x <= ( i * 38 ) + 35 ) ) {
            int k = ( LedChangTime * 8 ) + i;
            tled [k]++;
            if ( tled [k] > 255 ) {
              tled [k] = 255;
            }
            myGLCD.printNumI ( tled [k], ( i * 38 ) + 12, 105 );
          }
        }
      }
      else if ( ( y >= 125 ) && ( y <= 150 ) )      //minus buttons were touched
      {
        for ( int i = 0; i < 8; i++ ) {
          if ( ( x >= ( i * 38 ) + 10 ) && ( x <= ( i * 38 ) + 35 ) ) {
            int k = ( LedChangTime * 8 ) + i;
            tled [k]--;
            if ( tled [k] < 0 ) {
              tled [k] = 0;
            }
            myGLCD.print ( "  ", ( i * 38 ) + 20, 105 );
            myGLCD.printNumI ( tled [k], ( i * 38 ) + 12, 105 );
          }
        }
      }
      break;

    case 10 :     //-------------------- WAVEMAKER SCREEN -------------------
      if ( ( x >= 5 ) && ( x <= 155 ) && ( y >= 20 ) && ( y <= 40 ) )      //press Alternating Mode
      {
        waitForIt ( 5, 20, 155, 40 );
        Synch = 0;
        WAVE = 1;
        dispScreen = 11;
        clearScreen ();
        WaveMakerSettingsScreen ();
        waveMakerOff = true;
        digitalWrite ( WaveMakerTop, LOW );
        digitalWrite ( WaveMakerBottom, LOW );
      }
      else if ( ( x >= 165 ) && ( x <= 315 ) && ( y >= 20 ) && ( y <= 40 ) )    //press Synchronous Mode
      {
        waitForIt ( 165, 20, 315, 40 );
        WAVE = 2;
        dispScreen = 11;
        clearScreen ();
        WaveMakerSettingsScreen ();
        synchronousSynch ();
        waveMakerOff = true;
        digitalWrite ( WaveMakerTop, LOW );
        digitalWrite ( WaveMakerBottom, LOW );
      }
      else if ( ( x >= 165 ) && ( x <= 315 ) && ( y >= 46 ) && ( y <= 66 ) )    //press Turn Pumps OFF/ON
      {
        waitForIt ( 165, 46, 315, 66 );
        if ( WAVE == 3 ) {
          ReadFromEEPROM ();
          waveMakerOff = false;
          WaveMakerStatusScreen ();
        }
        else {
          MODE = WAVE;
          WAVE = 3;
          WaveMakerStatusScreen ();
        }
      }
      else if ( ( x >= 5 ) && ( x <= 155 ) && ( y >= 46 ) && ( y <= 66 ) )      //press Feeding Mode
      {
        waitForIt ( 5, 46, 155, 66 );
        if ( WAVE == 4 ) {
          ReadFromEEPROM ();
          waveMakerOff = false;
          WaveMakerStatusScreen ();
        }
        else {
          ReadFromEEPROM ();
          MODE = WAVE;
          WAVE = 4;
          countDown = 5 * 60 + 0;     //Reset countdown for 5 minutes and zero seconds
          MIN_O = 5;                 //Reset start time at 5 Minutes
          SEC_T = 0;
          SEC_O = 0;
          WaveMakerStatusScreen ();
        }
      }
      break;

    case 11 :     //---------------- WAVEMAKER SETTINGS SCREEN --------------
      if ( ( y >= 200 ) && ( y <= 220 ) )                          //Bottom Row
      {
        if ( ( x >= 5 ) && ( x <= 105 ) )                         //press BACK
        {
          waitForIt ( 5, 200, 105, 220 );
          ReadFromEEPROM ();
          waveMakerTest = false;
          waveMakerOff = false;
          dispScreen = 10;
          clearScreen ();
          WaveMakerScreen ();
          WaveMakerStatusScreen ();
        }
        else if ( ( x >= 110 ) && ( x <= 210 ) )                       //press TEST
        {
          waitForIt ( 110, 200, 210, 220 );
          myGLCD.setColor ( 0, 255, 0 );
          myGLCD.fillRoundRect ( 110, 200, 210, 220 );    //TEST Button in Green
          setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
          myGLCD.print ( "TESTING", 132, 204 );
          myGLCD.setColor ( 255, 255, 255 );
          myGLCD.drawRoundRect ( 110, 200, 210, 220 );
          waveMakerOff = false;
          waveMakerTest = true;
        }
        else if ( ( x >= 210 ) && ( x <= 315 ) )                       //press SAVE
        {
          waitForIt ( 215, 200, 315, 220 );
          if ( WAVE == 1 ) {
            Pump1m = Min1;
            Pump1s = Sec1;
            Pump2m = Min2;
            Pump2s = Sec2;
          }
          if ( WAVE == 2 ) {
            OnForTm = Min1;
            OnForTs = Sec1;
            OffForTm = Min2;
            OffForTs = Sec2;
          }
          SaveWaveToEEPROM ();
          waveMakerTest = false;
          waveMakerOff = false;
          dispScreen = 10;
          clearScreen ();
          WaveMakerScreen ();
          WaveMakerStatusScreen ();
        }
      }
      else {
        if ( WAVE == 1 )                                    //Alternating Mode Settings
        {
          waveModePlusMinus ();
        }

        if ( WAVE == 2 )                                    //Synchronous Mode Settings
        {
          if ( ( x >= 5 ) && ( x <= 155 ) && ( y >= 20 ) && ( y <= 40 ) )    //Constantly ON
          {
            waitForIt ( 5, 20, 155, 40 );
            Synch = 1;
            synchronousSynch ();
          }
          else if ( ( x >= 165 ) && ( x <= 315 ) && ( y >= 20 ) && ( y <= 40 ) )  //Pulsating Mode
          {
            waitForIt ( 165, 20, 315, 40 );
            Synch = 2;
            synchronousSynch ();
          }
          if ( Synch == 2 )                                //Synchronous-Pulsating Settings
          {
            waveModePlusMinus ();
          }
        }
      }
      break;

    case 12 :     //-------------------- GENERAL SETTINGS -------------------
      if ( ( x >= prSAVE [0] ) && ( x <= prSAVE [2] ) && ( y >= prSAVE [1] ) && ( y <= prSAVE [3] ) )  //press SAVE
      {
        waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
        SaveGenSetsToEEPROM ();
        dispScreen = MENUSCREEN_ONE;
        clearScreen ();
        menuScreen ();
      }
      if ( ( x >= 185 ) && ( x <= 305 ) && ( y >= 19 ) && ( y <= 39 ) )   //press DD/MM/YYYY Button
      {
        waitForIt ( 185, 19, 305, 39 );
        setCalendarFormat = 0;
        genSetSelect ();
      }

      if ( ( x >= 185 ) && ( x <= 305 ) && ( y >= 45 ) && ( y <= 65 ) )   //press Month DD, YYYY Button
      {
        waitForIt ( 185, 45, 305, 65 );
        setCalendarFormat = 1;
        genSetSelect ();
      }

      if ( ( x >= 195 ) && ( x <= 235 ) )                         //first column
      {
        if ( ( y >= 76 ) && ( y <= 96 ) )                        //press 12HR Button
        {
          waitForIt ( 195, 76, 235, 96 );
          setTimeFormat = 1;
          genSetSelect ();
        }
        if ( ( y >= 107 ) && ( y <= 127 ) )                      //press deg C
        {
          waitForIt ( 195, 107, 235, 127 );
          setTempScale = 0;
          genSetSelect ();
        }
        if ( ( y >= 138 ) && ( y <= 158 ) )                      //press Screensaver ON
        {
          waitForIt ( 195, 138, 235, 158 );
          setScreensaver = 1;
          genSetSelect ();
        }
        if ( ( y >= 169 ) && ( y <= 189 ) )                      //press Auto-Stop on Feed ON
        {
          waitForIt ( 195, 169, 235, 189 );
          setAutoStop = 1;
          genSetSelect ();
        }
      }
      if ( ( x >= 255 ) && ( x <= 295 ) )                         //second column
      {
        if ( ( y >= 76 ) && ( y <= 96 ) )                        //press 24HR Button
        {
          waitForIt ( 255, 76, 295, 96 );
          setTimeFormat = 0;
          genSetSelect ();
        }
        if ( ( y >= 107 ) && ( y <= 127 ) )                      //press deg F
        {
          waitForIt ( 255, 107, 295, 127 );
          setTempScale = 1;
          genSetSelect ();
        }
        if ( ( y >= 138 ) && ( y <= 158 ) )                      //press Screensaver OFF
        {
          waitForIt ( 255, 138, 295, 158 );
          setScreensaver = 2;
          genSetSelect ();
        }
        if ( ( y >= 169 ) && ( y <= 189 ) )                      //press Auto-Stop on Feed OFF
        {
          waitForIt ( 255, 169, 295, 189 );
          setAutoStop = 2;
          genSetSelect ();
        }
      }

      break;

    case 13 :     //--------------- AUTOMATIC FISH FEEDER PAGE --------------
      if ( ( x >= 5 ) && ( x <= 155 ) && ( y >= 20 ) && ( y <= 40 ) )      //press Feeding Time 1
      {
        waitForIt ( 5, 20, 155, 40 );
        feedTime = 1;
        dispScreen = 14;
        clearScreen ();
        setFeederTimesScreen ();
      }
      else if ( ( x >= 165 ) && ( x <= 315 ) && ( y >= 20 ) && ( y <= 40 ) )    //press Feeding Time 2
      {
        waitForIt ( 165, 20, 315, 40 );
        feedTime = 2;
        dispScreen = 14;
        clearScreen ();
        setFeederTimesScreen ();
      }
      else if ( ( x >= 5 ) && ( x <= 155 ) && ( y >= 168 ) && ( y <= 188 ) )      //press Feeding Time 3
      {
        waitForIt ( 5, 168, 155, 188 );
        feedTime = 3;
        dispScreen = 14;
        clearScreen ();
        setFeederTimesScreen ();
      }
      else if ( ( x >= 165 ) && ( x <= 315 ) && ( y >= 168 ) && ( y <= 188 ) )    //press Feeding Time 4
      {
        waitForIt ( 165, 168, 315, 188 );
        feedTime = 4;
        dispScreen = 14;
        clearScreen ();
        setFeederTimesScreen ();
      }
      else if ( ( x >= 85 ) && ( x <= 235 ) && ( y >= 94 ) && ( y <= 114 ) )      //press Feeding Fish Now!
      {
        waitForIt ( 85, 94, 235, 114 );
        myGLCD.setColor ( 0, 255, 0 );
        myGLCD.fillRoundRect ( 85, 94, 235, 114 );
        myGLCD.setColor ( 255, 255, 255 );
        myGLCD.drawRoundRect ( 85, 94, 235, 114 );
        setFont ( SMALL, 0, 0, 0, 0, 255, 0 );
        myGLCD.print ( "Now Feeding", 118, 98 );
        digitalWrite ( autoFeeder, HIGH );
        delay ( 5000 );
        myGLCD.setColor ( 153, 0, 102 );
        myGLCD.fillRoundRect ( 85, 94, 235, 114 );
        myGLCD.setColor ( 255, 255, 255 );
        myGLCD.drawRoundRect ( 85, 94, 235, 114 );
        setFont ( SMALL, 255, 255, 255, 153, 0, 102 );
        myGLCD.print ( "Feed Fish Now!", 106, 98 );
        digitalWrite ( autoFeeder, LOW );
      }
      break;

    case 14 :     //------------ SET AUTOMATIC FISH FEEDER TIMES ------------
      if ( ( x >= back [0] ) && ( x <= back [2] ) && ( y > back [1] ) && ( y <= back [3] ) )          //press back
      {
        waitForIt ( back [0], back [1], back [2], back [3] );
        ReadFromEEPROM ();
        if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) {
          AM_PM = 1;
        }
        else {
          AM_PM = 2;
        }
        dispScreen = 13;
        clearScreen ();
        autoFeederScreen ();
      }
      else if ( ( x >= prSAVE [0] ) && ( x <= prSAVE [2] ) && ( y >= prSAVE [1] ) && ( y <= prSAVE [3] ) )  //press SAVE
      {
        waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
        if ( setTimeFormat == 1 ) {
          if ( ( rtcSet [2] == 0 ) && ( AM_PM == 2 ) ) {
            rtcSet [2] += 12;
          }
          if ( ( ( rtcSet [2] >= 1 ) && ( rtcSet [2] <= 11 ) ) && ( AM_PM == 2 ) ) {
            rtcSet [2] += 12;
          }
          if ( ( ( rtcSet [2] >= 12 ) && ( rtcSet [2] <= 23 ) ) && ( AM_PM == 1 ) ) {
            rtcSet [2] -= 12;
          }
        }
        if ( feedTime == 1 ) {
          feedFish1H = rtcSet [2];
          feedFish1M = rtcSet [1];
        }
        if ( feedTime == 2 ) {
          feedFish2H = rtcSet [2];
          feedFish2M = rtcSet [1];
        }
        if ( feedTime == 3 ) {
          feedFish3H = rtcSet [2];
          feedFish3M = rtcSet [1];
        }
        if ( feedTime == 4 ) {
          feedFish4H = rtcSet [2];
          feedFish4M = rtcSet [1];
        }
        SaveFeedTimesToEEPROM ();
        dispScreen = 13;
        clearScreen ();
        autoFeederScreen ();
      }
      else if ( ( x >= 70 ) && ( x <= 250 ) && ( y >= 150 ) && ( y <= 170 ) )     //Feeding ON/OFF
      {
        waitForIt ( 70, 150, 250, 170 );
        if ( feedTime == 1 ) {
          if ( FEEDTime1 == 1 ) {
            FEEDTime1 = 0;
          }
          else {
            FEEDTime1 = 1;
          }
        }
        if ( feedTime == 2 ) {
          if ( FEEDTime2 == 1 ) {
            FEEDTime2 = 0;
          }
          else {
            FEEDTime2 = 1;
          }
        }
        if ( feedTime == 3 ) {
          if ( FEEDTime3 == 1 ) {
            FEEDTime3 = 0;
          }
          else {
            FEEDTime3 = 1;
          }
        }
        if ( feedTime == 4 ) {
          if ( FEEDTime4 == 1 ) {
            FEEDTime4 = 0;
          }
          else {
            FEEDTime4 = 1;
          }
        }
        feedingTimeOnOff ();
      }
      else {
        if ( ( y >= houP [1] ) && ( y <= houP [3] ) )                 //FIRST ROW
        {
          if ( ( x >= houP [0] ) && ( x <= houP [2] ) )              //press hour up
          {
            waitForIt ( houP [0], houP [1], houP [2], houP [3] );
            rtcSet [2]++;
            if ( rtcSet [2] >= 24 ) {
              rtcSet [2] = 0;
            }
          }
          if ( ( x >= minP [0] ) && ( x <= minP [2] ) )              //press min up
          {
            waitForIt ( minP [0], minP [1], minP [2], minP [3] );
            rtcSet [1]++;
            if ( rtcSet [1] >= 60 ) {
              rtcSet [1] = 0;
            }
          }
          if ( ( x >= ampmP [0] ) && ( x <= ampmP [2] ) )            //press AMPM up
          {
            waitForIt ( ampmP [0], ampmP [1], ampmP [2], ampmP [3] );
            if ( AM_PM == 1 ) {
              AM_PM = 2;
            }
            else {
              AM_PM = 1;
            }
          }
        }
        if ( ( y >= houM [1] ) && ( y <= houM [3] ) )                 //SECOND ROW
        {
          if ( ( x >= houM [0] ) && ( x <= houM [2] ) )              //press hour down
          {
            waitForIt ( houM [0], houM [1], houM [2], houM [3] );
            rtcSet [2]--;
            if ( rtcSet [2] < 0 ) {
              rtcSet [2] = 23;
            }
          }
          if ( ( x >= minM [0] ) && ( x <= minM [2] ) )              //press min down
          {
            waitForIt ( minM [0], minM [1], minM [2], minM [3] );
            rtcSet [1]--;
            if ( rtcSet [1] < 0 ) {
              rtcSet [1] = 59;
            }
          }
          if ( ( x >= ampmM [0] ) && ( x <= ampmM [2] ) )            //press AMPM down
          {
            waitForIt ( ampmM [0], ampmM [1], ampmM [2], ampmM [3] );
            if ( AM_PM == 1 ) {
              AM_PM = 2;
            }
            else {
              AM_PM = 1;
            }
          }
        }
        setFeederTimesScreen ( false );
      }
      break;

    case 15 :     //------------------------- ABOUT -------------------------
      break;

    case 16 :  //---- options Menu 2

      if ( ( x >= dosingPumpButton [0] ) && ( x <= dosingPumpButton [2] ) )                      //first column
      {
        if ( ( y >= dosingPumpButton [1] ) && ( y <= dosingPumpButton [3] ) )                   //press Dosing pump Screen
        {
          waitForIt ( dosingPumpButton [0], dosingPumpButton [1], dosingPumpButton [2], dosingPumpButton [3] );

          dispScreen = 17;
          clearScreen ();
          dosingPumpMainMenu ();
        }
      }

      break;
    case 17 :  // dosing pump main menu
      //				Serial.println ( "Touch screen 17" );
      if ( ( x >= 5 ) && ( x <= 155 ) && ( y >= 20 ) && ( y <= 40 ) )      //press Feeding Time 1
      {
        waitForIt ( 5, 20, 155, 40 );
        dosingPumpSelected = 1;
        dispScreen = 18;
        clearScreen ();
        dosingPumpSubmenu ();
      }
      else if ( ( x >= 165 ) && ( x <= 315 ) && ( y >= 20 ) && ( y <= 40 ) )    //press Feeding Time 2
      {
        waitForIt ( 165, 20, 315, 40 );
        dosingPumpSelected = 2;
        dispScreen = 18;
        clearScreen ();
        dosingPumpSubmenu ();
      }
      else if ( ( x >= 5 ) && ( x <= 155 ) && ( y >= 168 ) && ( y <= 188 ) )      //press Feeding Time 3
      {
        waitForIt ( 5, 168, 155, 188 );
        dosingPumpSelected = 3;
        dispScreen = 18;
        clearScreen ();
        dosingPumpSubmenu ();
      }
      else if ( ( x >= 165 ) && ( x <= 315 ) && ( y >= 168 ) && ( y <= 188 ) )    //press Feeding Time 4
      {
        waitForIt ( 165, 168, 315, 188 );
        dosingPumpSelected = 4;
        dispScreen = 18;
        clearScreen ();
        dosingPumpSubmenu ();
      }
      break;

    case 18 :  // dosing pump settings
      if ( ( x >= prSAVE [0] ) && ( x <= prSAVE [2] ) && ( y >= prSAVE [1] ) && ( y <= prSAVE [3] ) )  //press SAVE / NEXT
      {
        waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
        dispScreen = 17;

        if ( ( * ( pumpml [dosingPumpSelected - 1] [0] ) ) == 0 ) {
          ( * ( pumpOn [dosingPumpSelected - 1] ) ) = 0;
        }
        else {
          for ( int i = 3; i >= 0; i-- ) {
            if ( ( ( * ( pumpOn [dosingPumpSelected - 1] ) ) == 0 ) && ( ( * ( pumpml [dosingPumpSelected - 1] [i] ) ) > 0 ) ) {
              ( * ( pumpOn [dosingPumpSelected - 1] ) ) = i;
            }
          }
        }

        SaveDosingPumpToEEPROM ();
        dosingPumpSettingsChanged = false;
        clearScreen ();
        dosingPumpMainMenu ();
      }
      else if ( ( x >= calibrateDosingPump [0] ) && ( x <= calibrateDosingPump [2] ) && ( y >= calibrateDosingPump [1] )
        && ( y <= calibrateDosingPump [3] ) )  //press calibrate
      {
        waitForIt ( calibrateDosingPump [0], calibrateDosingPump [1], calibrateDosingPump [2], calibrateDosingPump [3] );
        dispScreen = 19;
        clearScreen ();
        //					ReadFromEEPROM ();
        dosingPumpCalibration ();
      }
      else if ( ( x >= back [0] ) && ( x <= back [2] ) && ( y >= back [1] ) && ( y <= back [3] ) ) {  // press back
        waitForIt ( calibrateDosingPump [0], calibrateDosingPump [1], calibrateDosingPump [2], calibrateDosingPump [3] );
        dispScreen = 17;
        ReadFromEEPROM ();
        clearScreen ();
        dosingPumpMainMenu ();
        dosingPumpSettingsChanged = false;

      }

      else {
        if ( ( x >= dosingHour1up [0] ) && ( x <= dosingHour1up [2] ) )                      //Hour column
        {
          if ( ( y >= dosingHour1up [1] ) && ( y <= dosingHour1up [3] ) )                   //hour 1 up
          {
            waitForIt ( dosingHour1up [0], dosingHour1up [1], dosingHour1up [2], dosingHour1up [3] );

            ( * ( pumph [dosingPumpSelected - 1] [0] ) )++;
            if ( ( * ( pumph [dosingPumpSelected - 1] [0] ) ) >= 24 ) {
              ( * ( pumph [dosingPumpSelected - 1] [0] ) ) = 0;
            }
            dosingPumpSettingsChanged = true;
            //							Serial.println ( ( * ( pumph [dosingPumpSelected - 1] [0] ) ) );
          }
          if ( ( y >= dosingHour1down [1] ) && ( y <= dosingHour1down [3] ) )                   //hour 1 down
          {
            waitForIt ( dosingHour1down [0], dosingHour1down [1], dosingHour1down [2], dosingHour1down [3] );
            ( * ( pumph [dosingPumpSelected - 1] [0] ) )--;
            if ( ( * ( pumph [dosingPumpSelected - 1] [0] ) ) < 0 ) {
              ( * ( pumph [dosingPumpSelected - 1] [0] ) ) = 23;
            }
            dosingPumpSettingsChanged = true;
          }

          if ( ( y >= dosingHour2up [1] ) && ( y <= dosingHour2up [3] ) )                   //hour 1 up
          {
            waitForIt ( dosingHour2up [0], dosingHour2up [1], dosingHour2up [2], dosingHour2up [3] );

            ( * ( pumph [dosingPumpSelected - 1] [1] ) )++;
            if ( ( * ( pumph [dosingPumpSelected - 1] [1] ) ) >= 24 ) {
              ( * ( pumph [dosingPumpSelected - 1] [1] ) ) = 0;
            }
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingHour2down [1] ) && ( y <= dosingHour2down [3] ) )                   //hour 1 down
          {
            waitForIt ( dosingHour2down [0], dosingHour2down [1], dosingHour2down [2], dosingHour2down [3] );
            ( * ( pumph [dosingPumpSelected - 1] [1] ) )--;
            if ( ( * ( pumph [dosingPumpSelected - 1] [1] ) ) < 0 ) {
              ( * ( pumph [dosingPumpSelected - 1] [1] ) ) = 23;
            }
            dosingPumpSettingsChanged = true;
          }

          if ( ( y >= dosingHour3up [1] ) && ( y <= dosingHour3up [3] ) )                   //hour 1 up
          {
            waitForIt ( dosingHour3up [0], dosingHour3up [1], dosingHour3up [2], dosingHour3up [3] );

            ( * ( pumph [dosingPumpSelected - 1] [2] ) )++;
            if ( ( * ( pumph [dosingPumpSelected - 1] [2] ) ) >= 24 ) {
              ( * ( pumph [dosingPumpSelected - 1] [2] ) ) = 0;
            }
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingHour3down [1] ) && ( y <= dosingHour3down [3] ) )                   //hour 1 down
          {
            waitForIt ( dosingHour3down [0], dosingHour3down [1], dosingHour3down [2], dosingHour3down [3] );
            ( * ( pumph [dosingPumpSelected - 1] [2] ) )--;
            if ( * ( pumph [dosingPumpSelected - 1] [2] ) < 0 ) {
              * ( pumph [dosingPumpSelected - 1] [2] ) = 23;
            }
            dosingPumpSettingsChanged = true;
          }
          //						clearScreen ();
          dosingPumpSubmenu ();
        }
        if ( ( x >= dosingMinute1up [0] ) && ( x <= dosingMinute1up [2] ) )                      //Minute column
        {
          if ( ( y >= dosingMinute1up [1] ) && ( y <= dosingMinute1up [3] ) )                   //Minute 1 up
          {
            waitForIt ( dosingMinute1up [0], dosingMinute1up [1], dosingMinute1up [2], dosingMinute1up [3] );

            ( * ( pumpm [dosingPumpSelected - 1] [0] ) )++;
            if ( ( * ( pumpm [dosingPumpSelected - 1] [0] ) ) >= 60 ) {
              ( * ( pumpm [dosingPumpSelected - 1] [0] ) ) = 0;
            }
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingMinute1down [1] ) && ( y <= dosingMinute1down [3] ) )                   //Minute 1 down
          {
            waitForIt ( dosingMinute1down [0], dosingMinute1down [1], dosingMinute1down [2], dosingMinute1down [3] );
            ( * ( pumpm [dosingPumpSelected - 1] [0] ) )--;
            if ( ( * ( pumpm [dosingPumpSelected - 1] [0] ) ) < 0 ) {
              ( * ( pumpm [dosingPumpSelected - 1] [0] ) ) = 59;
            }
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingMinute2up [1] ) && ( y <= dosingMinute2up [3] ) )                   //Minute 1 up
          {
            waitForIt ( dosingMinute2up [0], dosingMinute2up [1], dosingMinute2up [2], dosingMinute2up [3] );

            ( * ( pumpm [dosingPumpSelected - 1] [1] ) )++;
            if ( ( * ( pumpm [dosingPumpSelected - 1] [1] ) ) >= 60 ) {
              ( * ( pumpm [dosingPumpSelected - 1] [1] ) ) = 0;
            }
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingMinute2down [1] ) && ( y <= dosingMinute2down [3] ) )                   //Minute 1 down
          {
            waitForIt ( dosingMinute2down [0], dosingMinute2down [1], dosingMinute2down [2], dosingMinute2down [3] );
            ( * ( pumpm [dosingPumpSelected - 1] [1] ) )--;
            if ( ( * ( pumpm [dosingPumpSelected - 1] [1] ) ) < 0 ) {
              ( * ( pumpm [dosingPumpSelected - 1] [1] ) ) = 59;
            }
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingMinute3up [1] ) && ( y <= dosingMinute3up [3] ) )                   //Minute 1 up
          {
            waitForIt ( dosingMinute3up [0], dosingMinute3up [1], dosingMinute3up [2], dosingMinute3up [3] );

            ( * ( pumpm [dosingPumpSelected - 1] [2] ) )++;
            if ( ( * ( pumpm [dosingPumpSelected - 1] [2] ) ) >= 60 ) {
              ( * ( pumpm [dosingPumpSelected - 1] [2] ) ) = 0;
            }
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingMinute3down [1] ) && ( y <= dosingMinute3down [3] ) )                   //Minute 1 down
          {
            waitForIt ( dosingMinute3down [0], dosingMinute3down [1], dosingMinute3down [2], dosingMinute3down [3] );
            ( * ( pumpm [dosingPumpSelected - 1] [2] ) )--;
            if ( ( * ( pumpm [dosingPumpSelected - 1] [2] ) ) < 0 ) {
              ( * ( pumpm [dosingPumpSelected - 1] [2] ) ) = 59;
            }
            dosingPumpSettingsChanged = true;
          }
          dosingPumpSubmenu ();
        }

        if ( ( x >= dosingML1up [0] ) && ( x <= dosingML1up [2] ) )                      //ML column
        {
          if ( ( y >= dosingML1up [1] ) && ( y <= dosingML1up [3] ) )                   //ML 1 up
          {
            waitForIt ( dosingML1up [0], dosingML1up [1], dosingML1up [2], dosingML1up [3] );

            ( * ( pumpml [dosingPumpSelected - 1] [0] ) ) += 10;
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingML1down [1] ) && ( y <= dosingML1down [3] ) )                   //ML 1 down
          {
            waitForIt ( dosingML1down [0], dosingML1down [1], dosingML1down [2], dosingML1down [3] );
            ( * ( pumpml [dosingPumpSelected - 1] [0] ) ) -= 10;
            if ( ( * ( pumpml [dosingPumpSelected - 1] [0] ) ) < 0 ) {
              ( * ( pumpml [dosingPumpSelected - 1] [0] ) ) = 0;
            }
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingML2up [1] ) && ( y <= dosingML2up [3] ) )                   //ML 1 up
          {
            waitForIt ( dosingML2up [0], dosingML2up [1], dosingML2up [2], dosingML2up [3] );

            ( * ( pumpml [dosingPumpSelected - 1] [1] ) ) += 10;
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingML2down [1] ) && ( y <= dosingML2down [3] ) )                   //ML 1 down
          {
            waitForIt ( dosingML2down [0], dosingML2down [1], dosingML2down [2], dosingML2down [3] );
            ( * ( pumpml [dosingPumpSelected - 1] [1] ) ) -= 10;
            if ( ( * ( pumpml [dosingPumpSelected - 1] [1] ) ) < 0 ) {
              ( * ( pumpml [dosingPumpSelected - 1] [1] ) ) = 0;
            }
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingML3up [1] ) && ( y <= dosingML3up [3] ) )                   //ML 1 up
          {
            waitForIt ( dosingML3up [0], dosingML3up [1], dosingML3up [2], dosingML3up [3] );

            ( * ( pumpml [dosingPumpSelected - 1] [2] ) ) += 10;
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingML3down [1] ) && ( y <= dosingML3down [3] ) )                   //ML 1 down
          {
            waitForIt ( dosingML3down [0], dosingML3down [1], dosingML3down [2], dosingML3down [3] );
            ( * ( pumpml [dosingPumpSelected - 1] [2] ) ) -= 10;
            if ( ( * ( pumpml [dosingPumpSelected - 1] [2] ) ) < 0 ) {
              ( * ( pumpml [dosingPumpSelected - 1] [2] ) ) = 0;
            }
            dosingPumpSettingsChanged = true;
          }
          dosingPumpSubmenu ();
        }
        if ( ( x >= dosingSubML1up [0] ) && ( x <= dosingSubML1up [2] ) )                      //SubML column
        {
          if ( ( y >= dosingSubML1up [1] ) && ( y <= dosingSubML1up [3] ) )                   //SubML 1 up
          {
            waitForIt ( dosingSubML1up [0], dosingSubML1up [1], dosingSubML1up [2], dosingSubML1up [3] );

            ( * ( pumpml [dosingPumpSelected - 1] [0] ) )++;
            dosingPumpSettingsChanged = true;

          }
          if ( ( y >= dosingSubML1down [1] ) && ( y <= dosingSubML1down [3] ) )                   //SubML 1 down
          {
            waitForIt ( dosingSubML1down [0], dosingSubML1down [1], dosingSubML1down [2], dosingSubML1down [3] );
            ( * ( pumpml [dosingPumpSelected - 1] [0] ) )--;
            if ( ( * ( pumpml [dosingPumpSelected - 1] [0] ) ) < 0 ) {
              ( * ( pumpml [dosingPumpSelected - 1] [0] ) ) = 0;
            }
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingSubML2up [1] ) && ( y <= dosingSubML2up [3] ) )                   //SubML 1 up
          {
            waitForIt ( dosingSubML2up [0], dosingSubML2up [1], dosingSubML2up [2], dosingSubML2up [3] );

            ( * ( pumpml [dosingPumpSelected - 1] [1] ) )++;
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingSubML2down [1] ) && ( y <= dosingSubML2down [3] ) )                   //SubML 1 down
          {
            waitForIt ( dosingSubML2down [0], dosingSubML2down [1], dosingSubML2down [2], dosingSubML2down [3] );
            ( * ( pumpml [dosingPumpSelected - 1] [1] ) )--;
            if ( ( * ( pumpml [dosingPumpSelected - 1] [1] ) ) < 0 ) {
              ( * ( pumpml [dosingPumpSelected - 1] [1] ) ) = 0;
            }
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingSubML3up [1] ) && ( y <= dosingSubML3up [3] ) )                   //SubML 1 up
          {
            waitForIt ( dosingSubML3up [0], dosingSubML3up [1], dosingSubML3up [2], dosingSubML3up [3] );

            ( * ( pumpml [dosingPumpSelected - 1] [2] ) )++;
            dosingPumpSettingsChanged = true;
          }
          if ( ( y >= dosingSubML3down [1] ) && ( y <= dosingSubML3down [3] ) )                   //SubML 1 down
          {
            waitForIt ( dosingSubML3down [0], dosingSubML3down [1], dosingSubML3down [2], dosingSubML3down [3] );
            ( * ( pumpml [dosingPumpSelected - 1] [2] ) )--;
            if ( ( * ( pumpml [dosingPumpSelected - 1] [2] ) ) < 0 ) {
              ( * ( pumpml [dosingPumpSelected - 1] [2] ) ) = 0;
            }
            dosingPumpSettingsChanged = true;
          }
          dosingPumpSubmenu ();
        }
        if ( ( x >= dosingPump1Off [0] ) && ( x <= dosingPump1Off [2] ) )                      //Reset buttons
        {
          if ( ( y >= dosingPump1Off [1] ) && ( y <= dosingPump1Off [3] ) )                   //SubML 1 down
          {
            waitForIt ( dosingPump1Off [0], dosingPump1Off [1], dosingPump1Off [2], dosingPump1Off [3] );
            ( * ( pumpml [dosingPumpSelected - 1] [0] ) ) = 0;
            dosingPumpSettingsChanged = true;

          }
          if ( ( y >= dosingPump2Off [1] ) && ( y <= dosingPump2Off [3] ) )                   //SubML 1 down
          {
            waitForIt ( dosingPump2Off [0], dosingPump2Off [1], dosingPump2Off [2], dosingPump2Off [3] );
            ( * ( pumpml [dosingPumpSelected - 1] [1] ) ) = 0;
            dosingPumpSettingsChanged = true;

          }
          if ( ( y >= dosingPump3Off [1] ) && ( y <= dosingPump3Off [3] ) )                   //SubML 1 down
          {
            waitForIt ( dosingPump3Off [0], dosingPump3Off [1], dosingPump3Off [2], dosingPump3Off [3] );
            ( * ( pumpml [dosingPumpSelected - 1] [2] ) ) = 0;
            dosingPumpSettingsChanged = true;
          }

          //						clearScreen ();
          dosingPumpSubmenu ();
        }
      }
      break;
    case 19 :  // dosing pump calibration
      if ( ( x >= prSAVE [0] ) && ( x <= prSAVE [2] ) && ( y >= prSAVE [1] ) && ( y <= prSAVE [3] ) )  //press SAVE
      {
        waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
        dispScreen = 17;
        // save ml values
        //*pumpCalibrate [dosingPumpSelected - 1] =
        SaveDosingPumpToEEPROM ();
        clearScreen ();
        dosingPumpMainMenu ();
        dosingPumpOffTimes[dosingPumpSelected - 1]=0;
        dosingPumpCalibrationActive = false;
      }
      else if ( ( x >= dosingCalibrationStart [0] ) && ( x <= dosingCalibrationStart [2] ) && ( y >= dosingCalibrationStart [1] )
        && ( y <= dosingCalibrationStart [3] ) )  //press calibrate start
      {
        waitForIt ( calibrateDosingPump [0], calibrateDosingPump [1], calibrateDosingPump [2], calibrateDosingPump [3] );
        //	clearScreen ();
        //					ReadFromEEPROM ();
        if ( dosingPumpOffTimes [dosingPumpSelected - 1] < millis () ) {
          // Dosing pump is started for pump dosingPumpSelected
          dosingPumpCalibrationActive = true;
          dosingPumpOffTimes [dosingPumpSelected - 1] = millis () + ( testtime * 1000L );
          digitalWrite ( *pumpPins [dosingPumpSelected - 1], HIGH );
        }
        else {
          dosingPumpCalibrationActive = false;
          dosingPumpOffTimes [dosingPumpSelected - 1] = 0L;
          digitalWrite ( *pumpPins [dosingPumpSelected - 1], LOW );
        }
        //					dosingPumpCalibrationActive = !dosingPumpCalibrationActive;
        dosingPumpCalibration ( false );
      }
      else if ( ( x >= back [0] ) && ( x <= back [2] ) && ( y >= back [1] ) && ( y <= back [3] ) ) {  // press back
        waitForIt ( calibrateDosingPump [0], calibrateDosingPump [1], calibrateDosingPump [2], calibrateDosingPump [3] );
        dispScreen = 18;
        ReadFromEEPROM ();
        clearScreen ();
        dosingPumpSubmenu ();

      }
      else {
        if ( ( x >= dosingCalibrationMLup [0] ) && ( x <= dosingCalibrationMLup [2] ) )                      //ML column
        {
          if ( ( y >= dosingCalibrationMLup [1] ) && ( y <= dosingCalibrationMLup [3] ) )                      //calibrate ml up
          {

            waitForIt ( dosingCalibrationMLup [0], dosingCalibrationMLup [1], dosingCalibrationMLup [2], dosingCalibrationMLup [3] );
            ( *pumpCalibrate [dosingPumpSelected - 1] ) += 10;
            dosingPumpCalibration ( true );

          }
          else if ( ( y >= dosingCalibrationMLdown [1] ) && ( y <= dosingCalibrationMLdown [3] ) )                      //calibrate ml down
          {
            waitForIt ( dosingCalibrationMLdown [0], dosingCalibrationMLdown [1], dosingCalibrationMLdown [2], dosingCalibrationMLdown [3] );
            ( * ( pumpCalibrate [dosingPumpSelected - 1] ) ) -= 10;

            if ( ( * ( pumpCalibrate [dosingPumpSelected - 1] ) ) < 0 ) {
              ( * ( pumpCalibrate [dosingPumpSelected - 1] ) ) = 0;
            }
            dosingPumpCalibration ( true );
          }
        }
        if ( ( x >= dosingCalibrationSubMLup [0] ) && ( x <= dosingCalibrationSubMLup [2] ) )                      //SubML column
        {
          if ( ( y >= dosingCalibrationSubMLup [1] ) && ( y <= dosingCalibrationSubMLup [3] ) )                      //calibrate subml up
          {
            waitForIt ( dosingCalibrationSubMLup [0], dosingCalibrationSubMLup [1], dosingCalibrationSubMLup [2],
            dosingCalibrationSubMLup [3] );
            ( * ( pumpCalibrate [dosingPumpSelected - 1] ) ) += 1;
            dosingPumpCalibration ( true );
          }
          else if ( ( y >= dosingCalibrationSubMLdown [1] ) && ( y <= dosingCalibrationSubMLdown [3] ) )                    //calibrate subml down
          {
            waitForIt ( dosingCalibrationSubMLdown [0], dosingCalibrationSubMLdown [1], dosingCalibrationSubMLdown [2],
            dosingCalibrationSubMLdown [3] );
            ( * ( pumpCalibrate [dosingPumpSelected - 1] ) ) -= 1;

            if ( ( * ( pumpCalibrate [dosingPumpSelected - 1] ) ) < 0 ) {
              ( * ( pumpCalibrate [dosingPumpSelected - 1] ) ) = 0;
            }
            dosingPumpCalibration ( true );
          }
        }
      }
      break;

    }
  }
  delay ( 100 );
}
/********************************* END of TOUCH SCREEN ********************************/

/**************************************** SETUP **************************************/
void setup () {
  Serial.begin ( 9600 );
  // Serial.write("Hello");

  TCCR5A = B00101011;  // Fast PWM change at OCR5A (Timer 5 - pins 44 & 45)
  TCCR5B = B10001;    // No Prescalering (Timer 5 - pins 44 & 45)

  pinMode ( ledPinSump, OUTPUT );
  pinMode ( ledPinBlue, OUTPUT );
  pinMode ( ledPinWhite, OUTPUT );
  pinMode ( ledPinRoyBlue, OUTPUT );
  pinMode ( ledPinRed, OUTPUT );
  pinMode ( ledPinUV, OUTPUT );
  pinMode ( ledPinMoon, OUTPUT );

  pinMode ( HoodFansPWM, OUTPUT );
  pinMode ( SumpFanPWM, OUTPUT );
  pinMode ( HoodFansTranzPin, OUTPUT );
  pinMode ( SumpFanTranzPin, OUTPUT );

  pinMode ( tempHeatPin, OUTPUT );
  pinMode ( tempChillPin, OUTPUT );
  pinMode ( tempAlarmPin, OUTPUT );

  pinMode ( WaveMakerTop, OUTPUT );
  pinMode ( WaveMakerBottom, OUTPUT );

  pinMode ( *pumpPins [0], OUTPUT );
  pinMode ( *pumpPins [1], OUTPUT );
  pinMode ( *pumpPins [2], OUTPUT );
  pinMode ( *pumpPins [3], OUTPUT );

  digitalWrite ( *pumpPins [0], LOW );
  digitalWrite ( *pumpPins [1], LOW );
  digitalWrite ( *pumpPins [2], LOW );
  digitalWrite ( *pumpPins [3], LOW );

  pinMode ( autoFeeder, OUTPUT );

  //  Serial.write(" Init Screen");
  myGLCD.InitLCD ( LANDSCAPE );
  myGLCD.clrScr ();

  myTouch.InitTouch ( LANDSCAPE );
  myTouch.setPrecision ( PREC_MEDIUM );
  if ( checkTemp ) {
    //    Serial.write(" Init Temp");

    sensors.begin ();     //start up temperature library
    // set the resolution to 9 bit
    sensors.setResolution ( waterThermometer, 9 );
    sensors.setResolution ( hoodThermometer, 9 );
    sensors.setResolution ( sumpThermometer, 9 );
  }
  //  Serial.write(" Init Screen 2");
  myGLCD.setColor ( 64, 64, 64 );
  myGLCD.fillRect ( 0, 226, 319, 239 );                     //Bottom Bar
  // Serial.write(" Init RTC");
  RTC.get ( rtc, true );
  min_cnt = ( rtc [2] * 60 ) + rtc [1];
  //	SaveDosingPumpToEEPROM();
  ReadFromEEPROM ();
  initializeDimmExecution ();
  LED_levels_output ();
  // Serial.write(" Init Mainscreen");
  wave_output ();
  mainScreen ( true );
}
/*********************************** END of SETUP ************************************/

/********************************** BEGIN MAIN LOOP **********************************/
void loop () {

  if ( ( myTouch.dataAvailable () ) && ( screenSaverTimer >= setScreenSaverTimer ) ) {
    LEDtestTick = false;
    myGLCD.setColor ( 64, 64, 64 );
    myGLCD.fillRect ( 0, 226, 319, 239 );                     //Bottom Bar
    screenSaverTimer = 0;
    clearScreen ();
    mainScreen ( true );
    delay ( 1000 );
    dispScreen = 0;
  }
  else {
    if ( myTouch.dataAvailable () ) {
      processMyTouch ();
    }
  }
  // dosing pump calibration
  //	if ( dosingPumpCalibrationActive ) {
  //		if ( calibrationEnd == 0 ) {
  //			calibrationEnd = millis () + ( testtime * 1000 );
  //
  //		}
  //		if ( ( calibrationEnd > 0 ) && ( calibrationEnd > millis () ) ) {
  //			digitalWrite ( *pumpPins [dosingPumpSelected - 1], HIGH );
  //		}
  //		else {
  //			digitalWrite ( *pumpPins [dosingPumpSelected - 1], LOW );
  //			dosingPumpCalibrationActive = false;
  //			calibrationEnd = 0;
  //			dosingPumpCalibration ( false );
  //		}
  //	}
  //	else if ( calibrationEnd > 0 ) {
  //		digitalWrite ( *pumpPins [dosingPumpSelected - 1], LOW );
  //		dosingPumpCalibrationActive = false;
  //		calibrationEnd = 0;
  //	}
  checkDosingPumpOnOff ();

  if ( waveMakerOff == false ) {
    wave_output ();
  }
  else {
    PumpTstate = LOW;
    PumpBstate = LOW;
  }

  unsigned long currentMillis = millis ();
  if ( currentMillis - previousMillisFive > 5000 )   //check time, temp and LED levels every 5s
  {
    previousMillisFive = currentMillis;
    RTC.get ( rtc, true );

    feedingTimeOutput ();

    if ( screenSaverTimer < setScreenSaverTimer ) {
      TimeDateBar ();
    }

    checkTempC ();
    OCR5A = 16000000.0 / ( 2 * 25000.0 );            //25kHz PWM - above our audible range so fans are quiet
    OCR5B = 16000000.0 / ( 2 * 25000.0 ) * SumpPWM;  //"SumpPWM" is the % duty cycle for pin 45
    OCR5C = 16000000.0 / ( 2 * 25000.0 ) * HoodPWM;  //"HoodPWM" is the % duty cycle for pin 44

    min_cnt = ( rtc [2] * 60 ) + rtc [1];
    LED_levels_output ();

    screenReturn ();
    screenSaver ();

    if ( ( dispScreen == 0 ) && ( screenSaverTimer < setScreenSaverTimer ) ) {
      mainScreen ();
    }
  }
}

/********************************** END OF MAIN LOOP *********************************/

/************************************ Compute led use time **************************/
// getaverageontime !!
/********************************* send dimm values *********************************/
void setzeDimmWerte ( uint8_t weiss, uint8_t blau ) {

  mydata.blau1 = blau;
  mydata.blau2 = blau;
  mydata.weiss1 = weiss;
  mydata.weiss2 = weiss;
  mydata.weiss3 = weiss;

  sendeDimmWerte ();
}
void initializeDimmExecution () {

  Serial1.begin ( 9600 );
  //start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc.
  ET.begin ( details(mydata), &Serial1 );

}
void sendeDimmWerte () {

  ET.sendData ();
}

//v23


