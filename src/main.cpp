#include <Arduino.h>

int ledPin = 2;
int buttonPin = 4;

String test = "Test";

void setup() {
  Serial.begin(921600);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  Serial.println(test);
}

void loop() {
  int buttonState = digitalRead(buttonPin);

  digitalWrite(ledPin, !buttonState); 

  delay(50);
}

// void setup() {
//   pinMode(LED_BUILTIN, OUTPUT);
//   Serial.begin(921600);
//   Serial.println("Hello from setup");
// }

// void loop() {
//   delay(1000);
//   digitalWrite(LED_BUILTIN, HIGH);
//   Serial.println("Hello from the loop");
//   delay(1000);
//   digitalWrite(LED_BUILTIN, LOW);
// }




// // put function declarations here:
// int myFunction(int, int);

// void setup() {
//   // put your setup code here, to run once:
//   int result = myFunction(2, 3);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
// }

// // put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }