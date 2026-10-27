
#include <Arduino.h>

// ---------- PIN CONFIGURATION ----------
const int BUTTON_PIN = 5;
const int LED_PINS[] = {16, 17, 18};
const int LED_COUNT = 3;

// ---------- MORSE ALPHABET ----------
const char* alphabetMorse[] = {
  "sl", "lsss", "lsls", "lss", "s", "ssls",
  "lls", "ssss", "ss", "slll", "lsl", "slss",
  "ll", "ls", "lll", "slls", "llsl", "sls",
  "sss", "l", "ssl", "sssl", "sll", "lssl",
  "lsll", "llss",
  "lllll", "sllll", "sslll", "sssll", "ssssl",
  "sssss", "lssss", "llsss", "lllss", "lllls",
  "slslsl", "llssll", "ssllss"
};

const char* alphabetText[] = {
  "a", "b", "c", "d", "e", "f", "g", "h",
  "i", "j", "k", "l", "m", "n", "o", "p",
  "q", "r", "s", "t", "u", "v", "w", "x",
  "y", "z",
  "0", "1", "2", "3", "4", "5", "6", "7",
  "8", "9", ".", ",", "?"
};

const int ALPHABET_SIZE =
  sizeof(alphabetMorse) / sizeof(alphabetMorse[0]);

// ---------- TIMING ----------
const unsigned long LED_TIME = 950;       // 0.95 seconds
const unsigned long DEBOUNCE_MS = 25;
const unsigned long LETTER_GAP_MS = 700;
const unsigned long DELETE_HOLD_MS = 2000;
const unsigned long RESET_HOLD_MS = 4000;

// ---------- STATE ----------
String currentChar = "";
String decodedText = "";

bool stablePressed = false;
bool lastRawPressed = false;
bool buttonWasPressed = false;

unsigned long lastDebounceTime = 0;
unsigned long pressStart = 0;
unsigned long releaseStart = 0;

bool pressHandled = false;
bool deleteDone = false;
bool resetDone = false;

// ---------- LED CONTROL ----------
void clearLEDs() {
  for (int i = 0; i < LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
}

void updateLEDs(unsigned long heldMs) {
  clearLEDs();

  // LEDs progressively light up as the button is held.
  if (heldMs > 0) {
    digitalWrite(LED_PINS[0], HIGH);
  }
  if (heldMs > LED_TIME / 3) {
    digitalWrite(LED_PINS[1], HIGH);
  }
  if (heldMs > 2 * LED_TIME / 3) {
    digitalWrite(LED_PINS[2], HIGH);
  }
}

// ---------- MORSE TRANSLATION ----------
String translateFromMorse(const String& morse) {
  for (int i = 0; i < ALPHABET_SIZE; i++) {
    if (morse == alphabetMorse[i]) {
      return String(alphabetText[i]);
    }
  }
  return "";
}

bool isPossibleMorse(const String& code) {
  for (int i = 0; i < ALPHABET_SIZE; i++) {
    String candidate = alphabetMorse[i];

    if (candidate.startsWith(code)) {
      return true;
    }
  }
  return false;
}

// ---------- LETTER HANDLING ----------
void acceptLetter() {
  if (currentChar.length() == 0) return;

  String letter = translateFromMorse(currentChar);

  if (letter.length() > 0) {
    decodedText += letter;
    Serial.print("Text: ");
    Serial.println(decodedText);
  } else {
    Serial.print("Unknown Morse code: ");
    Serial.println(currentChar);
  }

  currentChar = "";
}

void deleteLast() {
  if (currentChar.length() > 0) {
    currentChar = "";
    Serial.println("Current letter cleared");
  } else if (decodedText.length() > 0) {
    decodedText.remove(decodedText.length() - 1);
    Serial.print("Text after deletion: ");
    Serial.println(decodedText);
  }
}

void resetWord() {
  currentChar = "";
  decodedText = "";
  Serial.println("Word reset");
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  for (int i = 0; i < LED_COUNT; i++) {
    pinMode(LED_PINS[i], OUTPUT);
  }

  clearLEDs();

  Serial.println("Morse translator ready");
  Serial.println("Short press = dot; long press = dash");
}

// ---------- MAIN LOOP ----------
void loop() {
  unsigned long now = millis();

  // Button is wired between GPIO 5 and GND.
  bool rawPressed = (digitalRead(BUTTON_PIN) == LOW);

  // Debounce
  if (rawPressed != lastRawPressed) {
    lastDebounceTime = now;
    lastRawPressed = rawPressed;
  }

  if (now - lastDebounceTime >= DEBOUNCE_MS) {
    stablePressed = rawPressed;
  }

  // Button pressed
  if (stablePressed && !buttonWasPressed) {
    buttonWasPressed = true;
    pressStart = now;
    pressHandled = false;
    deleteDone = false;
    resetDone = false;
    releaseStart = 0;
  }

  // Button held
  if (stablePressed) {
    unsigned long heldMs = now - pressStart;
    updateLEDs(heldMs);

    // Hold for 4 seconds to reset everything
    if (heldMs >= RESET_HOLD_MS && !resetDone) {
      resetWord();
      resetDone = true;
      pressHandled = true;
    }
    // Hold for 2 seconds to delete
    else if (heldMs >= DELETE_HOLD_MS && !deleteDone) {
      deleteLast();
      deleteDone = true;
      pressHandled = true;
    }

    return;
  }

  // Button released
  if (buttonWasPressed) {
    unsigned long heldMs = now - pressStart;
    buttonWasPressed = false;
    clearLEDs();

    if (!pressHandled) {
      // A short press is a dot; a long press is a dash.
      if (heldMs < LED_TIME) {
        currentChar += "s";
      } else {
        currentChar += "l";
      }

      if (!isPossibleMorse(currentChar)) {
        Serial.println("Invalid Morse sequence; cleared");
        currentChar = "";
      } else {
        Serial.print("Current code: ");
        Serial.println(currentChar);
      }
    }

    releaseStart = now;
  }

  // Finish a letter after a pause
  if (currentChar.length() > 0 &&
      releaseStart > 0 &&
      now - releaseStart >= LETTER_GAP_MS) {
    acceptLetter();
    releaseStart = 0;
  }

  delay(5);
}
