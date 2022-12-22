// playground https://wokwi.com/projects/351723643404812885

#include "SoftPWM.h"

// SETTINGS

const unsigned long vibrationDurationMicrosecs = 100000; // 100ms
const unsigned long pauseDurationMicrosecs = 66700; // 66.7ms
const int vibrationFrequencyPercent = 100; // value from 0 to 100

// DEV

const unsigned long buttonDebounceMs = 100;
const bool development = false;
const int debugLevel = 1;

enum State {
  NULL_STATE,
  SESSION_STARTED_STATE,
  SESSION_DONE_STATE,
  CYCLE_STARTED_STATE,
  CYCLE_DONE_STATE,
  STEP_STARTED_STATE,
  STEP_DONE_STATE,
  VIBRATION_STARTED_STATE,
  VIBRATION_DONE_STATE,
  PAUSE_STARTED_STATE,
  PAUSE_DONE_STATE,
};

enum Hand {
  LEFT_HAND,
  RIGHT_HAND
};

enum MirrorMode {
  MIRROR_MODE_NORMAL,
  MIRROR_MODE_INVERTED
};

enum FingerPin {
  LEFT_INDEX_FINGER_PIN = 11,
  LEFT_MIDDLE_FINGER_PIN = 10,
  LEFT_RING_FINGER_PIN = 9,
  LEFT_LITTLE_FINGER_PIN = 8,
  RIGHT_INDEX_FINGER_PIN = 7,
  RIGHT_MIDDLE_FINGER_PIN = 6,
  RIGHT_RING_FINGER_PIN = 5,
  RIGHT_LITTLE_FINGER_PIN = 4
};

enum IndicatorPin {
  SMALL_SESSION_INDICATOR_PIN = 0,
  MEDIUM_SESSION_INDICATOR_PIN = 1,
  LARGE_SESSION_INDICATOR_PIN = 2,
  SESSION_DONE_INDICATOR_PIN = 12,
  MIRROR_MODE_INDICATOR_PIN = 13
};

enum ButtonAction {
  NULL_BUTTON_ACTION,
  RESET_BUTTON_ACTION,
  SMALL_SESSION_BUTTON_ACTION,
  MEDIUM_SESSION_BUTTON_ACTION,
  LARGE_SESSION_BUTTON_ACTION,
  MIRROR_MODE_BUTTON_ACTION
};

enum ButtonPin {
  NULL_BUTTON_PIN,
  RESET_BUTTON_PIN = 14,
  SMALL_SESSION_BUTTON_PIN = 15,
  MEDIUM_SESSION_BUTTON_PIN = 16,
  LARGE_SESSION_BUTTON_PIN = 17,
  MIRROR_MODE_BUTTON_PIN = 18
};

enum SessionIndex {
  SMALL_SESSION = 0,
  MEDIUM_SESSION = 1,
  LARGE_SESSION = 2
};

enum Reset {
  FULL_RESET,
  SESSION_RESET
};

const int fingerPins[2][4] = {
  { 
    LEFT_INDEX_FINGER_PIN,
    LEFT_MIDDLE_FINGER_PIN,
    LEFT_RING_FINGER_PIN,
    LEFT_LITTLE_FINGER_PIN
  },
  {
    RIGHT_INDEX_FINGER_PIN,
    RIGHT_MIDDLE_FINGER_PIN,
    RIGHT_RING_FINGER_PIN,
    RIGHT_LITTLE_FINGER_PIN
  }
};

enum SessionDuration {
  SMALL_SESSION_DURATION = 1000000000,
  MEDIUM_SESSION_DURATION = 2000000000,
  LARGE_SESSION_DURATION = 3000000000,
};

const ButtonPin buttonPins[5] = {
  RESET_BUTTON_PIN,
  SMALL_SESSION_BUTTON_PIN,
  MEDIUM_SESSION_BUTTON_PIN,
  LARGE_SESSION_BUTTON_PIN,
  MIRROR_MODE_BUTTON_PIN
};

