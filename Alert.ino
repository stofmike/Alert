int alertMinute = -1;
int alertHour = -1;
int alertWeekDay[7] = {0,0,0,0,0,0,0};
int alertMonthDay = -1;

int triggerTimeSeconds = -1;
int triggerTimeMinute = -1;
int triggerMillis = -1;
bool firstMove = false;

bool checkTime = false;
bool checkAcc = false;
bool checkHourRequired = false;
bool checkWeekDayRequired = false;
bool checkDayMonthRequired = false;
bool setOn = false;
bool firstTimeFlash = true;

bool continueAlert = true;
int compareMillis = -1;
bool movementTriggered = false;

int commandTimeType = -1;

//Variables for what to do what an alert happens
int isOutputSet = -1;
int ledAction = -1;
int ledFlashDuration = -1;
int ledFlashSpeed = -1;
int sendNotification = -1;
int soundBuzzer = -1;
int testConfig = -1;
int startFlashTime = -1;
int flashCounter = 0;
int whichLedFlash = 0;

//Now for the Accelerometer

//Accelerometer Pins
const int x = A0; // X pin connected to Analog 0
const int y = A1; // Y pin connected to Analog 1
const int z = A2; // Z pin connected to Analog 2

boolean calibrated=false; // When accelerometer is calibrated - changes to true
boolean moveDetected=false;

//Accelerometer limits
int xMin; //Minimum x Value
int xMax; //Maximum x Value
int xVal; //Current x Value

int yMin; //Minimum y Value
int yMax; //Maximum y Value
int yVal; //Current y Value

int zMin; //Minimum z Value
int zMax; //Maximum z Value
int zVal; //Current z Value

//Alarm LED
//const int ledPin = D7; // LED connected to D7



int tolerance=20; // Sensitivity of the Alarm
int numSeconds = -1; //number of seconds before Alert
int numTriggered = 0; //used to check to numReads

void setup() {
    Particle.function("setTimeAlert", setTimeAlert);
    Particle.function("setAcc", setAcc);
    Particle.function("setOutput", setOutput);
    Serial.begin(9600);
    pinMode(D7, OUTPUT);
    pinMode(D0, OUTPUT);
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);

    //Accelerometer is connected to A0,A1,A2
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(D7,OUTPUT);
}

void loop() {


    /*
  x = analogRead(A0);       // read analog input pin 0
  y = analogRead(A1);       // read analog input pin 1
  z = analogRead(A2);       // read analog input pin 12
  Serial.print("accelerations are x, y, z: ");
  Serial.print(x);    // print the acceleration in the X axis
  Serial.print(" ");       // prints a space between the numbers
  Serial.print(y, DEC);    // print the acceleration in the Y axis
  Serial.print(" ");       // prints a space between the numbers
  Serial.println(z, DEC);  // print the acceleration in the Z axis
  delay(100);              // wait 100ms for next reading
  */

    checkAccBasedAlert();
    checkTimeBasedAlert();
    checkSetOn();
    /*
        Serial.print(Time.hour());
    Serial.print(":");
    Serial.print(Time.minute());
    Serial.print(":");
    Serial.print(Time.second());
    Serial.print(" Day of Week: ");
    Serial.println(Time.weekday());

    Serial.println("AlertHour: " + String(alertHour));
    Serial.println("alertMinute: " + String(alertMinute));
    Serial.println("alertWeekDay: " + String(alertWeekDay[0]) + "," + String(alertWeekDay[1]) + "," + String(alertWeekDay[2]) + "," + String(alertWeekDay[3]) + "," + String(alertWeekDay[4]) + "," + String(alertWeekDay[5]) + "," + String(alertWeekDay[6]));
    Serial.println("alertMonthDay: " + String(alertMonthDay));
    Serial.println("checkTime: " + String(checkTime));
    Serial.println("checkHourRequired: " + String(checkHourRequired));
    Serial.println("checkWeekDayRequired: " + String(checkWeekDayRequired));
    Serial.println("checkDayMonthRequired: " + String(checkDayMonthRequired));
    //delay(2000);
    */

}
int setAcc(String command) {
    //AAA is tolerance level
    //BB is number of seconds before alert
    tolerance = (((command.charAt(0) - '0') - 0)*100) + (((command.charAt(1) - '0') - 0)*10) + ((command.charAt(2) - '0') - 0);
    numSeconds = (((command.charAt(4) - '0') - 0)*10) + ((command.charAt(5) - '0') - 0);
    Serial.println("Tolerance Change to " + String(tolerance));
    Serial.println("Number of Seconds before alert changed to " + String(numSeconds));
    checkAcc = true;
    firstMove = true;
    calibrated=false;
    return 1;
}

