
/*



              Mustafa Ayberk Kale 50359 mkale4
              Mert Akman          53908 makman15



*/

//SD Card and RTC Libraries
#include <SD.h>
#include <SPI.h>
#include <Time.h>
#include <TimeLib.h>
#include <DS1302RTC.h>

//RTC Values

#define SHour 22
#define SMinute 14
#define SSecond 10
#define SDay 26
#define SMonth 12
#define SYear 2017






//Servo

#include<Servo.h> //getting servo library
Servo windowServo;// create servo object to control a servo
int servoPin = 13;

//Relays
int relayPin1 = 25; //12V
int relayPin2 = 23; //5V
int relayPin3 = 22; //Empty

//LDR

int LDRPin1 = A2;
int LDRPin2 = A3;

int LDRValue = 0;      //a variable to store LDR values
int LDRValue2 = 0;

int light_sensitivity = 500;    //This is the approx value of light surrounding your LDR
bool LDRcondition = false;

//Rain Sensor

int rainSensorPin = A1;

bool RainCondition = false;

int rainSensorValue = 0;

//LM35 Temperature

int tempPinInside = A11;
int tempPinOutside = A6;

float TemperatureMeasuredInside = 0;
float TemperatureMeasuredOutside = 0;

// Temporary variables
float oldTemperatureIns = 0;
float oldTemperatureOut = 0;



//Wifi Communication to Server



/**********************  WiFi SSID Name and Password **************************/

String Host_name = "MBP13A";//wifi name
String password = "ayberk123";//wifi password

String sendData(String command, const int Goldout, boolean debug); // send and receive data to/from Thingspeak

void connectToWiFi();//wifi connectection maker

//Functions to Get data from server

String WSAirConCondition();
String WSLightConCondition();
String WSWindowsCondition();
String WSisManualControlActive();


String WSLatitude();
String WSLongitude();
String WSmanualTempSet();




//backups initial condition from server

String backupAirConCondition = "false";
String backupLightConCondition = "false";
String backupWindowsCondition = "false";
String backupisManualControlActive = "false";
String backupLatitude = "";
String backupLongitude = "";
String backupmanualTempSet = "23";



//// 

bool BoolStringtoBoolean(String str);//string true/false to boolean true/false converter function
String BooltoStr(bool a);// true and false booleans to string converter


//Location


float CompLocationLat = 41.204842;  //constant
float CompLocationLong = 29.070287;


float userLocationLat = 41.195715; //will come from phone's gps though the applicatio
float userLocationLong = 29.061204;

void LocationDifferenceCalculator();

float LocationDifference = 1.27; //comes from LocationDifferenceCalculator function

float IdealLocationDifference = 0.500; //in kilometers , the desired distance between home and user


//DatalogtoSerial

//RTC Pins
// DS1302:  CE pin    -> Arduino Digital 33
//          I/O pin   -> Arduino Digital 35
//          SCLK pin  -> Arduino Digital 37
//          GND pin   -> Arduino Digital 39
//          VCC pin   -> Arduino Digital 41

// Set pins:  CE, IO,CLK
DS1302RTC RTC(33, 35, 37);

// Optional connection for RTC module
#define DS1302_GND_PIN 39
#define DS1302_VCC_PIN 41



tmElements_t tm;

//SDcard

int LineCounter = 0;
File myFile;


int pinCS = 53; // SD Card CS pin


// Manual Buttons// come from server

bool isManualControlActive = false;

bool WindowsCondition = false;

bool LightCondition = false;

bool AirConCondition = false;

double manualTempSet = 22;

// All functions pre-defined

void LDRfunction();
void servoFunction();
void tempFunction();
void relayFunction();
void rainFunction();
void windowsConditionCheck();
void DataLogtoSD();

//