struct ControlButton {
  ButtonPin pin;
  ButtonAction action;
  bool operator==(const ControlButton &other) const {
    return this->pin == other.pin && this->action == other.action;
  }
};

struct Session {
  SessionDuration duration;
  IndicatorPin pin;
  bool operator==(const Session &other) const {
    return this->duration == other.duration;
  }
};

const ControlButton controlButtons[5] = {
  {
    RESET_BUTTON_PIN,
    RESET_BUTTON_ACTION,
  },
  {
    SMALL_SESSION_BUTTON_PIN,
    SMALL_SESSION_BUTTON_ACTION
  },
  {
    MEDIUM_SESSION_BUTTON_PIN,
    MEDIUM_SESSION_BUTTON_ACTION
  },
  {
    LARGE_SESSION_BUTTON_PIN,
    LARGE_SESSION_BUTTON_ACTION
  },
  {
    MIRROR_MODE_BUTTON_PIN,
    MIRROR_MODE_BUTTON_ACTION
  }
};

const Session sessions[3] = {
  {
    SMALL_SESSION_DURATION,
    SMALL_SESSION_INDICATOR_PIN
  },
  {
    MEDIUM_SESSION_DURATION,
    MEDIUM_SESSION_INDICATOR_PIN
  },
  {
    LARGE_SESSION_DURATION,
    LARGE_SESSION_INDICATOR_PIN
  }
};


const int vibrationPatterns[33][4] = {
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
  // ADDITIONAL STEP WITHOUT VIBRATION (pause here is 166.7ms)
  { 0, 0, 0, 0 }
};

class Timer {
  private:
    unsigned long startedAt;
    unsigned long endsAt;
    unsigned long duration;
    bool hasOverflowed;
    bool isMillis;
  unsigned long getTime () {
    if (isMillis) {
      return millis();
    } else {
      return micros();
    }
  }
  public:
    Timer (bool _isMillis) {
      isMillis = _isMillis;
    }
    void start(unsigned long durationMicroseconds) {
      startedAt = getTime();
      endsAt = startedAt + durationMicroseconds;
      duration = durationMicroseconds;
      hasOverflowed = endsAt < startedAt;
    }
    unsigned long isDone () {
      if (duration == 0) {
        return false;
      }
      unsigned long now = getTime();
      if (hasOverflowed) {
        return now + duration > startedAt;
      } else {
        return now > endsAt;
      }
    }
    void reset () {
      startedAt = 0;
      endsAt = 0;
      duration = 0;
      hasOverflowed = false;
    }
};

const ControlButton nullButton = { 0, 0 };
const Session nullSession = { 0, 0 };
const int furtherResearchPin = 3;
const char* stateStr[] = { "NULL_STATE", "SESSION_STARTED_STATE", "SESSION_DONE_STATE", "CYCLE_STARTED_STATE", "CYCLE_DONE_STATE", "STEP_STARTED_STATE", "STEP_DONE_STATE", "VIBRATION_STARTED_STATE", "VIBRATION_DONE_STATE", "PAUSE_STARTED_STATE", "PAUSE_DONE_STATE" };


// RESETTABLE
int currentStep = -1;
State currentState = 0;
ControlButton pressedButton = nullButton;
ControlButton previouslyPressedButton = nullButton;
Session session = nullSession;
Timer stepTimer = Timer(false);
Timer debounceTimer = Timer(true);
Timer sessionTimer = Timer(true);
MirrorMode mirrorMode = MIRROR_MODE_NORMAL;

void setup() {
  if (development) {
    Serial.begin(9600);
  }
  // Initialize soft pwm library (https://github.com/bhagman/SoftPWM)
  SoftPWMBegin();
  initHandPins(LEFT_HAND, 0);
  initHandPins(RIGHT_HAND, 0);
  initIndicatorPins(HIGH);
  SoftPWMSet(furtherResearchPin, 0);
  initControlButtons();
  printState(currentState);
}

