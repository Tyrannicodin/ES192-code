// Code modified from https://www.instructables.com/Simple-Arduino-Metal-Detector/

const int pulsePin = A1;
const int capPin = A2;
const int buz = 9;

long expectedSum = 0; // Running sum of 64 sums 
long ignored = 0; // Number of ignored sums
long diff = 0; // Difference between sum and avgsum
long pTime = 0;
long buzPeriod = 0;

void setup() 
{
  Serial.begin(9600);
  pinMode(pulsePin, OUTPUT); 
  digitalWrite(pulsePin, LOW);
  pinMode(capPin, INPUT);  
  pinMode(buz, OUTPUT);
  digitalWrite(buz, LOW);
}

void loop() 
{
  int minval = 1023;
  int maxval = 0;
  long unsigned int sum = 0;
  for (int i = 0; i < 256; i++) {
    // Drain capacitor
    pinMode(capPin, OUTPUT);
    digitalWrite(capPin, LOW);
    delayMicroseconds(20);
    pinMode(capPin, INPUT);
    applyPulses();
    
    // Read capacitor voltage
    int val = analogRead(capPin); // Takes 13x8=104 microseconds
    minval = min(val, minval);
    maxval = max(val, maxval);
    sum += val;
    
    long unsigned int cTime=millis();
    char buzState=0;
    if (cTime < pTime + 10) {
      if (diff > 0) {
        buzState = 1;
      } else if (diff < 0) {
        buzState = 2;
      }
    }
    if (cTime < pTime + buzPeriod) {
      if (diff > 0) {
        buzState = 1;
      } else if (diff < 0) {
        buzState = 2;
        pTime = cTime;
      }
    }
    if (buzPeriod > 300) {
      buzState = 0;
    }

    if (buzState==0) {
      noTone(buz);
      Serial.write("low\n");
    } else if (buzState==1) {
      tone(buz, 100);
      Serial.write("high\n");
    } else if (buzState==2) {
      tone(buz, 500);
      Serial.write("high\n");
    }
  }

  // Remove min and max to reduce random spikes
  sum -= minval;
  sum -= maxval;
  
  if (expectedSum==0) {
    expectedSum = sum << 6; // Update expected value to be 2**6 (64) * the sum
  }

  long int avgsum = (expectedSum + 32) >> 6; 
  diff = sum - avgsum;
  if (abs(diff) > 10) {
    expectedSum = expectedSum + sum - avgsum;
    ignored = 0;
  } else {
    ignored++;
  }

  if (ignored > 64) { 
    expectedSum = sum<<6;
    ignored = 0;
  }
  
  if (diff == 0) {
    buzPeriod = 1000000; // Very large
  } else {
    buzPeriod = avgsum / (2 * abs(diff));
  }
}

void applyPulses()
{
    for (int i = 0; i < 3; i++) 
    {
      digitalWrite(pulsePin, HIGH); // Take 3.5 uS
      delayMicroseconds(3);
      digitalWrite(pulsePin, LOW);  // Take 3.5 uS
      delayMicroseconds(3);
    }
}  