int setOutput(String command){
    //AA is 01 is set output, 02 is clear all outputs
    //BB is what to do with the LEDs.
    //01 is all come on with alarm
    //02 is all flash with alarm
    //03 is cycle thorugh the 3 leds
    //CC is flash duration in seconds
    //DD is flash speed
    //EE is send notification to phone
    //FF is sound buzzer
    //GG is test configuration now as if in alarm state

    isOutputSet = (((command.charAt(0) - '0') - 0)*10) + (((command.charAt(1) - '0') - 0));
    ledAction = (((command.charAt(3) - '0') - 0)*10) + ((command.charAt(4) - '0') - 0);
    ledFlashDuration = (((command.charAt(6) - '0') - 0)*10) + ((command.charAt(7) - '0') - 0);
    ledFlashSpeed = (((command.charAt(9) - '0') - 0)*10) + ((command.charAt(10) - '0') - 0);
    ledFlashSpeed = ledFlashSpeed*100;
    sendNotification = (((command.charAt(12) - '0') - 0)*10) + ((command.charAt(13) - '0') - 0);
    soundBuzzer = (((command.charAt(15) - '0') - 0)*10) + ((command.charAt(16) - '0') - 0);
    testConfig = (((command.charAt(18) - '0') - 0)*10) + ((command.charAt(19) - '0') - 0);

    if (testConfig == 1 ) {
        testConfig = 0;
        setOn = true;
    }
    if (isOutputSet == 1) {
        //setOn = true;
    }
    return 1;
}
int setTimeAlert(String command){
    //Fomrat is
    //AA - 01 is on, 02 is Off
    //BB is type to be Set. 01 = timeOfDay. 02 = everyHour. 03 = weekday. 04 = everymonth
    //CC - Hour (ths will be ignore for BB = 02)
    //DD - Minute
    //E-K = weekdays starting with Sunday (only applicable when BB = 03)
    //JJ = day of month (only applicable when BB = 04)

    //If setAlarm
    if (((command.charAt(1) - '0') - 0) == 1) {
        checkTime = true;
        commandTimeType = (((command.charAt(3) - '0') - 0)*10) + ((command.charAt(4) - '0') - 0);
        switch (commandTimeType) {
            case 1://timeOfDay
                Serial.println("Inside Case 1");
                checkHourRequired = true;
                checkWeekDayRequired = false;
                checkDayMonthRequired = false;
                alertHour = (((command.charAt(6) - '0') - 0)*10) + ((command.charAt(7) - '0') - 0);
                alertMinute = (((command.charAt(9) - '0') - 0)*10) + ((command.charAt(10) - '0') - 0);
            break;
            case 2://everyHour
                Serial.println("Inside Case 2");
                checkHourRequired = false;
                checkWeekDayRequired = false;
                checkDayMonthRequired = false;
                 alertMinute = (((command.charAt(9) - '0') - 0)*10) + ((command.charAt(10) - '0') - 0);
            break;
            case 3://weekDay
                Serial.println("Inside Case 3");
                checkHourRequired = true;
                checkWeekDayRequired = true;
                checkDayMonthRequired = false;
                alertHour = (((command.charAt(6) - '0') - 0)*10) + ((command.charAt(7) - '0') - 0);
                 alertMinute = (((command.charAt(9) - '0') - 0)*10) + ((command.charAt(10) - '0') - 0);
                 alertWeekDay[0] = ((command.charAt(12) - '0') - 0);
                 alertWeekDay[1] = ((command.charAt(14) - '0') - 0);
                 alertWeekDay[2] = ((command.charAt(16) - '0') - 0);
                 alertWeekDay[3] = ((command.charAt(18) - '0') - 0);
                 alertWeekDay[4] = ((command.charAt(20) - '0') - 0);
                 alertWeekDay[5] = ((command.charAt(22) - '0') - 0);
                 alertWeekDay[6] = ((command.charAt(24) - '0') - 0);

            break;
            case 4://everyMonth
                Serial.println("Inside Case 4");
                checkHourRequired = true;
                checkWeekDayRequired = false;
                checkDayMonthRequired = true;
                alertHour = (((command.charAt(6) - '0') - 0)*10) + ((command.charAt(7) - '0') - 0);
                 alertMinute = (((command.charAt(9) - '0') - 0)*10) + ((command.charAt(10) - '0') - 0);
                 alertMonthDay = (((command.charAt(26) - '0') - 0)*10) + ((command.charAt(27) - '0') - 0);
            break;
        }
    }



    return  1;
}
void checkSetOn() {
     /*
     Variables set by funstion to be used here. This block can be deleted
    //AA is 01 is set output, 02 is clear all outputs
    //BB is what to do with the LEDs.
    //01 is all come on with alarm
    //02 is all flash with alarm
    //03 is cycle thorugh the 3 leds
    //CC is flash duration in seconds
    //DD is flash speed
    //EE is send notification to phone
    //FF is sound buzzer
    //GG is test configuration now as if in alarm state
    isOutputSet = (((command.charAt(0) - '0') - 0)*10) + (((command.charAt(1) - '0') - 0));
    ledAction = (((command.charAt(3) - '0') - 0)*10) + ((command.charAt(4) - '0') - 0);
    ledFlashDuration = (((command.charAt(6) - '0') - 0)*10) + ((command.charAt(7) - '0') - 0);
    ledFlashSpeed = (((command.charAt(9) - '0') - 0)*10) + ((command.charAt(10) - '0') - 0);
    sendNotification = (((command.charAt(12) - '0') - 0)*10) + ((command.charAt(13) - '0') - 0);
    soundBuzzer = (((command.charAt(15) - '0') - 0)*10) + ((command.charAt(16) - '0') - 0);
    testConfig = (
    */

    if (setOn) {
        //Serial.println("Inside setOn");
        if (sendNotification) {
            sendNotification = false;
            Particle.publish("Alert", "Alert");
        }
        if (ledAction == 1) { // write high to all leds
            Serial.println("INside ledAction1");
            digitalWrite(D0, HIGH);
            digitalWrite(D1, HIGH);
            digitalWrite(D2, HIGH);
        }
         else if (ledAction == 2 || ledAction == 3) { //cycle


            if (firstTimeFlash) {
                Serial.println("inside firstTimeFlash");
                startFlashTime = millis();

                firstTimeFlash = false;
                flashCounter = millis();
                if (ledAction ==3 ) {
                    Serial.println("ledFlash = 3");
                    whichLedFlash = 0;
                }
                else {
                    whichLedFlash = 3;
                    Serial.println("ledFlash = 2");
                }
            }


            if (millis() - startFlashTime < ledFlashDuration*1000) {
               Serial.println("millis () - flashCounter is  " + String(millis () - flashCounter) + ". ledFlashSpeed " + String(ledFlashSpeed));
                if (millis () - flashCounter > ledFlashSpeed) {
                    Serial.println("inside ledaction3 " + String(millis() - startFlashTime));

                    Serial.println("ledflash speed is " + String(ledFlashSpeed));
                    Serial.println("whichLedFlash " + String(whichLedFlash));
                    if (whichLedFlash == 0) {
                        digitalWrite(D0, HIGH);
                        digitalWrite(D1, LOW);
                        digitalWrite(D2, LOW);
                    }
                    else if (whichLedFlash == 1) {
                        digitalWrite(D0, LOW);
                        digitalWrite(D1, HIGH);
                        digitalWrite(D2, LOW);
                    }
                    else if (whichLedFlash ==2 ) {
                        digitalWrite(D0, LOW);
                        digitalWrite(D1, LOW);
                        digitalWrite(D2, HIGH);
                        whichLedFlash = -1;
                    }
                    else if (whichLedFlash == 3) {
                        whichLedFlash = 2;
                        if(digitalRead(D0)==LOW) {
                            Serial.println("Writing High");
                            digitalWrite(D0, HIGH);
                            digitalWrite(D1, HIGH);
                            digitalWrite(D2, HIGH);
                        }
                        else {

                            digitalWrite(D0, LOW);
                            digitalWrite(D1, LOW);
                            digitalWrite(D2, LOW);
                        }
                    }
                    flashCounter = millis();
                whichLedFlash = whichLedFlash + 1;
                }
            }
            else {
                setOn = false;
                digitalWrite(D0, LOW);
                digitalWrite(D1, LOW);
                digitalWrite(D2, LOW);
                firstTimeFlash = true;
            }

        }
        //Alert has been triggered
        //digitalWrite(D7, HIGH);
        //Serial.println("ALERT");
        //delay(1000);
        //digitalWrite(D7,LOW);

    }
}