void setup() {

  Serial.begin(57600);//serial monitor
  Serial1.begin(250000);//Esp 8266

  windowServo.attach(servoPin);
  pinMode(relayPin1, OUTPUT);//LED relay
  pinMode(relayPin2, OUTPUT);// AC relay
  pinMode(relayPin3, OUTPUT);//empty

  connectToWiFi();// connect to wifi

  //SD Card

  pinMode(pinCS, OUTPUT);

  // SD Card Initialization
  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }

  // DS1302

  Serial.println("DS1302RTC Read Test");
  Serial.println("-------------------");

  // Activate RTC module
  digitalWrite(DS1302_GND_PIN, LOW);
  pinMode(DS1302_GND_PIN, OUTPUT);

  digitalWrite(DS1302_VCC_PIN, HIGH);
  pinMode(DS1302_VCC_PIN, OUTPUT);

  Serial.println("RTC module activated");
  Serial.println();
  delay(500);


  setTime(SHour, SMinute, SSecond, SDay, SMonth, SYear);   //set the system time to
  //23h31m30s on 3Feb2009
  RTC.set(now());                     //set the RTC from the system time

  if (RTC.haltRTC()) {
    Serial.println("The DS1302 is stopped.  Please run the SetTime");
    Serial.println("example to initialize the time and begin running.");
    Serial.println();
  }
  if (!RTC.writeEN()) {
    Serial.println("The DS1302 is write protected. This normal.");
    Serial.println();
  }
//Datato SDCard Logger


}

int LocationInit = 1;



/*
 * 
 * 
 *                                     ***********************************MAIN CODE*************************** 
 * 
 * 
 *
 */
void loop() {
  DataLogtoSD();//LOG DATA TO SD CARD

  LocationInit++;
  if (LocationInit > 1) { //wait until retreived location data from server to be settled
    LocationDifferenceCalculator();
  }





  isManualControlActive = BoolStringtoBoolean(WSisManualControlActive());
  Serial.println("isManualControlActive: "+BooltoStr(isManualControlActive));

  AirConCondition = BoolStringtoBoolean(WSAirConCondition());
  Serial.println("AirCon Condition:"+BooltoStr(AirConCondition));

  LightCondition = BoolStringtoBoolean(WSLightConCondition());
 Serial.println("Light Condition: "+BooltoStr(LightCondition));

  LDRfunction();//CHECK LDR

  tempFunction();//CHECK TEMP
  
  WindowsCondition = BoolStringtoBoolean(WSWindowsCondition());
  Serial.println("Windows Condition: "+BooltoStr(WindowsCondition));


  rainFunction();//CHECK RAIN

  windowsConditionCheck();//CHECK WINDOWS' DESIRED CONDITION

  manualTempSet = WSmanualTempSet().toDouble();
  Serial.print("manualTempSet: ");Serial.println(manualTempSet);

  userLocationLat = WSLatitude().toFloat();
  Serial.print("Latitude: ");Serial.println(userLocationLat);

  userLocationLong = WSLongitude().toFloat();
  Serial.print("Longitude: ");Serial.println(userLocationLong);

 Serial.print("Location Difference: ");Serial.print(LocationDifference);Serial.println(" km");






}

/*
 * 
 * 
 *  END OF MAIN LOOP
 * 
 * 
 */




/*Wifi Communication*********************************************************************************************************************************************Wifi******************************************************








 **************************************************/


String WSLatitude()    //Latitude
{
  delay(2000);

  char inv = '"';
  String add = "AT+CIPSTART";
  add += "=";
  add += inv;
  add += "TCP";   // type of connection
  add += inv;
  add += ",";
  add += inv;
  add += "api.thingspeak.com";  // host server
  add += inv;
  add += ",";
  add += "80";   // port
  add += "\r\n";
  sendData(add, 3000, 0);

  String rest = "AT+CIPSEND=90";//CHARACTERS
  rest += "\r\n";   // carriage return and new line
  sendData(rest, 1000, 0);


  String hostt = "GET /apps/thinghttp/send_request?api_key=IVDLE3W5TXRIYE6W";//REQUESTED DATA
  hostt += "\r\n";
  hostt += "Host:api.thingspeak.com";
  hostt += "\r\n\r\n\r\n\r\n\r\n";
  String Temp = sendData(hostt, 2000, 0);
  

    String myString = "";
  String latiCount="";
  int latiCounti="";

  for (int i = 3; i < 25; i++) {

    myString = String(i);
    
    if (Temp.indexOf(myString + ":") > 0) {// THE STRING RESULT WE WANT 

      latiCount=myString;
      latiCounti=i;

      //Serial.println(longiCount);
       // Serial.println(longiCounti);
      
      
    }
 
  }

  String latiWifi= latiCount +":";
  

   if (Temp.indexOf(latiWifi) > 0) {

      Temp = Temp.substring(Temp.indexOf(latiWifi) + 3, Temp.indexOf(latiWifi) + 3 + latiCounti);//EQUATE THE SERVER GOT RESULT ARDUINO'S VALUES
      backupLatitude = Temp;
    } else {

      Temp = backupLatitude;
    }       
    myString = "";
   latiCount="";
   latiCounti="";
    latiWifi="";
 


  return (Temp);

}




String WSLongitude()  //Longitude
{
  delay(2000);

  char inv = '"';
  String add = "AT+CIPSTART";
  add += "=";
  add += inv;
  add += "TCP";   // type of connection
  add += inv;
  add += ",";
  add += inv;
  add += "api.thingspeak.com";  // host server
  add += inv;
  add += ",";
  add += "80";   // port
  add += "\r\n";
  sendData(add, 3000, 0);

  String rest = "AT+CIPSEND=90";
  rest += "\r\n";   // carriage return and new line
  sendData(rest, 1000, 0);


  String hostt = "GET /apps/thinghttp/send_request?api_key=TX9V6XVITX0J5XV3";
  hostt += "\r\n";
  hostt += "Host:api.thingspeak.com";
  hostt += "\r\n\r\n\r\n\r\n\r\n";
  String Temp = sendData(hostt, 2000, 0);
 

  String myString = "";
  String longiCount="";
  int longiCounti="";

  for (int i = 3; i < 25; i++) {

    myString = String(i);
    
    if (Temp.indexOf(myString + ":") > 0) {

      longiCount=myString;
      longiCounti=i;

      //Serial.println(longiCount);
       // Serial.println(longiCounti);
      
      
    }
 
  }

  String longitudeWifi= longiCount +":";
  

   if (Temp.indexOf(longitudeWifi) > 0) {

      Temp = Temp.substring(Temp.indexOf(longitudeWifi) + 3, Temp.indexOf(longitudeWifi) + 3 + longiCounti);
      backupLongitude = Temp;
    } else {

      Temp = backupLongitude;
    }

       myString = "";
   longiCount="";
   longiCounti="";
    longitudeWifi="";

  return (Temp);



}

String WSmanualTempSet()  //manualTempSet
{

  delay(2000);
  char inv = '"';
  String add = "AT+CIPSTART";
  add += "=";
  add += inv;
  add += "TCP";   // type of connection
  add += inv;
  add += ",";
  add += inv;
  add += "api.thingspeak.com";  // host server
  add += inv;
  add += ",";
  add += "80";   // port
  add += "\r\n";
  sendData(add, 3000, 0);

  String rest = "AT+CIPSEND=90";
  rest += "\r\n";   // carriage return and new line
  sendData(rest, 1000, 0);


  String hostt = "GET /apps/thinghttp/send_request?api_key=KV5WMX4M2HOAUHUA";
  hostt += "\r\n";
  hostt += "Host:api.thingspeak.com";
  hostt += "\r\n\r\n\r\n\r\n\r\n";
  String Temp = sendData(hostt, 2000, 0);



  if (Temp.indexOf("1:") > 0) {//SEARCH TEMPERATURE

    Temp = Temp.substring(Temp.indexOf("1:") + 2, Temp.indexOf("1:") + 2 + 1);
    backupmanualTempSet = Temp;
  } else  if (Temp.indexOf("2:") > 0) {

    Temp = Temp.substring(Temp.indexOf("2:") + 2, Temp.indexOf("2:") + 2 + 2);
    backupmanualTempSet = Temp;

  } else  if (Temp.indexOf("3:") > 0) {

    Temp = Temp.substring(Temp.indexOf("3:") + 2, Temp.indexOf("3:") + 2 + 3);
    backupmanualTempSet = Temp;

  } else {

    Temp = backupmanualTempSet;
  }


  return (Temp);



}


