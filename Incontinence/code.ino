const int READ = A0;
const int PROBE = 0;
const int BUZZ = 1;

const int GAP_R = 2200; // Resistance between probes
const int BRIDGE_R = 2000; // Resistance between connected rails
const int DIVIDER_R = 10000; // Resistance of potential divider

const int SUPPLY = 5;
const float THRESHOLD = 0.001; // Voltage required at divider for there to be a connection.
const int GAPS = 5;

bool connected = false;
bool buzzOn = false;
int currentGap = 0;
int buzzSteps = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(READ, INPUT);
  pinMode(PROBE, OUTPUT);
  pinMode(BUZZ, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (connected) {
    buzzSteps--;
    if (buzzSteps <= 0) {
      buzzSteps = currentGap;
      buzzOn = !buzzOn;
      digitalWrite(BUZZ, buzzOn);
    }
  }
  
  delay(450); // Settle
  digitalWrite(PROBE, 1);
  delay(50); // Settle
  float voltage = analogRead(READ) / 1023.0 * 5;
  digitalWrite(PROBE, 0);

  if (voltage < THRESHOLD) {
    connected = false;
    buzzOn = false;
    digitalWrite(BUZZ, 0);
    return;
  }
  connected = true;
  float res = DIVIDER_R * (SUPPLY / voltage);
  currentGap = (res - BRIDGE_R) / GAP_R;
}

