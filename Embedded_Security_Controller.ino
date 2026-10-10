#include <SHA256.h>

//GLOBAL VARIABLES

//Pin Assignments
const int buttonPin = 7;
const int ledPin = 13;
const int greenPin = 4;
const int redPin = 2;

//Authentication
const byte storedPasswordHash[32] = {
  0x9d, 0xad, 0xc8, 0x8d,
  0xb1, 0x16, 0xa8, 0x4e,
  0xc0, 0xb5, 0x85, 0xae,
  0x15, 0x30, 0xc4, 0x6d,
  0x7f, 0x1e, 0xcb, 0xf8,
  0x9f, 0x97, 0xb3, 0x9d,
  0x75, 0xbf, 0x60, 0x89,
  0x91, 0x92, 0x59, 0xe0
};
bool authenticated = false;

//Login Failures
const byte MAX_LOGIN_ATTEMPTS = 3;
byte failedLoginAttempts = 0;
bool loginLocked = false;
unsigned long lockoutStartTime = 0;
const unsigned long LOCKOUT_DURATION = 10000;

//Button State
int lastButtonState = HIGH;

//Command Buffer
const byte MAX_COMMAND_LENGTH = 40;
char commandBuffer[MAX_COMMAND_LENGTH];
byte commandIndex = 0;

//State Machine
bool switchActive = false;
bool switchLedState = false;
byte switchCount = 0;
unsigned long switchPreviousTime = 0;
const unsigned long SWITCH_INTERVAL = 1000;

//Hash
SHA256 hash;


//FUNCTIONS

void hashPassword(const char* password, byte* outputHash) {

  hash.reset();

  hash.update(password,strlen(password));

  hash.finalize(outputHash, 32);
}


void updateSwitch() {

  if (switchActive == true) {

    if (millis() - switchPreviousTime >= SWITCH_INTERVAL) {

      switchPreviousTime = millis();

      if (switchLedState == false) {

        digitalWrite(ledPin, HIGH);

        switchLedState = true;
      }

      else {

        digitalWrite(ledPin, LOW);

        switchLedState = false;

        switchCount++;

        if (switchCount >= 3) {

          switchActive = false;

          switchCount = 0;

          Serial.println("SWITCH executed.");
        }
      }
    }
  }
}


void Read(){

  char incomingChar = Serial.read();

  if (incomingChar == '\n') {

    commandBuffer[commandIndex] = '\0';

    processCommand(commandBuffer);

    commandIndex = 0;
  }

  else if (incomingChar != '\r') {

    if (commandIndex < MAX_COMMAND_LENGTH - 1) {

      commandBuffer[commandIndex] = incomingChar;

      commandIndex++;
    }

    else {

      Serial.println("ERROR: Command too long");

      commandIndex = 0;
    }
  }

}

void checkLoginLockout() {

  if (loginLocked == true) {

    if (millis() - lockoutStartTime >= LOCKOUT_DURATION) {

      loginLocked = false;

      failedLoginAttempts = 0;

      Serial.println("Login lockout expired.");
      Serial.println();
    }
  }
}


void checkButton() {

  int currentButtonState = digitalRead(buttonPin);

  if (lastButtonState == HIGH && currentButtonState == LOW) {

    Serial.println("Physical override activated.");
    Serial.println("Access Granted.");
    Serial.println();
    authenticated = true;
  }

  lastButtonState = currentButtonState;
}


void AuthIndicator() {

  if (authenticated == true) {
    digitalWrite(greenPin, HIGH);
  }
  
  else {
    digitalWrite(greenPin, LOW);
  }

}


void Denied(bool startDenied = false) {

  static bool deniedActive = false;
  static unsigned long deniedStartTime = 0;

  const unsigned long DENIED_DURATION = 1000;

  if (startDenied == true) {

    Serial.println("ACCESS DENIED.");

    digitalWrite(redPin, HIGH);

    deniedActive = true;

    deniedStartTime = millis();
  }

  if (deniedActive == true) {

    if (millis() - deniedStartTime >= DENIED_DURATION) {

      digitalWrite(redPin, LOW);

      deniedActive = false;
    }
  }
}


