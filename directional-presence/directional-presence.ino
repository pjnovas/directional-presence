// Uncomment for serial monitor and LED debug
// #define DEBUG // SERIAL DOES NOT WORK ON ATTiny

#ifdef DEBUG  
  #define PIN_LED_A 5
  #define PIN_LED_B 6
#endif

#define PIN_REC_A A3    // IR RECEIVER A 
#define PIN_REC_B A2    // IR RECEIVER B
#define PIN_RELAY 2     // RELAY to turn on/off 

#define THRESHOLD_A 40  // Minimal distance of Receiver A (for noise)
#define THRESHOLD_B 40  // Minimal distance of Receiver B (for noise)

int count = 0;
bool recActiveA = false;
bool recActiveB = false;

enum State {
  NONE,
  BOTH,
  ENTERING,
  LEAVING
};

State firstState = NONE;
State lastState = NONE;

void setup() {  
  pinMode(PIN_REC_A, INPUT);
  pinMode(PIN_REC_B, INPUT);
  pinMode(PIN_RELAY, OUTPUT);

  digitalWrite(PIN_RELAY, LOW);

#ifdef DEBUG  
  pinMode(PIN_LED_A, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);

  digitalWrite(PIN_LED_A, HIGH);
  digitalWrite(PIN_LED_B, HIGH);

  Serial.begin(9600);
  Serial.println("READY");
#endif
}

void readReceivers() {
  recActiveA = analogRead(PIN_REC_A) > THRESHOLD_A ? true : false;
  recActiveB = analogRead(PIN_REC_B) > THRESHOLD_B ? true : false;

#ifdef DEBUG  
  digitalWrite(PIN_LED_A, recActiveA ? LOW : HIGH);
  digitalWrite(PIN_LED_B, recActiveB ? LOW : HIGH);

  Serial.print("A: ");
  Serial.print(analogRead(PIN_REC_A));
  Serial.print(" | B: ");
  Serial.println(analogRead(PIN_REC_B));
#endif
}

State getState() {
  if (recActiveA && recActiveB) {
    return BOTH;
  }

  if (recActiveA && !recActiveB) {
    return ENTERING;
  }

  if (!recActiveA && recActiveB) {
    return LEAVING;
  }

  return NONE;
} 

bool hasEntered() {
  return lastState == ENTERING && firstState == LEAVING;
}

bool hasLeft() {
  return lastState == LEAVING && firstState == ENTERING;
}

void resetStates() {
  firstState = NONE;
  lastState = NONE;
}

void loop() {
  readReceivers();
  State newState = getState();

  if (newState == lastState || newState == BOTH) {
    return; // ignore both on or non changed states
  }

  if (newState == NONE) {
    if (firstState == NONE || lastState == NONE) {
      return;
    }

    if (hasEntered()) {
      if (count == 0) digitalWrite(PIN_RELAY, HIGH);
      count++;

#ifdef DEBUG  
      Serial.print("ENTER ");
      Serial.println(count);
#endif
    }
    
    if (hasLeft()) {
      if (count == 1) digitalWrite(PIN_RELAY, LOW);      
      if (count > 0) count--;

#ifdef DEBUG  
      Serial.print("LEAVE ");
      Serial.println(count);
#endif
    }

    resetStates();
    return;
  }

  if (firstState == NONE) {
    firstState = newState;
    return;
  }

  if (lastState == NONE && firstState != newState) {
    lastState = newState;
    return;
  }
}
