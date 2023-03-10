// playground link https://wokwi.com/projects/350950475523162706

// PROGRAM SETTINGS
unsigned long burstDurationMicrosecs = 100000;
unsigned long pauseDurationMicrosecs = 66700;
int burstFrequencyPercent = 100; // motor speed for a pair of fingers being actively stimulated (value from 0 to 100)
int restFrequencyPercent = 0; // motor speed for the fingers not being actively stimulated (value from 0 to 100)
int pauseFrequencyPercent = 0; // motor speed for all fingers during pause (value from 0 to 100)

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

const char* stateStr[] = { "OFF", "CYCLE_STARTED", "CYCLE_DONE", "STEP_STARTED", "STEP_DONE", "VIBRATION_STARTED", "VIBRATION_DONE", "PAUSE_STARTED", "PAUSE_DONE" };

State currentState = OFF;

int vibrationPatterns[33][4] = {
  // // T1
  // { 0, 0, 1, 0 },
  // { 0, 1, 0, 0 },
  // { 0, 0, 0, 1 },
  // { 1, 0, 0, 0 },
  // // T2
  // { 1, 0, 0, 0 },
  // { 0, 1, 0, 0 },
  // { 0, 0, 0, 1 },
  // { 0, 0, 1, 0 },
  // // T3
  // { 0, 1, 0, 0 },
  // { 0, 0, 0, 1 },
  // { 0, 0, 1, 0 },
  // { 1, 0, 0, 0 },
  // // T4 (OFF)
  // { 0, 0, 0, 0 },
  // { 0, 0, 0, 0 },
  // { 0, 0, 0, 0 },
  // { 0, 0, 0, 0 },
  // // T5 (OFF)
  // { 0, 0, 0, 0 },
  // { 0, 0, 0, 0 },
  // { 0, 0, 0, 0 },
  // { 0, 0, 0, 0 },
  // // T6
  // { 0, 1, 0, 0 },
  // { 0, 0, 0, 1 },
  // { 0, 0, 1, 0 },
  // { 1, 0, 0, 0 },
  // // T7
  // { 0, 1, 0, 0 },
  // { 1, 0, 0, 0 },
  // { 0, 0, 1, 0 },
  // { 0, 0, 0, 1 },
  // // T8
  // { 1, 0, 0, 0 },
  // { 0, 0, 1, 0 },
  // { 0, 0, 0, 1 },
  // { 0, 1, 0, 0 },
  // // ADDITIONAL STEP WITHOUT VIBRATION
  // { 0, 0, 0, 0 }
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
int burstFrequency = map(burstFrequencyPercent, 0, 100, 0, 255);
int restFrequency = map(restFrequencyPercent, 0, 100, 0, 255);
int pauseFrequency = map(pauseFrequencyPercent, 0, 100, 0, 255);
Timer timer = Timer();
bool development = true;
int debugLevel = 1;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 4; i++) {
    pinMode(fingerPins[i], OUTPUT);
    analogWrite(fingerPins[i], 0);
  }
}

void loop() {

  bool hasStateBeenUpdated = updateCurrentState();
  if (hasStateBeenUpdated) {
    printState(currentState)
  }

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

bool updateCurrentState () {
  State beforeUpdateState = currentState;
  if (currentState == OFF) {
    currentState = CYCLE_STARTED;
  } else if (currentState == CYCLE_DONE) {
    currentState = CYCLE_STARTED;
  } else if (currentState == CYCLE_STARTED) {
    currentState = STEP_STARTED;
  } else if (currentState == STEP_STARTED) {
    currentState = VIBRATION_STARTED;
    timer.start(burstDurationMicrosecs);
  } else if (currentState == VIBRATION_STARTED) {
    if (timer.isDone()) {
      currentState = VIBRATION_DONE;
    }
  } else if (currentState == VIBRATION_DONE) {
    currentState = PAUSE_STARTED;
    timer.start(pauseDurationMicrosecs);
  } else if (currentState == PAUSE_STARTED) {
    if (timer.isDone()) {
      currentState = PAUSE_DONE;
    }    
  } else if (currentState == PAUSE_DONE) {
    currentState = STEP_DONE;
  } else if (currentState == STEP_DONE) {
    if (currentStep < 32) {
      currentState = STEP_STARTED;
    } else {
      currentState = CYCLE_DONE;  
    }
  }
  return beforeUpdateState != currentState
}

void startVibration (int pattern[4]) {
  for (int i = 0; i < 4; i++) {
    if (pattern[i]) {
      analogWrite(fingerPins[i], burstFrequency);
    } else {
      analogWrite(fingerPins[i], restFrequency);
    }
  }
}

void stopVibration () {
  toggleFingerPins(0);
}

void startPause (int pattern[4]) {
  toggleFingerPins(pauseFrequency);
}

void stopPause () {
  toggleFingerPins(0);
} 

void toggleFingerPins (int value) {
  for (int i = 0; i < 4; i++) {
    analogWrite(fingerPins[i], value);
  }
}

// DEBUG

void printState (State state) {
  if (development == true) {
    if (debugLevel > 3) {
        Serial.print("STATE: ");
        Serial.println(stateStr[state]);
    }
  }
}