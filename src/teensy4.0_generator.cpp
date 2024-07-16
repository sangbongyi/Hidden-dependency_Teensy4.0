/* 
  Hidden dependency project
  Copyright (c) 2024 Sangbong Lee <sangbong@me.com>
  
  * Frequency generator for 3 speakers connected to a Teensy 4.0 board with 2 Audio-shield rev D
  * This code allows the Teensy board 4.0 to generate certain frequencies for 3 speakers to draw laser patterns on the wall.
  * It also has functions to interact between audiences and the device with a microphone module.

  This work is licensed under the Creative Commons Attribution 4.0 International License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/.
*/

#include <Arduino.h>

// Libraries
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 11
#define SDCARD_SCK_PIN 13

// GUItool: begin automatically generated code
AudioSynthWaveform waveform1;   
AudioSynthWaveform waveform2;   
AudioSynthWaveform waveform3;   
AudioSynthWaveform waveform4;   
AudioSynthWaveform waveform5;   
AudioSynthWaveform waveform6;   
AudioSynthWaveform waveform7;   
AudioSynthWaveform waveform8;   
AudioSynthWaveform waveform9;   

AudioEffectEnvelope envelope1;  //xy=313,134
AudioEffectEnvelope envelope2;  //xy=314,184
AudioEffectEnvelope envelope3;  //xy=315,236
AudioEffectEnvelope envelope5;  //xy=316,360
AudioEffectEnvelope envelope4;  //xy=317,309
AudioEffectEnvelope envelope6;  //xy=317,412
AudioEffectEnvelope envelope7;  //xy=331,471
AudioEffectEnvelope envelope8;  //xy=335,531
AudioEffectEnvelope envelope9;  //xy=340,578

AudioMixer4 mixer1;             //xy=583,223
AudioMixer4 mixer2;             //xy=586,331
AudioMixer4 mixer3;             //xy=583,427

AudioOutputI2SQuad i2s_quad1;     //xy=851,372
AudioControlSGTL5000 sgtl5000_1;  //xy=599,571
AudioControlSGTL5000 sgtl5000_2;  //xy=628,620

AudioConnection patchCord1(waveform6, envelope6);
AudioConnection patchCord2(waveform3, envelope3);
AudioConnection patchCord3(waveform5, envelope5);
AudioConnection patchCord4(waveform4, envelope4);
AudioConnection patchCord5(waveform2, envelope2);
AudioConnection patchCord6(waveform1, envelope1);
AudioConnection patchCord7(waveform9, envelope9);
AudioConnection patchCord8(waveform8, envelope8);
AudioConnection patchCord9(waveform7, envelope7);
AudioConnection patchCord10(envelope1, 0, mixer1, 0);
AudioConnection patchCord11(envelope2, 0, mixer1, 1);
AudioConnection patchCord12(envelope3, 0, mixer1, 2);
AudioConnection patchCord13(envelope5, 0, mixer2, 1);
AudioConnection patchCord14(envelope4, 0, mixer2, 0);
AudioConnection patchCord15(envelope6, 0, mixer2, 2);
AudioConnection patchCord16(envelope7, 0, mixer3, 0);
AudioConnection patchCord17(envelope8, 0, mixer3, 1);
AudioConnection patchCord18(envelope9, 0, mixer3, 2);
AudioConnection patchCord19(mixer1, 0, i2s_quad1, 0);
AudioConnection patchCord20(mixer3, 0, i2s_quad1, 2);
AudioConnection patchCord21(mixer2, 0, i2s_quad1, 1);
// GUItool: end automatically generated code

// Arrays for Audio effect objects
AudioSynthWaveform *waveForm[9] = {
  &waveform1,
  &waveform2,
  &waveform3,
  &waveform4,
  &waveform5,
  &waveform6,
  &waveform7,
  &waveform8,
  &waveform9,
};

AudioEffectEnvelope *waveform_envelopes[9] = {
  &envelope1,
  &envelope2,
  &envelope3,
  &envelope4,
  &envelope5,
  &envelope6,
  &envelope7,
  &envelope8,
  &envelope9,
};

AudioMixer4 *mixers[3] = {
  &mixer1,
  &mixer2,
  &mixer3,
};

//LED indicator
int LED_GREEN_PIN = 1;
int LED_RED_PIN = 2;