///////


String WSAirConCondition()  //AirConCondition
{
  delay(2000);

  char inv = '"';
  String add = "AT+CIPSTART";
  add += "=";
  add += inv;
  add += "TCP";   // type of connection
  add += inv;
  add += ",";
  add += inv;
  add += "api.thingspeak.com";  // host server
  add += inv;
  add += ",";
  add += "80";   // port
  add += "\r\n";
  sendData(add, 3000, 0);

  String rest = "AT+CIPSEND=90";
  rest += "\r\n";   // carriage return and new line
  sendData(rest, 1000, 0);


  String hostt = "GET /apps/thinghttp/send_request?api_key=4WPTNNVAO5FEOZGG"; // 
  hostt += "\r\n";
  hostt += "Host:api.thingspeak.com";
  hostt += "\r\n\r\n\r\n\r\n\r\n";
  String Temp = sendData(hostt, 2000, 0);



  if (Temp.indexOf("false") > 0) { // SEARCH THE RECEIVED STRING

    Temp = "false";
    backupAirConCondition = Temp;
  } else  if (Temp.indexOf("true") > 0) {

    Temp = "true";
    backupAirConCondition = Temp;

  } else {

    Temp = backupAirConCondition;
  }


  return (Temp);



}


String WSLightConCondition()  //LightConCondition
{

  delay(2000);
  char inv = '"';
  String add = "AT+CIPSTART";
  add += "=";
  add += inv;
  add += "TCP";   // type of connection
  add += inv;
  add += ",";
  add += inv;
  add += "api.thingspeak.com";  // host server
  add += inv;
  add += ",";
  add += "80";   // port
  add += "\r\n";
  sendData(add, 3000, 0);

  String rest = "AT+CIPSEND=90";
  rest += "\r\n";   // carriage return and new line
  sendData(rest, 1000, 0);


  String hostt = "GET /apps/thinghttp/send_request?api_key=3A1JL1AGNYDSUCKN";
  hostt += "\r\n";
  hostt += "Host:api.thingspeak.com";
  hostt += "\r\n\r\n\r\n\r\n\r\n";
  String Temp = sendData(hostt, 2000, 0);



  if (Temp.indexOf("false") > 0) {

    Temp = "false";
    backupLightConCondition = Temp;
  } else  if (Temp.indexOf("true") > 0) {

    Temp = "true";
    backupLightConCondition = Temp;

  } else {

    Temp = backupLightConCondition;
  }


  return (Temp);



}

String WSWindowsCondition()  //WindowsCondition
{
  delay(2000);

  char inv = '"';
  String add = "AT+CIPSTART";
  add += "=";
  add += inv;
  add += "TCP";   // type of connection
  add += inv;
  add += ",";
  add += inv;
  add += "api.thingspeak.com";  // host server
  add += inv;
  add += ",";
  add += "80";   // port
  add += "\r\n";
  sendData(add, 3000, 0);

  String rest = "AT+CIPSEND=90";
  rest += "\r\n";   // carriage return and new line
  sendData(rest, 1000, 0);


  String hostt = "GET /apps/thinghttp/send_request?api_key=5KJEIP66OLVXCGGK";
  hostt += "\r\n";
  hostt += "Host:api.thingspeak.com";
  hostt += "\r\n\r\n\r\n\r\n\r\n";
  String Temp = sendData(hostt, 2000, 0);



  if (Temp.indexOf("false") > 0) {

    Temp = "false";
    backupWindowsCondition = Temp;
  } else  if (Temp.indexOf("true") > 0) {

    Temp = "true";
    backupWindowsCondition = Temp;

  } else {

    Temp = backupWindowsCondition;
  }


  return (Temp);



}




