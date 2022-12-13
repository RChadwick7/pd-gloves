// PROGRAM SETTINGS
unsigned long vibrationBurstDurationMicrosecs = 100000;
unsigned long pauseBetweenBurstsDurationMicrosecs = 66700;
int burstFrequencyPercent = 100;
int restFrequencyPercent = 0;
int pauseBurstFrequencyPercent = 0;
int pauseRestFrequencyPercent = 0;
int offPeriodFrequencyPercent = 0;

// PROGRAM LOGIC
enum State {
  OFF,
  CYCLE_STARTED,
  CYCLE_DONE,
  STEP_STARTED,
  STEP_DONE,
  VIBRATION_STARTED,
  VIBRATION_DONE,
  PAUSE_STARTED,
  PAUSE_DONE
};

State currentState = OFF;

int vibrationPatterns[33][4] = {
  // T1
  { 0, 0, 1, 0 },
  { 0, 1, 0, 0 },
  { 0, 0, 0, 1 },
  { 1, 0, 0, 0 },
  // T2
  { 1, 0, 0, 0 },
  { 0, 1, 0, 0 },
  { 0, 0, 0, 1 },
  { 0, 0, 1, 0 },
  // T3
  { 0, 1, 0, 0 },
  { 0, 0, 0, 1 },
  { 0, 0, 1, 0 },
  { 1, 0, 0, 0 },
  // T4 (OFF)
  { 0, 0, 0, 0 },
  { 0, 0, 0, 0 },
  { 0, 0, 0, 0 },
  { 0, 0, 0, 0 },
  // T5 (OFF)
  { 0, 0, 0, 0 },
  { 0, 0, 0, 0 },
  { 0, 0, 0, 0 },
  { 0, 0, 0, 0 },
  // T6
  { 0, 1, 0, 0 },
  { 0, 0, 0, 1 },
  { 0, 0, 1, 0 },
  { 1, 0, 0, 0 },
  // T7
  { 0, 1, 0, 0 },
  { 1, 0, 0, 0 },
  { 0, 0, 1, 0 },
  { 0, 0, 0, 1 },
  // T8
  { 1, 0, 0, 0 },
  { 0, 0, 1, 0 },
  { 0, 0, 0, 1 },
  { 0, 1, 0, 0 },
  // SMALL PAUSE
  { 0, 0, 0, 0 }
};

class Timer {
  private:
    unsigned long startedAt;
    unsigned long endsAt;
  public:
    void start(unsigned long durationMicroseconds) {
      startedAt = micros();
      endsAt = startedAt + durationMicroseconds;
    }
    unsigned long isDone () {
      return endsAt < micros();
    }
};

// [indexFinger, middleFinger, ringFinger, littleFinger]
int fingerPins[4] = { 3, 5, 6, 9 };
int currentStep = -1;
Timer timer = Timer();

void setup() {
  randomSeed(analogRead(0));
  for (int i = 0; i < 4; i++) {
    pinMode(fingerPins[i], OUTPUT);
    digitalWrite(fingerPins[i], LOW);
  }
}

void loop() {
  updateCurrentState();
  if (currentState == CYCLE_STARTED) {
    currentStep = -1;
  } else if (currentState == STEP_STARTED) {
    currentStep += 1;
  } else if (currentState == VIBRATION_STARTED) {
    startVibration(vibrationPatterns[currentStep]);
  } else if (currentState == VIBRATION_DONE) {
    stopVibration();
  } else if (currentState == PAUSE_STARTED) {
    startPause(vibrationPatterns[currentStep]);
  } else if (currentState == PAUSE_DONE) {
    stopPause();
  }
}

void updateCurrentState () {
  if (currentState == OFF) {
    currentState = CYCLE_STARTED;
  } else if (currentState == CYCLE_DONE) {
    currentState = CYCLE_STARTED;
  } else if (currentState = CYCLE_STARTED) {
    currentState = STEP_STARTED;
  } else if (currentState = STEP_STARTED) {
    currentState = VIBRATION_STARTED;
    timer.start(vibrationBurstDurationMicrosecs);
  } else if (currentState == VIBRATION_STARTED) {
    if (timer.isDone()) {
      currentState = VIBRATION_DONE;
    }
  } else if (currentState == VIBRATION_DONE) {
    currentState = PAUSE_STARTED;
    timer.start(pauseBetweenBurstsDurationMicrosecs);
  } else if (currentState == PAUSE_STARTED) {
    if (timer.isDone()) {
      currentState = PAUSE_DONE;
    }    
  } else if (currentState == PAUSE_DONE) {
    currentState = STEP_DONE;
  } else if (currentState == STEP_DONE) {
    currentState = CYCLE_DONE;
  }
}

void startVibration (int pattern[4]) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(pattern[i], 255); // TODO MAP TO PERCENT (USE SETTINGS VAR)
  }
}

void stopVibration () {
  toggleFingerPins(0);
}

void startPause (int pattern[4]) {
  // TODO make use of program settings variables
  toggleFingerPins(0);
}

void stopPause () {
  toggleFingerPins(0);
} 

void toggleFingerPins (int value) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(fingerPins[i], value);
  }
}