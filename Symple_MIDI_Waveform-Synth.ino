/*********************************************************************************************
   partly based on JCR SYMPLE 3 OSC POLYSYNTH WITH MIDI - REV.1 - By Julio Cesar - CesarSound Dec/2020
     NOTE   23    1  B0 Note On      NOTE   23    1  B0 Note Off
     NOTE   108   1  C8 Note On      NOTE   108   1  C8 Note Off
     -----------------------------------------------------------
   NOTES:
   -Connect to MIDI OUT of your MIDI controller via 5 pin DIN MIDI connector (not USB MIDI).
**********************************************************************************************/
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <MIDI.h>  //MIDI I/Os for Arduino https://github.com/FortySevenEffects/arduino_midi_library
#include <synth.h>

#define DEBUG (true)

#include "ActiveNotes.h"
#include "Debug.h"


MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI); //Serial1 -> MIDI input at RX1 Pin
const byte polyphony = 4; // Amount of notes that can be played simultaneously

byte i = 0, j = 0, k = 0; // local variables that won't have to be declared
Notes notes;

void setup() {  
  synth::begin(CHA);

  startupTone();
  
  for (unsigned char m = 0; m < polyphony; m++){
    // preset 4 (aggressive organ)
    //synth::setupVoice(m,RAMP, 10, 60, 80, 30);
    //synth::setupVoice(m,SAW, 10, 60, 80, 30); 
    // preset 3 (pad)
    //synth::setupVoice(m,SINE, 60, 105, 127, 70); 
    // preset 2 (basic tone)
    //synth::setupVoice(m,SINE, 20, 80, 64, 50); 
    // preset 1 (basic tone)
    //synth::setupVoice(m,TRIANGLE, 7, 105, 48, 40); 

    //Test mixed waveform (LFO)
    synth::setAmpEnvParams(m, 20, 80, 64, 50);
    
    //synth::setWaveformMix(m, PIANO);
    synth::setWaveformMix(m, TRIANGLE_AND_SAW);
    //synth::setWave(m, EPIANO);
  }

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(MyHandleNoteOn);
  MIDI.setHandleNoteOff(MyHandleNoteOff);

  #if DEBUG    
    setupDebug(polyphony);
    performanceTestSession(15, 20);
  #endif
  
}

void loop() {
  MIDI.read();
  sendPacket();

  synth::updateEnvelope(0);
  sendPacket();
  
  synth::updateEnvelope(1);
  sendPacket();
  
  synth::updateEnvelope(2);
  sendPacket();
  
  synth::updateEnvelope(3);
  sendPacket();

  #if DEBUG 
    packetCounter += 5;
    if(packetCounter > 12000){
      unsigned long package_duration = millis() - package_time_start;
      Serial.println("sample_rate: " + String(float(packetCounter)*1000/package_duration));
      package_time_start = millis();
      packetCounter = 0;
    }
    //notes.debug();
  #endif
}

void MyHandleNoteOn(byte channel, byte pitch, byte velocity) {
  if(notes.length < polyphony) { // at least one voice is free or at release
    bool isAnyVoiceFree = false;
    for(k = 0; k< polyphony; k++){
      if (envelopes[k] == IDLE){
        isAnyVoiceFree = true;
        break;
      }
    }
    if(!isAnyVoiceFree){ // no voice free, but at least one is in release
      for(k = 0; k<polyphony; k++){
        if(envelopes[k] == RELEASE){
          break;
        }
      }
    }
  }
  else { 
    // no voice is free/releasing
    for (j = 0; j<notes.length; j++){
      if(notes.playerIndices[j]!=255){
          k = notes.playerIndices[j]; //choose the oldest added note and override that pin
          notes.playerIndices[j]=255;
          break;
      }
    }
  }
  // play note
  synth::mStart(k, pitch);
  notes.append(pitch, k);
}

void MyHandleNoteOff(byte channel, byte pitch, byte velocity) {  
  for(j = 0; j<notes.length; j++){
    if (notes.pitches[j] == pitch){
      k = notes.playerIndices[j];
      synth::mStop(k);
      notes.remove(j);
      j--;
    }
  }
  if(notes.length +1>= polyphony){ //If there are unplayed notes remaining, play the newest unplayed one
    for(j = notes.length-1; j !=255; j--){
      if(notes.playerIndices[j] == 255){
        notes.playerIndices[j] = k;
        synth::mStart(k, notes.pitches[j], SUSTAIN);
        break;
      }
    }
  }
}


void startupTone() {
  for(i = 0; i < polyphony; i++){
    synth::setLength(i, 70);
    synth::setPitch(i, 50+i*5);
    synth::setAmpEnvParams(i, 0, 0, 255, 90, 1);
    synth::trigger(i);
  }
}