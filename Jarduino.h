// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef Jarduino_H_
#define Jarduino_H_
#include "Arduino.h"
//add your includes for the project Jarduino here

//#include <ITDB02_Graph16.h>
//#include <avr/pgmspace.h>
//#include <ITDB02_Touch.h>
//#include <Wire.h>
//#include <EEPROM.h>
////#include "writeAnything.h"
//#include <DS1307.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>

//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif
	void loop ();
	void setup ();
#ifdef __cplusplus
}  // extern "C"
#endif

//add your function definitions for the project Jarduino here

int check ( byte *pt1, byte *pt2, int lstep );
float moonPhase ( int moonYear, int moonMonth, int moonDay );
double MyNormalize ( double v );
void processMyTouch ();
void setFont ( boolean font, byte cr, byte cg, byte cb, byte br, byte bg, byte bb );
void timeChange ();
void timeCorrectFormat ();
void viewWaveTimesPage ();
void viewWaveTimesPage ();
void setzeDimmWerte ( uint8_t weiss, uint8_t blau );
void initializeDimmExecution ();
void sendeDimmWerte ();
void ReadFromEEPROM ();
void mainScreen ( boolean refreshAll );
void clearScreen ();
void menuScreen ();
void autoFeederScreen ();
void ledColorViewScreen ();
void WaveMakerScreen ();
void WaveMakerStatusScreen ();
void ledTestOptionsScreen ();
void SaveRTC ();
void SaveTempToEEPROM ();
void SaveMoonLEDToEEPROM ();
void ledValuesScreen ();
void SaveGenSetsToEEPROM ();
void SaveFeedTimesToEEPROM ();
void clockScreen ( boolean refreshAll );
void tempScreen ( boolean refreshAll );
void maindispNoHit ( int x, int y, int page );
void generalSettingsScreen ();
void AboutScreen () ;
//Do not add code below this line
#endif /* Jarduino_H_ */