// Time interval check variables
// For I2C communication
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 500;

// For MIC interaction
unsigned long startMicInteractionMillis;
unsigned long currentModeChangeMillis;
const unsigned long modeChangePeriod = 3000;

//NEW FLAG for TESTING
int MODE_ID = 0;
int PLAY_MODE_ID = 0;
enum mode {
  DEFAULT,
  STANDBY,
  SMALL,
  LARGE
};

// Wave file play counter
int wavefile_play_counter = 0;

// Potentiometer variables
float a1history = 0, a2history = 0, a3history = 0, a4history = 0;

// Microphone variables
const int basseline = 530;
int amplitude = 0;
const int max_amp = 495;
uint16_t sensor_val = 0;

float mic_value = 0;
const int delta = max_amp / 5;
int threshold[5];

// Wave theta for frequency generating
float small_audience_theta[9];
float large_audience_theta[9];

long randNumber;
bool silence = true;

//********** Initialize variables **********//
// Initialize microphone delta value
void init_threshold_val() {
  int mic_val_delta = max_amp / 5;

  for (int i = 0; i< 5; i++) {
    threshold[i] = i * mic_val_delta;
  }
}
// Initialize wave theta values for the frequency generating
void init_theta() {
  for (int i = 0; i < 9; i++) {
    small_audience_theta[i] = 0;
    large_audience_theta[i] = 0;
  }
}

//********** LED Functions **********//
// LED blinking functions to indicate that which play mode is engaging
void LED_indicator_blink_standby() {
  digitalWrite(LED_GREEN_PIN, HIGH);
  delay(25);
  digitalWrite(LED_GREEN_PIN, LOW);
  delay(25);
}
void LED_indicator_blink_small() {
  digitalWrite(LED_RED_PIN, HIGH);
  delay(25);
  digitalWrite(LED_RED_PIN, LOW);
  delay(25);
}
void LED_indicator_blink_large() {
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_RED_PIN, HIGH);
  delay(25);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);
  delay(25);
}

//********** Microphone checking functions **********//
// Check if any microphone inputs while the loop function
bool check_mic_interaction() {
  bool state = false;

  // Get analog value from the mic module
  sensor_val = analogRead(A8);
  amplitude = abs(sensor_val - basseline);

  // Loud noise
  if (amplitude > 520) {
    // Consider it's interacting at the moment
    state = true;
    Serial.print("IT'S INTERACTING!");
    Serial.print(" AMP VAL = ");
    Serial.println(amplitude);

    mic_value = amplitude * 0.1;
    
    // Reset elapsed time for the mode change 
    startMicInteractionMillis = millis();
  } else {
    // Consider it's not interacting at the moment
    state = false;
  }
  return state;
}

// Receive Microphone values
float get_mic_val() {
  float return_val;

  // Get analog value from the mic module
  sensor_val = analogRead(A8);  //volume1
  amplitude = abs(sensor_val - basseline);

  // Print mic amplitude value
  if (amplitude > threshold[4]) {
    return_val = amplitude;
    Serial.print("Threshold 5, AMP =  ");
    Serial.println(amplitude, DEC);
  } else if (amplitude > threshold[3]) {
    return_val = amplitude;
    Serial.print("Threshold 4, AMP =  ");
    Serial.println(amplitude, DEC);
  } else if (amplitude > threshold[2]) {
    return_val = amplitude;
    Serial.print("Threshold 3, AMP =  ");
    Serial.println(amplitude, DEC);
  } else if (amplitude > threshold[1]) {
    return_val = amplitude;
    Serial.print("Threshold 2, AMP =  ");
    Serial.println(amplitude, DEC);
  } else if (amplitude > threshold[0]) {
    return_val = amplitude;
    Serial.print("Threshold 1, AMP =  ");
    Serial.println(amplitude, DEC);
  } else {
    return_val = amplitude;
    //Serial.print("SENSOR WORKED BUT NOT TOO LOUD !!! ");
  }
  return return_val;
}

