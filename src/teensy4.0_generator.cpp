#include <Arduino.h>

//New comments for the Git
// Another

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform waveform6;   //xy=134,411
AudioSynthWaveform waveform3;   //xy=136,237
AudioSynthWaveform waveform5;   //xy=136,363
AudioSynthWaveform waveform4;   //xy=137,313
AudioSynthWaveform waveform2;   //xy=138,185
AudioSynthWaveform waveform1;   //xy=139,135
AudioSynthWaveform waveform9;   //xy=139,568
AudioSynthWaveform waveform8;   //xy=140,516
AudioSynthWaveform waveform7;   //xy=141,472
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
AudioMixer4 mixer3;             //xy=583,427
AudioMixer4 mixer2;             //xy=586,331
AudioOutputI2SQuad i2s_quad1;   //xy=851,372
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
AudioControlSGTL5000 sgtl5000_1;  //xy=599,571
AudioControlSGTL5000 sgtl5000_2;  //xy=628,620
// GUItool: end automatically generated code


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


// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 11
#define SDCARD_SCK_PIN 13

//LED indicator
int LED_GREEN_PIN = 1;
int LED_RED_PIN = 2;

// Time interval check variables
unsigned long startMillis;
unsigned long currentMillis;
unsigned long silenceMillis;
const unsigned long period = 500;

unsigned long startModeChangeMillis;
unsigned long currentModeChangeMillis;
unsigned long silenceChangeMillis;
const unsigned long modeChangePeriod = 3000;
const unsigned long silenceChangePeriod = 5000;

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

uint16_t micVal = 0;
uint16_t last_micVal = 0;

float mic_test = 0;

const int delta = max_amp / 5;
const int thresh1 = delta;
const int thresh2 = thresh1 + delta;
const int thresh3 = thresh2 + delta;
const int thresh4 = thresh3 + delta;
const int thresh5 = thresh4 + delta;

float small_theta1 = 0;
float small_theta2 = 0;
float small_theta3 = 0;
float small_theta4 = 0;
float small_theta5 = 0;
float small_theta6 = 0;
float small_theta7 = 0;
float small_theta8 = 0;
float small_theta9 = 0;

float large_theta1 = 0;
float large_theta2 = 0;
float large_theta3 = 0;
float large_theta4 = 0;
float large_theta5 = 0;
float large_theta6 = 0;
float large_theta7 = 0;
float large_theta8 = 0;
float large_theta9 = 0;

long randNumber;

bool silence = true;

bool check_mic_interaction() {
  bool state = false;

  // Get analog value from the mic module
  micVal = analogRead(A8);
  amplitude = abs(micVal - basseline);

  // Loude noise
  if (amplitude > 520) {
    // Consider it's interacting at the moment
    state = true;
    Serial.print("IT'S INTERACTING!");
    Serial.print(" AMP VAL = ");
    Serial.println(amplitude);

    mic_test = amplitude * 0.1;
    //Reset mode change elaped time
    startModeChangeMillis = millis();
  } else {
    // Consider it's not interacting at the moment
    state = false;
  }
  return state;
}

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


// LED blinking functions to indica which play mode is engaging
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

// Receive Microphone values
float get_mic_val() {
  float return_val;

  micVal = analogRead(A8);  //volume1
  amplitude = abs(micVal - basseline);

  if (amplitude > thresh5) {
    return_val = amplitude;
    Serial.print("Threshold 5, AMP =  ");
    //Serial.print(amplitude, DEC);
    //Serial.print("  MIC VAL =  ");
    Serial.println(amplitude, DEC);
  } else if (amplitude > thresh4) {
    return_val = amplitude;
    Serial.print("Threshold 4, AMP =  ");
    Serial.println(amplitude, DEC);
    //Serial.print("  MIC VAL =  ");
    //Serial.println(micVal, DEC);
  } else if (amplitude > thresh3) {
    return_val = amplitude;
    Serial.print("Threshold 3, AMP =  ");
    Serial.println(amplitude, DEC);
    //Serial.print("  MIC VAL =  ");
    //Serial.println(micVal, DEC);
  } else if (amplitude > thresh2) {
    return_val = amplitude;
    Serial.print("Threshold 2, AMP =  ");
    Serial.println(amplitude, DEC);
    //Serial.print("  MIC VAL =  ");
    //Serial.println(micVal, DEC);
  } else if (amplitude > thresh1) {
    return_val = amplitude;
    Serial.print("Threshold 1, AMP =  ");
    Serial.println(amplitude, DEC);
    //Serial.print("  MIC VAL =  ");
    //Serial.println(micVal, DEC);
  } else {
    return_val = amplitude;
    //Serial.print("SENSOR WORKED BUT NOT TOO LOUD !!! ");
    //Serial.print("BELOW Threshold 1, AMP =  ");
    //Serial.print(amplitude, DEC);
    //Serial.print("  MIC VAL =  ");
    //Serial.println(micVal, DEC);
  }
  return return_val;
}