void checkAccBasedAlert() {
    if (checkAcc) {
       // Serial.println("Inside checkAcc. Movedetected is " + String(moveDetected));
        if (calibrated == false) {
            calibrateAccel();
        }

         // Once the accelerometer is calibrated - check for movement
        if(calibrated){
            if(checkMotion()){
                moveDetected=true;
            }
            else if (checkMotion() == false) {
                moveDetected=false;
            }
        }

        if (millis() - compareMillis > 1000 && movementTriggered) {
            //no reading for 1000 miliiseconds
            Serial.println("No Reading for 1000 milliseconds");
            continueAlert = false;
            firstMove = true;
            movementTriggered = false;
        }



        // If motion is detected - sound the alarm !
        if(moveDetected){
            //compareMillis = millis();
            if (firstMove) {
                firstMove = false;
                //movementTriggered = true;
                triggerTimeSeconds = Time.second();
                triggerTimeMinute = Time.minute();
                triggerMillis = millis();
                Serial.println("First Trigger time is " + String(triggerTimeMinute) + ":" + String(triggerTimeSeconds));
                continueAlert = true;

            }


            if (millis() - triggerMillis > numSeconds*1000) {
               if (continueAlert) {
                    Serial.println("Time based Trigger hapenned. Millis is " + String(millis()) + "Trigger millis is " + String(triggerMillis) + ". Difference is " + String(millis() - triggerMillis) + "numSeconds is " + String(numSeconds));
                    continueAlert = false;
                    setOn = true;
                    Serial.println("Value of setOn is " + String(setOn));
                    firstMove = true;
                }
            }

            /*
            numTriggered = numTriggered + 1;
            Serial.println("Number of Times Triggered is " + String(numTriggered) + "Trigger Threshold is " + String(numReads));
            if (numTriggered == numReads) {
                Serial.println("ALARM");
                ALARM();
                numTriggered = 0;
                //delay(1000);
            }
            */
        //moveDetected = false;
        }
    }
}
void checkTimeBasedAlert() {
    //Alert can be set for
    //timeOfDay - Every day at
    //everyHour - Every hour at
    //weekDay - Every weekday at
    //month - Every month on

    //if a time based alert has been set
    if (checkTime) {
    //every case requires a check on the mins so check that first
        if (Time.minute() == alertMinute) {
            setOn = true;
            //if EveryHour has been set then the hour does not need to be checked. All other cases the hour needs to be checked
            if (checkHourRequired) {
                if (Time.hour() == alertHour) {
                    setOn = true;
                }
                else {
                    setOn = false;
                }
            }//checkHourRequired
            // heck to see if teh alert should only come on specific days of the week
            if (checkWeekDayRequired) {
                //Sunday is 1
                if (Time.weekday() == alertWeekDay[Time.weekday() - 1]) {
                    setOn = true;
                }
                else {
                    setOn = false;
                }
            }//checkWeekDayRequired
            if (checkDayMonthRequired) {
                if (Time.day() == alertMonthDay) {
                    setOn = true;
                }
                else {
                    setOn = false;
                }
            }// checkDayMonthRequired
        }// Time.minute() == alertMinute
        else {
            setOn = false;
        }

    }// checkTime
} //checkTimeBasedAlert