//********** Mode change functions **********//
// Play mode select functions
void set_no_audience_mode() {
  MODE_ID = DEFAULT;
  PLAY_MODE_ID = DEFAULT;
  Serial.println("SET MODE & PLAY MODE NO AUDIENCE");
}
void set_standby_mode() {
  MODE_ID = STANDBY;
  Serial.println("SET MODE STANDBY");
}
void set_small_audience_mode() {
  set_standby_mode();
  PLAY_MODE_ID = SMALL;
  Serial.println("SET PLAY_MODE SMALL AUDIENCE");
  Serial.println();
}
void set_large_audience_mode() {
  set_standby_mode();
  PLAY_MODE_ID = LARGE;
  Serial.println("SET PLAY_MODE LARGE AUDIENCE");
  Serial.println();
}

// Get switch ID val for the play mode selection
void get_switch_Id(char i2C_char) {
  // Small number of audiences
  Serial.print("STAGE : ");
  if (i2C_char == 'f') {
    set_small_audience_mode();
  }
  // Large number of audiences
  else if (i2C_char == 'r') {
    set_large_audience_mode();
  }
  // No audiences at the moment
  else {
    set_no_audience_mode();
  }
}

// Play mode(Frequency generating mode) change functions
void get_modeChange_command() {
  // No MIC inputs => Change mode
  Serial.println("No interaction with the MIC!  Ready to change the mode!");

  // Request a command from I2C peripharal
  if (currentMillis - startMillis >= period) {

    // request 1 bytes from peripheral device #8
    Wire.requestFrom(8, 1);

    // Peripheral may send less than requested
    while (Wire.available()) {
      // receive a byte as character
      char c = Wire.read();
      get_switch_Id(c);
    }

    // Reset timer
    startMillis = currentMillis;
  }
}

//********** Mode(Frequency generating) functions **********//
// Frequency generating (no BLE devices in the scanning area) => turn off all the mixers
void play_sleepMode() {

  // Turn off Mixer gain
  for (int i = 0; i < 3; i++) {
    mixers[i]->gain(0, 0.0);
    mixers[i]->gain(1, 0.0);
    mixers[i]->gain(2, 0.0);
    mixers[i]->gain(3, 0.0);
  }

  // Turn off speakers
  AudioNoInterrupts();
  for (int i = 0; i < 6; i++) {
    waveform_envelopes[i]->noteOff();
  }
  AudioInterrupts();
}

// Frequency generating (BLE devices are found in the scanning area) => Initialize all the variables
void play_standby() {
  // Blink LED indicator
  LED_indicator_blink_standby();
  init_theta();
}

// Frequency generating (BLE devices are found in the scanning area) => Initialize all the variables
void play_small_audience() {
  // Blink LED indicator
  LED_indicator_blink_small();

  // Reset values (Wave theta variables for the small audience mode)
  for (int i = 0; i < 9; i++) {
    large_audience_theta[i] = 0;
  }

  // Reset mixers
  for (int i = 0; i < 3; i++) {
    mixers[i]->gain(0, 0.0);  //SD CARD
    mixers[i]->gain(1, 0.0);  //SD CARD
    mixers[i]->gain(2, 0.0);
    mixers[i]->gain(3, 0.0);
  }

  // Reset envelopes
  AudioNoInterrupts();
  for (int i = 0; i < 9; i++) {
    waveform_envelopes[i]->noteOff();
  }
  AudioInterrupts();

  // Set wave theta increments
  small_audience_theta[0] += 0.0750;  //0.075
  small_audience_theta[1] += 0.0425;   //0.050
  small_audience_theta[2] += 0.0115;   //0.075
  small_audience_theta[3] += 0.0715;  //0.050
  small_audience_theta[4] += 0.0525;  //0.03;
  small_audience_theta[5] += 0.0215;   //0.015
  small_audience_theta[6] += 0.045;  //0.050
  small_audience_theta[7] += 0.0325;  //0.03;
  small_audience_theta[8] += 0.0290;   //0.015

  // Set frequency values
  float freqVals[9];
  for (int i = 0; i < 9; i += 3) {
    freqVals[i]   = sin(small_audience_theta[i]);
    freqVals[i+1] = sin(small_audience_theta[i+1]);
    freqVals[i+2] = tan(small_audience_theta[i+2]);
  }

  randNumber = random(50);
  float coefficient = 0.25;
  float mic_value = get_mic_val();
  float val_test = 2;
  float mic_freq = mic_value * val_test;

  // Do stuff with waveform
  for (int i = 0; i < 3; i++) {
    mixers[i]->gain(0, 1.0);  // WAVEFORM
    mixers[i]->gain(1, 1.0);  // WAVEFORM
    mixers[i]->gain(2, 1.0);  // WAVEFORM
    mixers[i]->gain(3, 0.0);  // NO
  }

  AudioNoInterrupts();
  // SPEAKER 1
  waveform1.frequency(((65) * freqVals[0]) + ((randNumber * coefficient) * freqVals[0]) + (mic_freq * freqVals[2]));
  waveform2.frequency(((125) * freqVals[1]) + ((randNumber * coefficient) * freqVals[1]) + (mic_freq * freqVals[2]));

  // SPEAKER 2
  waveform4.frequency(((140) * freqVals[3]) + ((randNumber * coefficient) * freqVals[3]) + (mic_freq * freqVals[5]));
  waveform5.frequency(((320) * freqVals[4]) + ((randNumber * coefficient) * freqVals[4]) + (mic_freq * freqVals[5]));  

  // SPEAKER 3
  waveform7.frequency(((130) * freqVals[5]) + ((randNumber * coefficient) * freqVals[6]) + (mic_freq * freqVals[8]));
  waveform8.frequency(((200) * freqVals[6]) + ((randNumber * coefficient) * freqVals[7]) + (mic_freq * freqVals[8]));


  for (int i = 0; i < 9; i++) {
    waveform_envelopes[i]->noteOn();
  }
  AudioInterrupts();
}