String WSisManualControlActive()  //isManualControlActive
{

  delay(2000);
  char inv = '"';
  String add = "AT+CIPSTART";
  add += "=";
  add += inv;
  add += "TCP";   // type of connection
  add += inv;
  add += ",";
  add += inv;
  add += "api.thingspeak.com";  // host server
  add += inv;
  add += ",";
  add += "80";   // port
  add += "\r\n";
  sendData(add, 3000, 0);// establish a TCP connection to communicate with servers via port 80

  String rest = "AT+CIPSEND=90";//how many number of characters we need to request. 
  rest += "\r\n";   // carriage return and new line
  sendData(rest, 1000, 0);


  String hostt = "GET /apps/thinghttp/send_request?api_key=UAV485BSK2N0TA4E";
  hostt += "\r\n";
  hostt += "Host:api.thingspeak.com";
  hostt += "\r\n\r\n\r\n\r\n\r\n";
  String Temp = sendData(hostt, 2000, 0);//Request the api generated DATA from thingspeak.com. As we want to get the data from the server, we will send a GET requesT



  if (Temp.indexOf("false") > 0) {

    Temp = "false";
    backupisManualControlActive = Temp;
  } else  if (Temp.indexOf("true") > 0) {

    Temp = "true";
    backupisManualControlActive = Temp;

  } else {

    Temp = backupisManualControlActive;
  }


  return (Temp);



}





String sendData(String command, const int Goldout, boolean debug)// THE FUNCTION THAT RECEIVES AND SENDS THE DATA
{
  String response = "";//our response

  Serial1.print(command); // send the read character to the Serial1

  long int Gold = millis();//wait

  while ( (Gold + Goldout) > millis())
  {
    while (Serial1.available())
    {

      // The esp has data so display its output to the serial window
      char c = Serial1.read(); // read the next character.
      response += c;
    }
  }

  if (debug)
  {

    Serial.println(response);// the received data

  }

  return response;//return received data

}


void connectToWiFi() {

  sendData("AT+RST\r\n", 2000, 1);
  sendData("AT+CWQAP\r\n", 5000, 1); // Disconnect WiFi if connected.
  char inv = '"';
  sendData("AT+CWMODE=1\r\n", 1000, 1); // configure as  station mode.
  delay(3000);

  sendData("AT+CWQAP\r\n", 4000, 1); // again quit the connection if any.
  String conn = "AT+CWJAP";
  conn += "=";
  conn += inv;
  conn += Host_name;  // Host name
  conn += inv;
  conn += ",";
  conn += inv;
  conn += password; // Password
  conn += inv;
  conn += "\r\n\r\n";
  sendData(conn, 7000, 1); // Join the connection.( AT+CWJAP = "Host_name","password" )



}


/*End of Wifi Communication*********************************************************************************************************************************************Wifi******************************************************








 **************************************************/




 /*
  * 
  * 
  * 
  * 
  * ********************MAIN FUNCTIONS************
  * 
  */




void LDRfunction() {

  LDRValue = analogRead(LDRPin1);      //reads the ldr’s value through LDR
  LDRValue2 = analogRead(LDRPin2);      //reads the ldr’s value through LDR
 // Serial.println(LDRValue);       //prints the LDR values to serial monitor
  //Serial.println(LDRValue2);       //prints the LDR values to serial monitor
  delay(50);        //This is the speed by which LDR sends value to arduino

  int LDRValueAverage = (LDRValue + LDRValue2) / 2;
  // Serial.println(LDRValueAverage);       //prints the LDR values to serial monitor

  if (isManualControlActive == false) {

    if (LDRValueAverage < light_sensitivity)// if the read value smaller than 500 that means there is light
    {
      LDRcondition = true; //sunset


      //Serial.println(LDRcondition);

      // if (LightCondition == true) {
      relayFunction(1, true);
      LightCondition = false;
      // }



    } else if (LDRValueAverage > light_sensitivity) {

      LDRcondition = false; //sundown
      //Serial.println(LDRcondition);
      // if (LightCondition==false){
      if (LightCondition == false && IdealLocationDifference > LocationDifference ) { // distance of user from home
        relayFunction(1, false);
        LightCondition = true;

      } else if (IdealLocationDifference < LocationDifference ) { // distance of user from home
        relayFunction(1, true);
        LightCondition = false;

      }
    }
  } else if (isManualControlActive == true) {
    if (LightCondition == true) {
      relayFunction(1, false);
    } else {
      relayFunction(1, true);
    }
  }

}