// Get switch ID val for the play mode selection
void get_switch_Id(char i2C_char) {
  // Stand by (BLE Devices nearby)
  //if (i2C_char == 'b') { //???
  //set_standby_mode(); // ???
  //}
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

void play_mode_change() {

  Serial.println("No interaction with the MIC!  Ready to change the mode!");

  // Request a value from I2C peripharal
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

void play_no_audience() {

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

//
void play_standby() {
  // Blink LED indicator
  LED_indicator_blink_standby();
  //Serial.println("PLAY STANDBY");

  // Do stuff with waveform
  // Reset values (small audience)
  small_theta1 = 0;
  small_theta2 = 0;
  small_theta3 = 0;
  small_theta4 = 0;
  small_theta5 = 0;
  small_theta6 = 0;
  small_theta7 = 0;
  small_theta8 = 0;
  small_theta9 = 0;
  // Reset values (Large audience)
  large_theta1 = 0;  //0.075
  large_theta2 = 0;  //0.050
  large_theta3 = 0;  //0.075
  large_theta4 = 0;  //0.050
  large_theta5 = 0;
  large_theta6 = 0;
  large_theta7 = 0;  //0.050
  large_theta8 = 0;
  large_theta9 = 0;

  // Get frequency value fluctuation from sine period over time
}

//
void play_small_audience() {
  // Blink LED indicator
  LED_indicator_blink_small();
  //Serial.println("PLAY SMALL AUDIENCE");

  // Reset values (Large audience)
  large_theta1 = 0;  //0.075
  large_theta2 = 0;  //0.050
  large_theta3 = 0;  //0.075
  large_theta4 = 0;  //0.050
  large_theta5 = 0;
  large_theta6 = 0;
  large_theta7 = 0;  //0.050
  large_theta8 = 0;
  large_theta9 = 0;

  for (int i = 0; i < 3; i++) {
    mixers[i]->gain(0, 0.0);  //SD CARD
    mixers[i]->gain(1, 0.0);  //SD CARD
    mixers[i]->gain(2, 0.0);
    mixers[i]->gain(3, 0.0);
  }

  AudioNoInterrupts();
  for (int i = 0; i < 9; i++) {
    waveform_envelopes[i]->noteOff();
  }
  AudioInterrupts();

  // Sine wave parameters
  small_theta1 += 0.0750;  //0.075
  small_theta2 += 0.0425;   //0.050
  small_theta3 += 0.0115;   //0.075

  small_theta4 += 0.0715;  //0.050
  small_theta5 += 0.0525;  //0.03;
  small_theta6 += 0.0215;   //0.015

  small_theta7 += 0.045;  //0.050
  small_theta8 += 0.0325;  //0.03;
  small_theta9 += 0.0290;   //0.015

  float freqVal1 = sin(small_theta1);
  float freqVal2 = sin(small_theta2);
  float freqVal3 = tan(small_theta3);
  float freqVal4 = sin(small_theta4);
  float freqVal5 = sin(small_theta5);
  float freqVal6 = tan(small_theta6);
  float freqVal7 = sin(small_theta7);
  float freqVal8 = sin(small_theta8);
  float freqVal9 = tan(small_theta9);

  randNumber = random(50);
  float coefficient = 0.25;

  float mic_test = get_mic_val();
  float val_test = 2;
  float mic_freq = mic_test * val_test;

  // Do stuff with waveform
  for (int i = 0; i < 3; i++) {
    mixers[i]->gain(0, 1.0);  // WAVEFORM
    mixers[i]->gain(1, 1.0);  // WAVEFORM
    mixers[i]->gain(2, 1.0);  // WAVEFORM
    mixers[i]->gain(3, 0.0);  // NO
  }

  AudioNoInterrupts();
  // SPEAKER 1
  waveform1.frequency(((65) * freqVal1) + ((randNumber * coefficient) * freqVal1) + (mic_freq * freqVal3));
  waveform2.frequency(((125) * freqVal2) + ((randNumber * coefficient) * freqVal2) + (mic_freq * freqVal3));  //250

  // SPEAKER 2
  waveform4.frequency(((140) * freqVal4) + ((randNumber * coefficient) * freqVal4) + (mic_freq * freqVal6));
  waveform5.frequency(((320) * freqVal5) + ((randNumber * coefficient) * freqVal5) + (mic_freq * freqVal6));  //* (amplitude * 0.1)

  // SPEAKER 3
  waveform7.frequency(((130) * freqVal6) + ((randNumber * coefficient) * freqVal7) + (mic_freq * freqVal9));  // 57 //130
  waveform8.frequency(((200) * freqVal7) + ((randNumber * coefficient) * freqVal8) + (mic_freq * freqVal9));  // 115  //200


  for (int i = 0; i < 9; i++) {
    waveform_envelopes[i]->noteOn();
  }
  AudioInterrupts();

  //silenceMillis = millis();

  /*float mic_test = get_mic_val();

  if (mic_test > 200) {


    //Serial.println("Slience !!!");
    small_theta1 = 0;
    small_theta2 = 0;
    small_theta3 = 0;
    small_theta4 = 0;
    small_theta5 = 0;
    small_theta6 = 0;
    small_theta7 = 0;
    small_theta8 = 0;
    small_theta9 = 0;

    AudioNoInterrupts();
    // SPEAKER 1
    waveform1.frequency(0);
    waveform2.frequency(0);  //250

    // SPEAKER 2
    waveform4.frequency(0);
    waveform5.frequency(0);  //* (amplitude * 0.1)

    // SPEAKER 3
    waveform7.frequency(0);  // 57 //130
    waveform8.frequency(0);  // 115  //200

    for (int i = 0; i < 9; i++) {
      waveform_envelopes[i]->noteOff();
    }
    AudioInterrupts();
  } else {

    //silenceChangeMillis = millis();
    //if (currentModeChangeMillis - silenceChangeMillis >= modeChangePeriod) {

    //}
    //Serial.println("Talking !!!");
    float val_test = 2;
    float mic_freq = mic_test * val_test;

    AudioNoInterrupts();
    // SPEAKER 1
    waveform1.frequency(((65) * freqVal1) + ((randNumber * coefficient) * freqVal1) + (mic_freq * freqVal3));
    waveform2.frequency(((125) * freqVal2) + ((randNumber * coefficient) * freqVal2) + (mic_freq * freqVal3));  //250

    // SPEAKER 2
    waveform4.frequency(((140) * freqVal4) + ((randNumber * coefficient) * freqVal4) + (mic_freq * freqVal6));
    waveform5.frequency(((320) * freqVal5) + ((randNumber * coefficient) * freqVal5) + (mic_freq * freqVal6));  //* (amplitude * 0.1)

    // SPEAKER 3
    waveform7.frequency(((130) * freqVal6) + ((randNumber * coefficient) * freqVal7) + (mic_freq * freqVal9));  // 57 //130
    waveform8.frequency(((200) * freqVal7) + ((randNumber * coefficient) * freqVal8) + (mic_freq * freqVal9));  // 115  //200


    for (int i = 0; i < 9; i++) {
      waveform_envelopes[i]->noteOn();
    }
    AudioInterrupts();
  }*/
}



void play_large_audience() {
  // Blink LED indicator
  LED_indicator_blink_large();
  //Serial.println("PLAY LARGE AUDIENCE");

  // Reset values (Small audience)
  small_theta1 = 0;
  small_theta2 = 0;
  small_theta3 = 0;
  small_theta4 = 0;
  small_theta5 = 0;
  small_theta6 = 0;
  small_theta7 = 0;
  small_theta8 = 0;
  small_theta9 = 0;

  // Reset
  for (int i = 0; i < 3; i++) {
    mixers[i]->gain(0, 0.0);  //SD CARD
    mixers[i]->gain(1, 0.0);  //SD CARD
    mixers[i]->gain(2, 0.0);
    mixers[i]->gain(3, 0.0);
  }

  AudioNoInterrupts();
  for (int i = 0; i < 9; i++) {
    waveform_envelopes[i]->noteOff();
  }
  AudioInterrupts();

  // Do stuff with waveform
  for (int i = 0; i < 3; i++) {
    mixers[i]->gain(0, 1.0);  // WAVEFORM
    mixers[i]->gain(1, 1.0);  // WAVEFORM
    mixers[i]->gain(2, 1.0);  //
    mixers[i]->gain(3, 0.0);  //
  }

  // Sine wave parameters
  large_theta1 += 0.065;  //0.06
  large_theta2 += 0.1;    //0.04
  large_theta3 += 0.33;

  large_theta4 += 0.087;  //0.055
  large_theta5 += 0.12;   //0.035
  large_theta6 += 0.41;

  large_theta7 += 0.053;
  large_theta8 += 0.15;
  large_theta9 += 0.36;

  float freqVal1 = tan(large_theta1);
  float freqVal2 = sin(large_theta2);
  float freqVal3 = sin(large_theta3);
  float freqVal4 = tan(large_theta4);
  float freqVal5 = sin(large_theta5);
  float freqVal6 = sin(large_theta6);
  float freqVal7 = tan(large_theta7);
  float freqVal8 = sin(large_theta8);
  float freqVal9 = sin(large_theta9);

  randNumber = random(150);
  float coefficient = 0.45;

  float mic_test = get_mic_val();
  float val_test = 2;
  float mic_freq = mic_test * val_test;

  AudioNoInterrupts();
  // change frequencies here
  // SPEAKER 1
  /*waveform1.frequency(((110) * freqVal1) + (mic_freq * freqVal3));
  waveform2.frequency(((250) * freqVal2) + (mic_freq * freqVal3));
  waveform3.frequency(((100) * freqVal3) + (mic_freq * freqVal3));
  //waveform1.frequency(((130) * freqVal1) + (randNumber * coefficient) * 0 + (mic_freq * freqVal1));  // 57 //130
  //waveform2.frequency(((200) * freqVal2) + (randNumber * coefficient) * 0 + (mic_freq * freqVal2));  // 115  //200
  //waveform3.frequency(((100) * freqVal3) + (randNumber * coefficient) * 0 + (mic_freq * freqVal3));  // 115  //200

  // SPEAKER 2
  waveform4.frequency(((70) * freqVal4) + (mic_freq * freqVal6));
  waveform5.frequency(((160) * freqVal5) + (mic_freq * freqVal6));  //* (amplitude * 0.1)
  waveform6.frequency(((120) * freqVal6) + (mic_freq * freqVal6));  //* (amplitude * 0.1)

  // SPEAKER 3
  //waveform5.frequency(((130) * freqVal5) + (randNumber * coefficient));  // 57 //130
  //waveform6.frequency(((200) * freqVal6) + (randNumber * coefficient));  // 115  //200

  waveform7.frequency(((65) * freqVal7) + (mic_freq * freqVal9));   // 57 //130
  waveform8.frequency(((100) * freqVal8) + (mic_freq * freqVal9));  // 115  //200
  waveform9.frequency(((10) * freqVal9) + (mic_freq * freqVal9));   // 115  //200
  */

    // SPEAKER 1
  waveform1.frequency(((65) * freqVal2) + ((randNumber * coefficient) * freqVal2) + (mic_freq * freqVal1));
  waveform2.frequency(((125) * freqVal3) + ((randNumber * coefficient) * freqVal3) + (mic_freq * freqVal1));  //250

  // SPEAKER 2
  waveform4.frequency(((140) * freqVal5) + ((randNumber * coefficient) * freqVal5) + (mic_freq * freqVal4));
  waveform5.frequency(((320) * freqVal6) + ((randNumber * coefficient) * freqVal6) + (mic_freq * freqVal4));  //* (amplitude * 0.1)

  // SPEAKER 3
  waveform7.frequency(((130) * freqVal8) + ((randNumber * coefficient) * freqVal8) + (mic_freq * freqVal7));  // 57 //130
  waveform8.frequency(((200) * freqVal9) + ((randNumber * coefficient) * freqVal9) + (mic_freq * freqVal7));  // 115  //200

  for (int i = 0; i < 9; i++) {
    waveform_envelopes[i]->noteOn();
  }

  AudioInterrupts();

  /*
  float mic_test = get_mic_val();
  if (mic_test > 200) {
    //Serial.println("Slience !!!");
    // Reset values (Large audience)
    large_theta1 = 0;  //0.075
    large_theta2 = 0;  //0.050
    large_theta3 = 0;  //0.075
    large_theta4 = 0;  //0.050
    large_theta5 = 0;
    large_theta6 = 0;
    large_theta7 = 0;  //0.050
    large_theta8 = 0;
    large_theta9 = 0;

    AudioNoInterrupts();
    // SPEAKER 1
    waveform1.frequency(0);
    waveform2.frequency(0);  //250

    // SPEAKER 2
    waveform4.frequency(0);
    waveform5.frequency(0);  //* (amplitude * 0.1)

    // SPEAKER 3
    waveform7.frequency(0);  // 57 //130
    waveform8.frequency(0);  // 115  //200

    for (int i = 0; i < 9; i++) {
      waveform_envelopes[i]->noteOn();
    }
    AudioInterrupts();
  } else {
    //Serial.println("Talking !!!");
    float val_test = 2;
    float mic_freq = mic_test * val_test;

    AudioNoInterrupts();
    // SPEAKER 1
    waveform1.frequency(((65) * freqVal1) + ((randNumber * coefficient) * freqVal1) + (mic_freq * freqVal3));
    waveform2.frequency(((125) * freqVal2) + ((randNumber * coefficient) * freqVal2) + (mic_freq * freqVal3));  //250

    // SPEAKER 2
    waveform4.frequency(((140) * freqVal4) + ((randNumber * coefficient) * freqVal4) + (mic_freq * freqVal6));
    waveform5.frequency(((320) * freqVal5) + ((randNumber * coefficient) * freqVal5) + (mic_freq * freqVal6));  //* (amplitude * 0.1)

    // SPEAKER 3
    waveform7.frequency(((130) * freqVal6) + ((randNumber * coefficient) * freqVal7) + (mic_freq * freqVal9));  // 57 //130
    waveform8.frequency(((200) * freqVal7) + ((randNumber * coefficient) * freqVal8) + (mic_freq * freqVal9));  // 115  //200


    for (int i = 0; i < 9; i++) {
      waveform_envelopes[i]->noteOn();
    }
    AudioInterrupts();
  }*/
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
}

void loop() {
  // Check elapsed time
  currentMillis = millis();
  // some flag == false;
  // Get i2C values from ESP32
  // PREVENT MODE CHANGING WHILE IT'S INTERACTING
  if (!check_mic_interaction()) {  //ONLY ALLOW MODE CHANGE WHEN NO MIC INPUT
    //CONTING ELAPSED TIME

    currentModeChangeMillis = millis();
    // AND CHANGE MODE
    if (currentModeChangeMillis - startModeChangeMillis >= modeChangePeriod) {
      play_mode_change();  //

      silence = true;
      // Reset timer
      startModeChangeMillis = currentModeChangeMillis;
    }
  } else {
    //MIC INPUT
    silence = false;

    //Slince
    small_theta1 = 0;
    small_theta2 = 0;
    small_theta3 = 0;
    small_theta4 = 0;
    small_theta5 = 0;
    small_theta6 = 0;
    small_theta7 = 0;
    small_theta8 = 0;
    small_theta9 = 0;

    // Reset values (Large audience)
    large_theta1 = 0;  //0.075
    large_theta2 = 0;  //0.050
    large_theta3 = 0;  //0.075
    large_theta4 = 0;  //0.050
    large_theta5 = 0;
    large_theta6 = 0;
    large_theta7 = 0;  //0.050
    large_theta8 = 0;
    large_theta9 = 0;

    AudioNoInterrupts();
    // SPEAKER 1
    waveform1.frequency(0);
    waveform2.frequency(0);  //250
    waveform3.frequency(0);
    // SPEAKER 2
    waveform4.frequency(0);
    waveform5.frequency(0);  //* (amplitude * 0.1)
    waveform6.frequency(0);
    // SPEAKER 3
    waveform7.frequency(0);  // 57 //130
    waveform8.frequency(0);  // 115  //200
    waveform9.frequency(0);
    for (int i = 0; i < 9; i++) {
      waveform_envelopes[i]->noteOff();
    }
    AudioInterrupts();
  }

  // Switch sound generation mode
  if (silence) {
    switch (MODE_ID) {

      // // Device activate and ready to be engaged
      case STANDBY:
        {
          //Nested switch statement
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
                //play_small_audience();
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
      // Do nothing. Only if there are no audeices for 5 min
      default:
        {
          play_no_audience();
        }
        break;
    }
  }
}