void play_large_audience() {
  // Blink LED indicator
  LED_indicator_blink_large();

  // Reset values (Wave theta variables for the large audience mode)
  for (int i = 0; i < 9; i++) {
    small_audience_theta[i] = 0;
  }

  // Reset mixers
  for (int i = 0; i < 3; i++) {
    mixers[i]->gain(0, 0.0);
    mixers[i]->gain(1, 0.0); 
    mixers[i]->gain(2, 0.0);
    mixers[i]->gain(3, 0.0);
  }
  
  // Reset envelopes
  AudioNoInterrupts();
  for (int i = 0; i < 9; i++) {
    waveform_envelopes[i]->noteOff();
  }
  AudioInterrupts();

  // Set wave theta increments
  large_audience_theta[0] += 0.065;  //0.06
  large_audience_theta[1] += 0.1;    //0.04
  large_audience_theta[2] += 0.33;
  large_audience_theta[3] += 0.087;  //0.055
  large_audience_theta[4] += 0.12;   //0.035
  large_audience_theta[5] += 0.41;
  large_audience_theta[6] += 0.053;
  large_audience_theta[7] += 0.15;
  large_audience_theta[8] += 0.36;

  // Set frequency values
  float freqVals[9];
  for (int i = 0; i < 9; i += 3) {
    freqVals[i]   = sin(large_audience_theta[i]);
    freqVals[i+1] = sin(large_audience_theta[i+1]);
    freqVals[i+2] = tan(large_audience_theta[i+2]);
  }

  randNumber = random(150);
  float coefficient = 0.45;
  float mic_value = get_mic_val();
  float val_test = 2;
  float mic_freq = mic_value * val_test;

  // Do stuff with waveform
  for (int i = 0; i < 3; i++) {
    mixers[i]->gain(0, 1.0);  // WAVEFORM
    mixers[i]->gain(1, 1.0);  // WAVEFORM
    mixers[i]->gain(2, 1.0);  //
    mixers[i]->gain(3, 0.0);  //
  }

  AudioNoInterrupts();
  // SPEAKER 1
  waveform1.frequency(((65) * freqVals[1]) + ((randNumber * coefficient) * freqVals[1]) + (mic_freq * freqVals[0]));
  waveform2.frequency(((125) * freqVals[2]) + ((randNumber * coefficient) * freqVals[2]) + (mic_freq * freqVals[0]));

  // SPEAKER 2
  waveform4.frequency(((140) * freqVals[4]) + ((randNumber * coefficient) * freqVals[4]) + (mic_freq * freqVals[3]));
  waveform5.frequency(((320) * freqVals[5]) + ((randNumber * coefficient) * freqVals[5]) + (mic_freq * freqVals[3]));

  // SPEAKER 3
  waveform7.frequency(((130) * freqVals[7]) + ((randNumber * coefficient) * freqVals[7]) + (mic_freq * freqVals[6]));
  waveform8.frequency(((200) * freqVals[8]) + ((randNumber * coefficient) * freqVals[8]) + (mic_freq * freqVals[6]));

  for (int i = 0; i < 9; i++) {
    waveform_envelopes[i]->noteOn();
  }
  AudioInterrupts();
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  // put your setup code here, to run once:
  // Initialize Serial & i2C
  Serial.begin(115200);
  Wire.begin();

  Serial.println("SYSTEM STARTED");

  AudioMemory(50);
  sgtl5000_1.setAddress(LOW);
  sgtl5000_1.enable();
  sgtl5000_1.volume(1.0);

  sgtl5000_2.setAddress(HIGH);
  sgtl5000_2.enable();
  sgtl5000_2.volume(1.0);

  // LED Indicator pin setup
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);

  // SD Card check
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  Serial.println("SD card confirmed!");
  Serial.println();
  delay(500);

  // Teensy Audio setup
  // Mixer 1 - 4 (Ch_1,2 Play SD/ Sine waveform)
  for (int i = 0; i < 3; i++) {
    mixers[i]->gain(0, 1.0);
    mixers[i]->gain(1, 1.0);
    mixers[i]->gain(2, 0.0);
    mixers[i]->gain(3, 0.0);
  }

  // Waveform setup
  for (int i = 0; i < 9; i++) {
    waveForm[i]->begin(WAVEFORM_SINE);
    waveForm[i]->frequency(0);
    waveForm[i]->amplitude(1.0);
    waveForm[i]->pulseWidth(0.10);
  }

  // Envelopes for waveforms
  for (int i = 0; i < 9; i++) {
    waveform_envelopes[i]->attack(10.5);
    waveform_envelopes[i]->hold(1);
    waveform_envelopes[i]->decay(35);
    waveform_envelopes[i]->sustain(1.0);
    waveform_envelopes[i]->release(10);
  }

  //initial start time`
  startMillis = millis();
  randomSeed(analogRead(A8));

  init_theta();
}

