// Uncomment for serial monitor and LED debug
// #define DEBUG // SERIAL DOES NOT WORK ON ATTiny

#define PIN_REC_INSIDE 0 // IR RECEIVER A
#define PIN_REC_DOOR 2   // IR RECEIVER B
#define PIN_RELAY 4      // RELAY to turn on/off

int people = 0;
bool recActiveInside = false;
bool recActiveDoor = false;

enum State
{
  NONE,
  BOTH,
  DOOR,
  INSIDE
};

const int PAST_LEN = 3;
State pastStates[PAST_LEN] = {NONE, NONE, NONE}; // FIFO of state changes

State firstState = NONE;
State lastState = NONE;

void setup()
{
  pinMode(PIN_REC_INSIDE, INPUT);
  pinMode(PIN_REC_DOOR, INPUT);
  pinMode(PIN_RELAY, OUTPUT);

  digitalWrite(PIN_RELAY, HIGH);

  delay(1000);

  while (digitalRead(PIN_REC_INSIDE) == HIGH || digitalRead(PIN_REC_DOOR) == HIGH)
  {
    delay(100);
  }

  digitalWrite(PIN_RELAY, LOW);

#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("READY");
#endif
}

void readReceivers()
{
  recActiveInside = digitalRead(PIN_REC_INSIDE) == HIGH ? true : false;
  recActiveDoor = digitalRead(PIN_REC_DOOR) == HIGH ? true : false;
}

State getState()
{
  if (recActiveInside && recActiveDoor)
  {
    return BOTH;
  }

  if (recActiveInside && !recActiveDoor)
  {
    return INSIDE;
  }

  if (!recActiveInside && recActiveDoor)
  {
    return DOOR;
  }

  return NONE;
}

bool hasEntered()
{
  return pastStates[1] == DOOR && pastStates[0] == BOTH;
}

bool hasLeft()
{
  return pastStates[1] == INSIDE && pastStates[0] == BOTH;
}

void resetStates()
{
  for (byte i = 0; i < PAST_LEN; i++)
  {
    pastStates[i] = NONE;
  }
}

bool storeState(State newState)
{
  if (pastStates[0] == newState)
  { // latest state is the same as new
    return false;
  }

  for (byte i = PAST_LEN - 1; i > 0; i--)
  {
    pastStates[i] = pastStates[i - 1];
  }

  pastStates[0] = newState;

  return true;
}

void printStates()
{
#ifdef DEBUG
  Serial.print(" > ");

  for (byte i = 0; i < PAST_LEN; i++)
  {
    switch (pastStates[i])
    {
    case NONE:
      Serial.print(" NONE ");
      break;
    case BOTH:
      Serial.print(" BOTH ");
      break;
    case DOOR:
      Serial.print(" DOOR ");
      break;
    case INSIDE:
      Serial.print("INSIDE");
      break;
    }

    if (i < PAST_LEN - 1)
      Serial.print(" | ");
  }
  Serial.println(" < ");
#endif
}

void loop()
{
  readReceivers();
  State currentState = getState();

  if (storeState(currentState))
  {
    printStates();

    if (hasEntered())
    {
      people++;

#ifdef DEBUG
      Serial.print("ENTER ");
      Serial.println(people);
#endif
    }

    if (hasLeft())
    {
      if (people > 0)
        people--;

#ifdef DEBUG
      Serial.print("LEAVE ");
      Serial.println(people);
#endif
    }

    if (pastStates[0] == INSIDE && pastStates[1] == NONE && people == 0)
    {
      // HACK: when it fails for some reason and there is someone inside
      people = 1;
    }

    if (people > 0)
    {
      digitalWrite(PIN_RELAY, HIGH);
      return;
    }

    digitalWrite(PIN_RELAY, LOW);
  }
}