void loop () {
  bool hasStateBeenUpdated = update();
  if (hasStateBeenUpdated) {
    printState(currentState);
  }
}

bool update () {
  State currentStateSaved = currentState;
  ButtonAction buttonAction = getButtonAction();
  if (buttonAction) {
    switch (buttonAction) {
      case RESET_BUTTON_ACTION:
        reset(FULL_RESET);
        setMirrorMode(MIRROR_MODE_NORMAL);
        break;
      case SMALL_SESSION_BUTTON_ACTION:
        reset(SESSION_RESET);
        currentState = SESSION_STARTED_STATE;
        session = sessions[SMALL_SESSION];
        break;
      case MEDIUM_SESSION_BUTTON_ACTION:
        reset(SESSION_RESET);
        currentState = SESSION_STARTED_STATE;
        session = sessions[MEDIUM_SESSION];
        break;
      case LARGE_SESSION_BUTTON_ACTION:
        reset(SESSION_RESET);
        currentState = SESSION_STARTED_STATE;
        session = sessions[LARGE_SESSION];
        break;
      case MIRROR_MODE_BUTTON_ACTION:
        toggleMirrorMode();
        break;
    }
  } else {
    switch (currentState) {
      case NULL_STATE:
        stopVibration();
        SoftPWMSetPercent(furtherResearchPin, 0);
        setSessionTimerIndicatorPins(true);
        break;
      case SESSION_STARTED_STATE:
        currentState = CYCLE_STARTED_STATE;
        sessionTimer.start(session.duration);
        setSessionTimerIndicatorPins(true);
        setOutputPin(session.pin, LOW);
        SoftPWMSetPercent(furtherResearchPin, 100);
        setOutputPin(SESSION_DONE_INDICATOR_PIN, HIGH);
        break;
      case CYCLE_STARTED_STATE:
        currentState = STEP_STARTED_STATE;
        currentStep = -1;
        break;
      case CYCLE_DONE_STATE:
        if (sessionTimer.isDone()) {
          currentState = SESSION_DONE_STATE;
        } else {
          currentState = CYCLE_STARTED_STATE;
        }
        break;
      case SESSION_DONE_STATE:
        setOutputPin(SESSION_DONE_INDICATOR_PIN, LOW);
        break;
      case STEP_STARTED_STATE:
        currentStep += 1;
        currentState = VIBRATION_STARTED_STATE;
        stepTimer.start(vibrationDurationMicrosecs);
        break;
      case VIBRATION_STARTED_STATE:
        if (stepTimer.isDone()) {
          currentState = VIBRATION_DONE_STATE;
        }
        startVibration(vibrationPatterns[currentStep]);
        break;
      case VIBRATION_DONE_STATE:
        currentState = PAUSE_STARTED_STATE;
        stepTimer.start(pauseDurationMicrosecs);
        stopVibration();
        break;
      case PAUSE_STARTED_STATE:
        if (stepTimer.isDone()) {
          currentState = PAUSE_DONE_STATE;
        }
        break;
      case PAUSE_DONE_STATE:
        currentState = STEP_DONE_STATE;
        break;
      case STEP_DONE_STATE:
        if (currentStep < 33) {
          currentState = STEP_STARTED_STATE;
        } else {
          currentState = CYCLE_DONE_STATE;  
        }
        break;
    }
  }
  return currentStateSaved != currentState;
}

void reset (Reset reset) {
  currentState = NULL_STATE;
  currentStep = -1;
  stepTimer.reset();
  debounceTimer.reset();
  sessionTimer.reset();
  if (reset == FULL_RESET) {
    session = nullSession;
    mirrorMode = MIRROR_MODE_NORMAL;
  } else if (reset == SESSION_RESET) {
    session = nullSession;
  }
}