void calibrateAccel(){
 // reset alarm
 moveDetected=false;

 //initialise x,y,z variables
 xVal = analogRead(x);
 xMin = xVal;
 xMax = xVal;

 yVal = analogRead(y);
 yMin = yVal;
 yMax = yVal;

 zVal = analogRead(z);
 zMin = zVal;
 zMax = zVal;


 //calibrate the Accelerometer (should take about 0.5 seconds)
 for (int i=0; i<50; i++){
 // Calibrate X Values
 xVal = analogRead(x);
 if(xVal>xMax){
 xMax=xVal;
 }else if (xVal < xMin){
 xMin=xVal;
 }

 // Calibrate Y Values
 yVal = analogRead(y);
 if(yVal>yMax){
 yMax=yVal;
 }else if (yVal < yMin){
 yMin=yVal;
 }

 // Calibrate Z Values
 zVal = analogRead(z);
 if(zVal>zMax){
 zMax=zVal;
 }else if (zVal < zMin){
 zMin=zVal;
 }

 //Delay 10msec between readings
 delay(10);
 }

 //End of calibration sequence sound. ARMED.

 printValues(); //Only useful when connected to computer- using serial monitor.
 calibrated=true;

}

// Prints the Sensor limits identified during Accelerometer calibration.
// Prints to the Serial monitor.
void printValues(){
 Serial.print("xMin=");
 Serial.print(xMin);
 Serial.print(", xMax=");
 Serial.print(xMax);
 Serial.println();

 Serial.print("yMin=");
 Serial.print(yMin);
 Serial.print(", yMax=");
 Serial.print(yMax);
 Serial.println();

 Serial.print("zMin=");
 Serial.print(zMin);
 Serial.print(", zMax=");
 Serial.print(zMax);
 Serial.println();

 Serial.println("------------------------");
}

//Function used to detect motion. Tolerance variable adjusts the sensitivity of movement detected.
boolean checkMotion(){
 boolean tempB=false;
 xVal = analogRead(x);
 yVal = analogRead(y);
 zVal = analogRead(z);

 if(xVal >(xMax+tolerance)||xVal < (xMin-tolerance)){
 tempB=true;
 Serial.print("X Failed = ");
 Serial.println(xVal);
 }

 if(yVal >(yMax+tolerance)||yVal < (yMin-tolerance)){
 tempB=true;
 Serial.print("Y Failed = ");
 Serial.println(yVal);
 }

 if(zVal >(zMax+tolerance)||zVal < (zMin-tolerance)){
 tempB=true;
 Serial.print("Z Failed = ");
 Serial.println(zVal);
 }

 return tempB;
}

//Function used to make the alarm sound, and blink the LED.
void ALARM(){
 moveDetected=false;
 Particle.publish("accAlarm","accAlarm");
 numTriggered = 0;


 //don't check for movement until recalibrated again
 //calibrated=false;

 // sound the alarm and blink LED
 digitalWrite(D7, HIGH);
 delay(100);
 digitalWrite(D7, LOW);
}