void rainFunction() {


  rainSensorValue = analogRead(rainSensorPin);
  //Serial.println(rainSensorValue);


  int range = map(analogRead(rainSensorPin), 0, 1024, 0, 2);// map the read voltage to 0,1,2 if the read voltage bigger than 341 there is no rain

  // range value:


  if (range == 0) {

    // Sensor getting wet
    // Serial.println("Rain Warning");

    RainCondition = true;
    //WindowsCondition = false;
    //servoFunction("CloseWindows");

    //servoFunction("Rain");

  } else if (range == 1) {
    //dry
    //Serial.println("Not Raining");
    RainCondition = false;
  }




  delay(10);  // delay between read

}


void tempFunction() {
  delay(200);


  for (int i = 0; i < 3; i++) {// make three sample to increase precision the last arrived data will set to actual value

    float  analogVinside = analogRead(tempPinInside);
    analogVinside = (analogVinside / 1023) * 5000; //convert analog to mili voltage
    TemperatureMeasuredInside = analogVinside / 10, 0; // mV to temperature

    //Serial.print("iAA");
    //Serial.print("\t");
//    Serial.println(TemperatureMeasuredInside);

  }

  if (oldTemperatureIns != 0) {//filtering peak values
    if (abs(oldTemperatureIns - TemperatureMeasuredInside) / abs(oldTemperatureIns) < 0.20) {
      oldTemperatureIns = TemperatureMeasuredInside;
    } else {
      TemperatureMeasuredInside = oldTemperatureIns;
    }
  } else {
    oldTemperatureIns = TemperatureMeasuredInside;
  }



 // Serial.print("InsideTemp");
 // Serial.print("\t");
  //Serial.println(TemperatureMeasuredInside);
  for (int i = 0; i < 3; i++) {

    float  analogVoutside = analogRead(tempPinOutside);
    analogVoutside = (analogVoutside / 1023) * 5000; //convert analog to mili voltage
    TemperatureMeasuredOutside = analogVoutside / 10, 0; // mV to temperature

    //Serial.print("oAA");
   // Serial.print("\t");
   // Serial.println(TemperatureMeasuredOutside);

  }



  if (oldTemperatureOut != 0) {//filtering peak values
    if (abs(oldTemperatureOut - TemperatureMeasuredOutside) / abs(oldTemperatureOut) < 0.2) {
      oldTemperatureOut = TemperatureMeasuredOutside;
    } else {
      TemperatureMeasuredOutside = oldTemperatureOut;
    }
  } else {
    oldTemperatureOut = TemperatureMeasuredOutside;
  }


  //Serial.print("OutsideTemp");
 // Serial.print("\t");
 // Serial.println(TemperatureMeasuredOutside);

  if (isManualControlActive == false) {

    if (manualTempSet < TemperatureMeasuredInside  && IdealLocationDifference > LocationDifference) {
      relayFunction(2, false); // open air fan
      AirConCondition = true;

      //      if (WindowsCondition == true) {
      //
      //        WindowsCondition = false;
      //      }


    } else if (IdealLocationDifference < LocationDifference ) { // distance of user from home
      relayFunction(2, true);
      AirConCondition = false;

    } else if (manualTempSet > TemperatureMeasuredInside ) {




      relayFunction(2, true);
      AirConCondition = false;


      delay(100);



      //      if (TemperatureMeasuredOutside >= manualTempSet && RainCondition == false && IdealLocationDifference > LocationDifference) {
      //
      //        WindowsCondition = true;
      //      }


    }
  } else if (isManualControlActive == true) {
    if (AirConCondition == true) {
      relayFunction(2, false);

    } else {
      relayFunction(2, true);
    }

    delay(10);
  }


}


void windowsConditionCheck() {// check the windows condition each cycle of loop

  if (WindowsCondition == true) {
    servoFunction("OpenWindows");// servo 1

  } else if (WindowsCondition == false) {
    servoFunction("CloseWindows"); // servo 2
  }


}