void toggleMirrorMode () {
  if (mirrorMode == MIRROR_MODE_NORMAL) {
    setMirrorMode(MIRROR_MODE_INVERTED);
  } else {
    setMirrorMode(MIRROR_MODE_NORMAL);
  }
}

void setMirrorMode (MirrorMode mode) {
  mirrorMode = mode;
  setOutputPin(MIRROR_MODE_INDICATOR_PIN, !mirrorMode);
}

ButtonAction getButtonAction () {
  ControlButton pressedButtonSaved = pressedButton;
  previouslyPressedButton = pressedButton;
  pressedButton = nullButton;
  for (int i = 0; i < 5; i++) {
    if (digitalRead(controlButtons[i].pin) == LOW) {
      pressedButton = controlButtons[i];
    }
  }
  if (pressedButton == nullButton) {
    return NULL_BUTTON_ACTION;
  } else if (pressedButton == pressedButtonSaved) {
    if (debounceTimer.isDone()) {
      debounceTimer.reset();
      return pressedButton.action;
    } else {
      return NULL_BUTTON_ACTION;
    }
  } else {
    debounceTimer.start(buttonDebounceMs);
    return NULL_BUTTON_ACTION;
  }
}

void initIndicatorPins (bool value) {
  initOutputPin(SESSION_DONE_INDICATOR_PIN, value);
  initOutputPin(MIRROR_MODE_INDICATOR_PIN, value);
  initOutputPin(SMALL_SESSION_INDICATOR_PIN, value);
  initOutputPin(MEDIUM_SESSION_INDICATOR_PIN, value);
  initOutputPin(LARGE_SESSION_INDICATOR_PIN, value);
}

void setSessionTimerIndicatorPins (bool highOrLow) {
  for (int i = 0; i < 3; i++) {
    setOutputPin(sessions[i].pin, highOrLow);
  }
}

void setOutputPin (int pin, bool value) {
  digitalWrite(pin, value);
}

void startVibration (int pattern[4]) {
  setHandPins(RIGHT_HAND, pattern, MIRROR_MODE_NORMAL, vibrationFrequencyPercent);
  setHandPins(LEFT_HAND, pattern, mirrorMode, vibrationFrequencyPercent);
}

void stopVibration () {
  int pattern[4] = { 0, 0, 0, 0 };
  setHandPins(LEFT_HAND, pattern, MIRROR_MODE_NORMAL, 0);
  setHandPins(RIGHT_HAND, pattern, MIRROR_MODE_NORMAL, 0);
}

void setHandPins (Hand hand, int pattern[4], MirrorMode mode, int intensity) {
  int modeAppliedPattern[4];
  applyModeToPattern(modeAppliedPattern, pattern, mode);
  for (int i = 0; i < 4; i++) {
    SoftPWMSetPercent(fingerPins[hand][i], modeAppliedPattern[i] * intensity);
  }
}

void applyModeToPattern(int * target, int * source, MirrorMode mode) {
  if (mode == MIRROR_MODE_NORMAL) {
    target[0] = source[0];
    target[1] = source[1];
    target[2] = source[2];
    target[3] = source[3];
  } else {
    target[0] = source[3];
    target[1] = source[2];
    target[2] = source[1];
    target[3] = source[0];
  }
}

// INIT

void initHandPins (Hand hand, int percent) {
  for (int i = 0; i < 4; i++) {
    SoftPWMSet(fingerPins[hand], percent);
  }
}

void initSessionTimerIndicatorPins (bool highOrLow) {
  for (int i = 0; i < 3; i++) {
    initOutputPin(sessions[i].pin, highOrLow);
  }
}
void initOutputPin (int pinNum, bool highOrLow) {
  pinMode(pinNum, OUTPUT);
  setOutputPin(pinNum, highOrLow);
}

void initControlButtons () {
  for (int i = 0; i < 5; i++) {
    pinMode(controlButtons[i].pin, INPUT_PULLUP);
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