void processCommand(const char* command) {

  if (strncmp(command, "LOGIN ", 6) == 0) {

    if (loginLocked == true) {

      Serial.println("LOGIN LOCKED.");
      Serial.println("Try again later.");
      Serial.println();

      return;
    }

    const char* password = command + 6;
    byte enteredPasswordHash[32];
    hashPassword(password, enteredPasswordHash);

    if (memcmp(enteredPasswordHash, storedPasswordHash, 32) == 0) {

      authenticated = true;

      failedLoginAttempts = 0;

      Serial.println("Authentication successful.");
      Serial.println();
    }

    else {

      authenticated = false;

      failedLoginAttempts++;

      Serial.println("Authentication failed.");
      Serial.println();

      Serial.print("Failed attempts: ");
      Serial.println(failedLoginAttempts);
      Serial.println();

      Denied(true);

      if (failedLoginAttempts >= MAX_LOGIN_ATTEMPTS) {

        loginLocked = true;

        lockoutStartTime = millis();

        Serial.println("Too many failed login attempts.");
        Serial.println("Login temporarily locked.");
        Serial.println();
      }
    }
  }

  else if (strcmp(command, "ON") == 0) {

    if (authenticated == true) {

      if (switchActive == true) {
        Serial.println("BUSY: SWITCH active.");
      }
      
      else{
        digitalWrite(ledPin, HIGH);
        Serial.println("LED turned ON.");
      }
    }

    else {

      Denied(true);
    }

    Serial.println();

  }

  else if (strcmp(command, "OFF") == 0) {

    if (authenticated == true) {

      if (switchActive == true) {

        
        Serial.println("BUSY: SWITCH active.");
      }
      
      else{
        digitalWrite(ledPin, LOW);

        Serial.println("LED turned OFF.");
      }
    }

    else {

      Denied(true);
    }

    Serial.println();

  }

  else if (strcmp(command, "STATUS") == 0) {

    Serial.println("<<STATUS>>");
    Serial.print("Authenticated: ");

    if (authenticated == true) {

      Serial.println("YES");
    }

    else {

      Serial.println("NO");
    }

    Serial.print("LED: ");

    if (switchActive == true){

      Serial.println("SWITCH");

    }

    else{

      if (digitalRead(ledPin) == HIGH) {

        Serial.println("ON");
      }

      else {

        Serial.println("OFF");
      }
    }
    Serial.println();
  }

  else if (strcmp(command, "LOGOUT") == 0) {

    authenticated = false;

    Serial.println("Logged out.");
    Serial.println();

    for (int i = 2; i > 0; i--) {

      digitalWrite(greenPin, LOW);
      digitalWrite(redPin, HIGH);
      delay(500);

      digitalWrite(greenPin, HIGH);
      digitalWrite(redPin, LOW);
      delay(500);
    }

    digitalWrite(greenPin, HIGH);
    digitalWrite(redPin, HIGH);
    delay(500);

    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, LOW);
  }

  else if (strcmp(command, "SWITCH") == 0) {

    if (authenticated == true) {

      if (switchActive == false) {

        Serial.println("LED: SWITCH");

        switchActive = true;

        switchLedState = false;

        switchCount = 0;

        switchPreviousTime = millis();
      }

      else {

        Serial.println("SWITCH already active.");
      }
    }

    else {

      Denied(true);
    }
    Serial.println();

  }

  else {

    Serial.println("Unknown command.");
    Serial.println();

    digitalWrite(greenPin, HIGH);
    digitalWrite(redPin, HIGH);

    delay(1000);

    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, LOW);
  }
}


//SETUP & LOOP

void setup() {

  Serial.begin(9600);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);

  digitalWrite(ledPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, LOW);

  Serial.println();
  Serial.println("=== Embedded Security Controller ===");
  Serial.println("Commands:");
  Serial.println("LOGIN <password>");
  Serial.println("ON");
  Serial.println("OFF");
  Serial.println("SWITCH");
  Serial.println("STATUS");
  Serial.println("LOGOUT");
  Serial.println("");
}

void loop() {

  checkButton();

  checkLoginLockout();

  Denied();

  updateSwitch();

  if (Serial.available() > 0) {

    Read();

  }

  AuthIndicator();
}