void servoFunction(String text) {


  int openValue = 150;
  int closeValue = 0;

  if (isManualControlActive == false) {

    if (RainCondition == true && WindowsCondition == true || text.equals("CloseWindows")) {

      windowServo.write( closeValue ); // sets the servo position

    } else if (text.equals("OpenWindows") && RainCondition == false) {

      windowServo.write( openValue ); // sets the servo position

    }
  } else if (isManualControlActive == true) {

    if (WindowsCondition == true) {
      windowServo.write( openValue );
    } else if (WindowsCondition == false) {
      windowServo.write( closeValue );
    }
  }

}



void relayFunction(int relayNumber, bool cond) {// relays are low triggerd when the digital value set to 0 it means above two pins of relay is connected (NO)

  if ( relayNumber == 1 ) { // Light
    if (cond == true) {
      digitalWrite(relayPin1, HIGH);
    } else if (cond == false) {
      digitalWrite(relayPin1, LOW);
    }
  }

  if ( relayNumber == 2 ) { // Air Fan
    if (cond == true) {
      digitalWrite(relayPin2, HIGH);
    } else if (cond == false) {
      digitalWrite(relayPin2, LOW);
    }
  }

  if ( relayNumber == 3 ) { // Empty
    if (cond == true) {
      digitalWrite(relayPin3, LOW);
    } else if (cond == false) {
      digitalWrite(relayPin3, HIGH);
    }
  }

}




bool BoolStringtoBoolean(String str) {// boolean to string converter

  if (str == "true") {

    return true;
  } else if (str == "false") {

    return false;
  }


}

String BooltoStr(bool a) {// string to boolean converter

  if (a) {

    return "true";
  } else if (!a) {

    return "false";
  }

}

void LocationDifferenceCalculator() {// location difference calculator in terms from latitude and longitude

  float pi = 3.14159265359;


  float R = 6371;  //kilometres earths radius
  float latr1 = userLocationLat * (pi / 180);
  float latr2 = CompLocationLat * (pi / 180);
  float deltaLat = (CompLocationLat - userLocationLat) * (pi / 180);
  float deltaLong = (userLocationLong - CompLocationLong) * (pi / 180);

  float a = (sin(deltaLat / 2) * sin(deltaLat / 2)) + cos(latr1) * cos(latr2) * sin(deltaLong / 2) * sin(deltaLong / 2);

  float c = 2 * atan2(sqrt(a), sqrt(1 - a));




  LocationDifference =  R * c;


}

void print2digits(int number) {
  if (number >= 0 && number < 10)
    Serial.write('0');
  Serial.print(number);
}

void DataLogtoSD() {// data to sd card printer

  myFile = SD.open("DataAnal.txt", FILE_WRITE);
  if (myFile) {

    if (LineCounter == 0) {

      myFile.println("Time,Date,RainSensor,TempInside,TempOutside,LDR1,LDR2");// initial header
    } else {

      //Time &Date
      if (! RTC.read(tm)) {

        if (tm.Hour >= 0 && tm.Hour < 10) {
          myFile.print('0');
        }
        myFile.print(tm.Hour);
        myFile.print(':');
        if (tm.Minute >= 0 && tm.Minute < 10) {
          myFile.print('0');
        }
        myFile.print(tm.Minute);
        myFile.print(':');
        if (tm.Second >= 0 && tm.Second < 10) {
          myFile.print('0');
        }
        myFile.print(tm.Second);

        myFile.print(",");
        //Date

        myFile.print(tm.Day);
        myFile.print('/');
        myFile.print(tm.Month);
        myFile.print('/');

      myFile.print(tmYearToCalendar(tm.Year));

       // myFile.print("2018,");


      } else {
        Serial.println("DS1302 read error!  Please check the circuitry.");
      }

      //RainSensor

      myFile.print(rainSensorValue);
      myFile.print(",");



      //Temperature
      myFile.print(TemperatureMeasuredInside);
      myFile.print(",");
      myFile.print(TemperatureMeasuredOutside);
      myFile.print(",");


      //LDR

      myFile.print(LDRValue);
      myFile.print(",");
      myFile.println(LDRValue2);



    }

    LineCounter++;
    myFile.close(); // close the file  // the written values seperated with comma on excel they will be seperated using retrived data from TXT file with comma separation method
  }
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening test.txt");
  }
  delay(3000);



}