void loop() {
  ////********** SET AN OFFSET TIME DURATION WHEN THERE IS A MODE CHANGE **********////
  // Check elapsed time
  currentMillis = millis();

  // PREVENT MODE CHANGING RIGHT AFTER THE MIC INTERACTION
  if (!check_mic_interaction()) {  //IT ONLY ALLOWS THE MODE CHANGE WHEN NO MIC INPUT
    
    // COUNTING ELAPSED TIME SINCE NO MIC INPUTS
    currentModeChangeMillis = millis();
    
    // AFTER A PERIOD OF TIME
    if (currentModeChangeMillis - startMicInteractionMillis >= modeChangePeriod) {
      get_modeChange_command();  // Get mode change command from I2C 

      silence = true;
      
      // Reset timer
      startMicInteractionMillis = currentModeChangeMillis;
    }
  } else { // DO NOTING WHEN THE MIC INPUT EXISTS
    
    silence = false;

    init_theta();

    // Stop generating waveform
    AudioNoInterrupts();
    for (int i = 0; i < 9; i++) {
      waveForm[i]->frequency(0);
      waveform_envelopes[i]->noteOff();
    }
    AudioInterrupts();
  }
  ////********************************////

  ////********** Switch modes when it doesn't have any interaction with the mic **********////
  // Switch sound generation mode
  if (silence) {
    switch (MODE_ID) {
      // Activate the device and ready to be engaged // ACTIVE
      case STANDBY:
        {
          // Nested switch statement
          // case 1 : small audience -> if (Mic value > something) -> play // else same as the stand by
          // case 2 : large audince  -> if (Mic value > something) -> play // else same as the stand by
          // case default : just play stanby sequence
          switch (PLAY_MODE_ID) {
            // Device starts to activate with the small number of audiences
            case SMALL:
              {
                play_small_audience();
              }
              break;

            // Device starts to activate with the large number of audiences
            case LARGE:
              {
                play_large_audience();
              }
              break;

            default:
              {
                play_standby();
              }
              break;
          }
        }
        break;
      
      // Do nothing. Only if there are no audeices for 5 min (Sleep mode?)
      default:
        {
          play_sleepMode();
        }
        break;
    }
  }
}