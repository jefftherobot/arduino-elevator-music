// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);
unsigned int numMP3files = 0;
char fileName[20];    //an array to hold the filename
const int pinSwitch = 2;
int stateOfSwitch = 0;
int playedOnce = 0;
long fileToPlay;
  
void setup() {
  pinMode(pinSwitch, INPUT);
	Serial.begin(9600);
	Serial.println("Elevator Music");

	if (! musicPlayer.begin()) { // initialise the music player
	 Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
	 while (1);
	}
	Serial.println(F("VS1053 found"));

	SD.begin(CARDCS);    // initialise the SD card

	printDirectory(SD.open("/"), 0);

	// Set volume for left, right channels. lower numbers == louder volume!
	musicPlayer.setVolume(20,20);

	// If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background audio playing
	musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
	//playRandFile();
  randomSeed(analogRead(0));
}

void loop() {
	stateOfSwitch = digitalRead(pinSwitch);
  //serial.println(stateOfSwitch);
  if (stateOfSwitch == HIGH) {
    // Door closed and triggered press
    if (!musicPlayer.playingMusic && !playedOnce){
      playedOnce=1;
      playerControl('r');
    }
  } else {
    // Door has opened and triggered button unpressed
    playedOnce=0;
    if (musicPlayer.playingMusic){
      musicPlayer.stopPlaying();
    }
  }
  
	delay(100);
}

/// Music controls

void playerControl(char c){
	
	if (c == 's') {
		Serial.println("Stopped");
		musicPlayer.stopPlaying();
	}

  if (c == 'r') {
    playRandFile();
  }

	if (c == 'p') {
		if (! musicPlayer.paused()) {
			Serial.println("Paused");
			musicPlayer.pausePlaying(true);
		} else { 
			Serial.println("Resumed");
			musicPlayer.pausePlaying(false);
		}
	}
}

void playRandFile(){
  int totalFiles = numMP3files + 1;
  fileToPlay = random(1, totalFiles);
  sprintf(fileName, "%d.mp3", fileToPlay);  //turn the number into a string and add extension
  Serial.println( fileName);
  musicPlayer.startPlayingFile(fileName);
  
}

/// File listing helper
void printDirectory(File dir, int numTabs) {
	while(true) {
		File entry =  dir.openNextFile();
		if (! entry) {
			// no more files
			break;
		}
		for (uint8_t i=0; i<numTabs; i++) {
			Serial.print('\t');
		}
		Serial.print(entry.name());
		if (entry.isDirectory()) {
			Serial.println("/");
			printDirectory(entry, numTabs+1);
		} else {
			// files have sizes, directories do not
			Serial.print("\t\t");
			Serial.println(entry.size(), DEC);
		}
		entry.close();
		numMP3files++;
	}
	
}
