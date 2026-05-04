const int NODES = 9;
const int STATES = 8;
const int OPS = 4;
const int RES = 12;
const int EQNS = STATES*NODES;

int edges[RES][2] = {
  {0, 1},
  {1, 2},
  {0, 3},
  {1, 4},
  {2, 5},
  {3, 4},
  {4, 5},
  {3, 6},
  {4, 7},
  {5, 8},
  {6, 7},
  {7, 8},
};

float voltages[STATES][NODES];

const int exciters[OPS] = {9, 8, 7, 6};

const int S0 = 2;
const int S1 = 3;
const int S2 = 4;
const int S3 = 0;

const int MUXIN = A0;

void readVoltages(int dest) {
  for (int i = 0; i < NODES; i++) {
    digitalWrite(S0, bitRead(i, 0));
    digitalWrite(S1, bitRead(i, 1));
    digitalWrite(S2, bitRead(i, 2));
    digitalWrite(S3, bitRead(i, 3));
    delay(50);
    analogRead(MUXIN);
    delay(5);
    voltages[dest][i] = analogRead(MUXIN) / 1023.0 * 5;
  }
}

int high;
int low;

int states[STATES][4] = {
  { 1, -1, 0, -1}, 
  {-1, 1, -1, 0},
  { 1, 0, -1, -1},
  {-1, -1, 1, 0},
  { 1, -1, -1, 0},
  {-1, 1, 0, -1},
  { 1, 1, 0, 0},
  { 0, 0, 1, 1},
};

int stateNodes[4] = {0, 6, 8, 5};

void setState(int state) {
  int* config = states[state];
  high = 0;
  low = 0;
  for (int i = 0; i < 4; i++) {
    if (config[i] == -1) {
      pinMode(exciters[i], INPUT);
    } else {
      if (config[i] == 1) {
        high = stateNodes[i];
      } else if (config[i] == 0) {
        low = stateNodes[i];
      }
      pinMode(exciters[i], OUTPUT);
      digitalWrite(exciters[i], config[i]);
    }
  }
  delay(50);
  readVoltages(state);
}

void populate() {
  int eqn = 0;

  for (int state = 0; state < STATES; state++) {
    setState(state);
  }
}


void setup() {
  // put your setup code here, to run once:
  pinMode(MUXIN, INPUT);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(exciters[0], INPUT);
  pinMode(exciters[1], INPUT);
  pinMode(exciters[2], INPUT);
  pinMode(exciters[3], INPUT);

  Serial.begin(9600);

  setState(0);
}

void loop() {
  // put your main code here, to run repeatedly:
  populate();
  Serial.print("!");
  for (int state = 0; state < STATES; state++) {
    for (int i = 0; i < 4; i++) {
      Serial.print(states[state][i]);
      if (i != 4 - 1) {
        Serial.print(",");
      }
    }
    Serial.print("|");
    for (int v = 0; v < NODES; v++) {
      Serial.print(voltages[state][v], 8);
      if (v != NODES - 1) {
        Serial.print(",");
      }
    }
    if (state != STATES -1) {
      Serial.print(":");
    }
  }
  Serial.println("!");
  // A is the system equations.
  // B is the solutions to the equations.
}
