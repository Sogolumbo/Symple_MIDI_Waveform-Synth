#ifndef debug_h
#define debug_h

#if DEBUG
  #include <synth.h>

  unsigned long packetCounter = 0;
  unsigned long package_time_start = 0;

  unsigned char lastAmp = 0;
  byte waveformMixTextIndex = 0;

  void setupDebug(byte polyphony){
    Serial.begin(921600);
    Serial.println("/");
    Serial.println("MIDI-Synth");
    Serial.println("Polyphony: " + String(polyphony));
  }


  // qsort requires you to create a sort function
  int sort_desc(const void *cmp1, const void *cmp2)
  {
    // Need to cast the void * to int *
    int a = *((int *)cmp1);
    int b = *((int *)cmp2);
    // The comparison
    return a > b ? -1 : (a < b ? 1 : 0);
    // A simpler, probably faster way:
    //return b - a;
  }

  struct PerformanceResult{
    unsigned long loopCounter;
    unsigned int interruptCounter;
  };

  PerformanceResult performanceTestRun(bool audioStop, unsigned int milliSeconds){
    unsigned long loopCounter = 1;
    unsigned long time;
    unsigned int interruptCounter;
    if(audioStop) {
      synth::suspend();
    }

    interruptCounter = tim;
    time = millis()+ milliSeconds;
    while(millis() < time){
      loop();
      loopCounter++;
    }
    interruptCounter = tim - interruptCounter;
    
    if(audioStop) {
      synth::resume();
    }
    return PerformanceResult{loopCounter, interruptCounter};
  }
  // Test Performance of the Interrupt routine
  void performanceTestSession( unsigned int milliSeconds, unsigned int runs){
    PerformanceResult testResults[runs];
    PerformanceResult referenceResults[runs];
    double usage[runs];
    int interrupts[runs];

    double testAvg = 0;
    double interruptsAvg = 0;
    int interruptsMed;
    double referenceAvg = 0;
    double usageAvg = 0;

    Serial.println("Starting performance test session (" + String(runs) + "x " + String(milliSeconds) + "ms)");
    Serial.println("Run, Synth CPU usage, (loop counts), interrupt counts");
    for(int i = 0; i<runs; i++){
      testResults[i] = performanceTestRun(false, milliSeconds);
      referenceResults[i] = performanceTestRun(true, milliSeconds);
      usage[i] = ((double)referenceResults[i].loopCounter - testResults[i].loopCounter)/referenceResults[i].loopCounter*100;
      
      interrupts[i] = testResults[i].interruptCounter;

      testAvg += testResults[i].loopCounter;
      referenceAvg += referenceResults[i].loopCounter;
      interruptsAvg += testResults[i].interruptCounter;
      usageAvg += usage[i];
      
      
      Serial.print(String(i) + ", ");
      Serial.print(String(usage[i]) + "%, (");
      Serial.print(String(testResults[i].loopCounter) + " vs. ");
      Serial.print(String(referenceResults[i].loopCounter) + "), ");
      Serial.println(String(testResults[i].interruptCounter));
    }
    testAvg /= runs;
    referenceAvg /= runs;
    interruptsAvg /= runs;
    usageAvg /= runs;
    
    qsort(interrupts, runs, sizeof(interrupts[0]), sort_desc);
    interruptsMed = interrupts[runs/2];
    
    Serial.println("Average results");
    Serial.println("Runs, Synth CPU usage, (loop counts), interrupt counts");
    Serial.print(String(runs) + ", ");
    Serial.print(String(usageAvg) + "%, (");
    Serial.print(String(testAvg) + " vs. ");
    Serial.print(String(referenceAvg) + "), ");
    Serial.println(String(interruptsAvg));
    Serial.println("Median interrupts: " +String(interruptsMed) + " Expected interrupts: "+ String(20*milliSeconds));
    Serial.println("");
  }

  /* Test performance outside the routine (copy to wherever needed)
    synth::suspend();
    unsigned long timer = micros();

    //Code
    
    timer = micros()-timer;
    synth::resume();
    Serial.println("<Performance test>: " + String(timer) + " µs");
    //For reference: At a sample rate of 20kHz one sample corresponds to 50µs.
  */

#endif
#endif