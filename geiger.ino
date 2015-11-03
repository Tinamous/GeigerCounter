#define BUILD_NUMBER '6'

int led = D1;
int ledComms = D1; // use led for now, but this is RS232 comms LED.
int pulse = D0;

int volatile countPerInterval = 0;
int volatile countPerMinute = 0;
int volatile ledOn = 0;

int cpmIntervalTotal = 0;
int tickIntervalTotal = 0;

int loopDelay = 10;

int lastCountPerMinute = 0;
int lastCountPerInterval = 0;

int cpm = 0;

const int maxPointsPerInterval = 30;
int countsPerInterval[maxPointsPerInterval]; // = {0,0,0,0,0,0};
int lastIndex = 0;

// Meta data about the device.
int rfStrength = 0;
char *localIp = "000.000.000.000";
char *ssid = "unknown sssid";
//int ssid=1;
int metaLoopCounter = 0;
int build = 9;
bool firstRun = true;

// This routine runs only once upon reset
void setup()
{
    // Take control of the spark LED
    RGB.control(true);
    RGB.brightness(10);
    RGB.color(255, 0, 0);
  
    // Initialize D0 pin as output
    pinMode(led, OUTPUT);
    pinMode(ledComms, OUTPUT);
    pinMode(pulse, INPUT_PULLDOWN);
  
    // Setup the spark variables.
    Spark.variable("cpm", &cpm, INT);
    Spark.variable("lastCountPer", &lastCountPerInterval, INT);
    Spark.variable("rfStrength", &rfStrength, INT);
    Spark.variable("build", &build, INT);

    // Little flash to indicate it's alive
    flashLed();
    
    Spark.publish("Status","Geiger setup complete.");
    Spark.publish("Version","1.0.6");
    
    RGB.color(0, 255, 0);
    attachInterrupt(pulse, flashLed, RISING);
}

// This routine loops forever
void loop()
{
    if (firstRun) {
        //firstRun = false;
          //if (WiFi.ready()) {
        //ssid = WiFi.SSID();
        //Spark.publish("Status", "Connected to " + String(ssid), 240, PRIVATE);
  
        //IPAddress address=  WiFi.localIP();
        //sprintf(localIp, "%d.%d.%d.%d", address[0], address[1], address[2], address[3]);
        //Spark.publish("Status", "Ip Address: " + String(localIp), 240, PRIVATE);
  //}
    }
    
  cpmIntervalTotal+=loopDelay;
  tickIntervalTotal+=loopDelay;
  metaLoopCounter+=loopDelay;
  
  if (ledOn > 0) {
      ledOn++;
  }
  
  // If the led has been set on by a pulse allow it time and then switch it off.
  if (ledOn > 5) {
      digitalWrite(led, LOW);
      digitalWrite(ledComms, LOW);
  }
  
  // Once per minute reset all the cpm counters.
  if (cpmIntervalTotal > 60000) {
      //cpm = countPerMinute;
      countPerMinute = 0;
      cpmIntervalTotal = 0;
  }
  
  // Every 10 seconds update the cpm with the rolling average
  // over the last minute.
  if (tickIntervalTotal > 1000) {
       
       // If the array is full then shift the contents down.
       shiftCountsArray();
       
        // Store the new reading in the last array position.
        countsPerInterval[lastIndex] = countPerInterval;
        lastCountPerInterval = countPerInterval;
        
        // Reset the counters
        countPerInterval = 0;
        tickIntervalTotal = 0;
       
        // Update the Spark variable
        // because we are averaging over 6 points
        // at 10 seconds the total count is cpm
        // for the last minute.
        cpm = computeTotalCounts();
       
        // Increment the last index to indicate that the 
        lastIndex++;
        if (lastIndex > maxPointsPerInterval -1) {
            lastIndex = maxPointsPerInterval -1;
        }
        
        // If High (well slightly higher than normal) 
        // make the LED red. Otherwise green
        if (cpm > 50) {
            RGB.color(255, 0, 0);
        } else {
            RGB.color(0, 255, 0);
        }
  }
  
  // Every 60 seconds get the WiFi status
  if (metaLoopCounter > 60000) {
    Spark.publish("Status","Read WiFi.");
    noInterrupts();
      
      
    //if (WiFi.ready()) {
        rfStrength = WiFi.RSSI();
//    }
    
    interrupts();
    metaLoopCounter =0;
  }
  
  // Delay by about 10ms.
  delay(loopDelay);
}

void shiftCountsArray() {
    // Shift the counts down one to add the latest on the top
    if (lastIndex >= maxPointsPerInterval -1 ) {
        for (int i=0; i<lastIndex; i++) {
            countsPerInterval[i] = countsPerInterval[i+1];
        }
    }
}

int computeTotalCounts() {
    int totalCounts = 0;
    for (int i=0; i<=lastIndex; i++) {
        totalCounts+=countsPerInterval[i];
    }
    return totalCounts * 2;
}

void flashLed() {
    countPerInterval++;
    countPerMinute++;
    ledOn = 1;
    digitalWrite(led, HIGH);
}
