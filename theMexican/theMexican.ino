/*
  MP3 Shield Trigger
  by: Jim Lindblom
      SparkFun Electronics
  date: September 23, 2013
  
  This is an example MP3 trigger sketch for the SparkFun MP3 Shield.
  Pins 0, 1, 5, 10, A0, A1, A2, A3, and A4 are setup to trigger tracks
  "track001.mp3", "track002.mp3", etc. on an SD card loaded into
  the shield. Whenever any of those pins are shorted to ground,
  their respective track will start playing.
  
  When a new pin is triggered, any track currently playing will
  stop, and the new one will start.
  
  A5 is setup to globally STOP playing a track when triggered.
  
  If you need more triggers, the shield's jumpers on pins 3 and 4 
  (MIDI-IN and GPIO1) can be cut open and used as additional
  trigger pins. Also, because pins 0 and 1 are used as triggers
  Serial is not available for debugging. Disable those as
  triggers if you want to use serial.
  
  Much of this code was grabbed from the FilePlayer example
  included with the SFEMP3Shield library. Major thanks to Bill
  Porter, again, for this amazing library!
*/
#include <SPI.h>

//Add the SdFat Libraries
#include <SdFat.h>
#include <SdFatUtil.h>
#include <Bounce2.h> 
//and the MP3 Shield Library
#include <SFEMP3Shield.h>
SdFat sd; // Create object to handle SD functions

SFEMP3Shield MP3player; // Create Mp3 library object
// These variables are used in the MP3 initialization to set up
// some stereo options:
const uint8_t volume = 0; // MP3 Player volume 0=max, 255=lowest (off)
const uint16_t monoMode = 1;  // Mono setting 0=off, 3=max
#define BUTTON_DEBOUNCE_PERIOD 20 //ms
/* Pin setup */
#define TRIGGER_COUNT 2
int TRIGGERSWITCH = A0;
int RANDOMSWITCH = A4;
int STOPPIN = A5; // This pin triggers a track stop.
int noOfSamples = 6;
int trackToPlay = 0;
int currentVol = 0;
int currentBank=0;
Bounce triggerSwitch  = Bounce();
Bounce randomSwitch  = Bounce();
Bounce stopSwitch  = Bounce();

void setup()
{
  randomSeed(analogRead(A1));
  Serial.begin(115200);
  
  /* Set up all trigger pins as inputs, with pull-ups activated: */
  pinMode(TRIGGERSWITCH, INPUT_PULLUP);
  pinMode(RANDOMSWITCH, INPUT_PULLUP);
  pinMode(STOPPIN, INPUT_PULLUP);
  
  triggerSwitch.attach(TRIGGERSWITCH);
  triggerSwitch.interval(BUTTON_DEBOUNCE_PERIOD);
  //randomSwitch.attach(RANDOMSWITCH);
  //randomSwitch.interval(BUTTON_DEBOUNCE_PERIOD);
  stopSwitch.attach(STOPPIN);
  stopSwitch.interval(BUTTON_DEBOUNCE_PERIOD);
  
  Serial.println("Pins setup" );
  initSD();  // Initialize the SD card
  initMP3Player(); // Initialize the MP3 Shield
  
}

// All the loop does is continuously step through the trigger
//  pins to see if one is pulled low. If it is, it'll stop any
//  currently playing track, and start playing a new one.
void loop()
{
      int vol = analogRead(A1);
      int volToSet = map(vol, 0, 1023, 0, 140);
      int bankPot = analogRead(A2);
        Serial.print("Bank pot: " );
      Serial.println(bankPot);
      
      int bank = map(bankPot, 0, 1023, 1, 10);
      Serial.print("Bank: " );
      Serial.println(bank);
      
      if (volToSet < (currentVol-1) || volToSet > (currentVol+1)){
      MP3player.setVolume(volToSet, volToSet);
      currentVol=volToSet;
      Serial.print("Volume: " );
      Serial.println(volToSet);
      }
      
   if (triggerSwitch.update()) {   
    if (triggerSwitch.read() == HIGH)
    {
      
      if (digitalRead(RANDOMSWITCH) == HIGH){
      int val = analogRead(A3);
      val = map(val, 0, 1023, 1, noOfSamples);
      Serial.print("Analog read: " );
      Serial.println(val);
      trackToPlay = val;
      
      }
      else {
      trackToPlay = random(1, noOfSamples);
      Serial.print("Random number: " );
      Serial.println(trackToPlay);
      }
      if (MP3player.isPlaying())
        MP3player.stopTrack();
      
      /* Use the playTrack function to play a numbered track: */
      uint8_t result = MP3player.playTrack(trackToPlay);
      // An alternative here would be to use the
      //  playMP3(fileName) function, as long as you mapped
      //  the file names to trigger pins.
      
      if (result == 0)  // playTrack() returns 0 on success
      {
        Serial.println("Playing" );
      }
      else // Otherwise there's an error, check the code
      {
        Serial.println("could not play." );
      }
    }
  }
  // After looping through and checking trigger pins, check to
  //  see if the stopPin (A5) is triggered.
  if (stopSwitch.update()) {
  if (stopSwitch.read() == LOW)
  {
    Serial.println("Stop strack." );
    // If another track is playing, stop it.
    if (MP3player.isPlaying())
      MP3player.stopTrack();
  }
}
}
// initSD() initializes the SD card and checks for an error.
void initSD()
{
  //Initialize the SdCard.
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED)) 
    sd.initErrorHalt();
  if(!sd.chdir("/")) 
    sd.errorHalt("sd.chdir");
}

// initMP3Player() sets up all of the initialization for the
// MP3 Player Shield. It runs the begin() function, checks
// for errors, applies a patch if found, and sets the volume/
// stero mode.
void initMP3Player()
{
  uint8_t result = MP3player.begin(); // init the mp3 player shield
  if(result != 0) // check result, see readme for error codes.
  {
    // Error checking can go here!
  }
  MP3player.setVolume(volume, volume);
  MP3player.setMonoMode(monoMode);
}
