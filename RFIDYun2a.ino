
/*****************************************************************
Phant_Yun.ino
Post data to SparkFun's data stream server system (phant) using
an Arduino Yun
Jim Lindblom @ SparkFun Electronics
Original Creation Date: July 3, 2014

This sketch uses an Arduino Yun to POST sensor readings to 
SparkFun's data logging streams (http://data.sparkfun.com). A post
will be initiated whenever pin 3 is connected to ground.

curl example from:
https://github.com/sparkfun/phant/blob/master/examples/sh/curl_post.sh

Distributed as-is; no warranty is given.
*****************************************************************/

// Controls a StrongLink SL018 or SL030 RFID reader by I2C
// Arduino to SL018/SL030 wiring:
// A3/TAG     1      5
// A4/SDA     2      3
// A5/SCL     3      4
// 5V         4      -
// GND        5      6
// 3V3        -      1
// Marc Boon <http://www.marcboon.com>

// Process.h gives us access to the Process class, which can be
// used to construct Shell commands and read the response.
#include <Process.h>
#include <Wire.h>
/////////////////
// Phant Stuff //
/////////////////
// URL to phant server (only change if you're not using data.sparkfun
// String phantURL = "http://data.sparkfun.com/input/";
String phantURL = "http://data.sparkfun.com/input/";
// Public key (the one you see in the URL):
String publicKey = "0lzWz1nqKaIqbYxlXn7l";
// Private key, which only someone posting to the stream knows
String privateKey = "D6n7nDkMYlFY1Exby4Mb";
// How many data fields are in your stream?
const int NUM_FIELDS = 3;
// What are the names of your fields?
String fieldName[NUM_FIELDS] = {"name", "number", "truck"};
// We'll use this array later to store our field data
String fieldData[NUM_FIELDS];

            char tagString[9];
            int index = 0;
            boolean reading = false;

//Register your RFID tags here
String tag1 = "F64708FC";
String tag2 = "56990AFC";
String tag3 = "9BD8AF9B";
String tag4 = "82D51E58"; 


////////////////
// Pin Inputs //
////////////////
  String driverName;
  int RFID_number;
  int truckID = 6486;
  
  // rfid tag pin 
  #define TAG A3 // A3
  byte data;
  //int value = 0;
  int LED1 = 13;
  
  int redPin = 11;
  int bluePin = 5;
  int greenPin = 6;
  
     // The 5 cards that will be listed in this example 
            boolean tag1Card;
            boolean tag2Card;
            boolean tag3Card;
            boolean tag4Card;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  SETUP
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() 
{
  Bridge.begin();
  Wire.begin();
  Serial.begin(115200);
  
  pinMode(TAG, INPUT);
  pinMode(LED1, OUTPUT); 
  
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  
  setColor(255, 0, 0);  // red
  delay(1000);
  Serial.println("=========== Ready ===========");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  LOOP
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{             
  
  // Wait for tag
  while(digitalRead(TAG));

  // Read tag ID
  readID();

  // Wait until tag is gone
  while(!digitalRead(TAG)); 
  
  delay(100);
  
}
 
 
   
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  COLOR LED
///////////////////////////////////////////////////////////////////////////////////////////////////////// 
  void setColor(int red, int green, int blue)
{
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}
  
  
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  POST DATA
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void postData()
{
  Process phant; // Used to send command to Shell, and view response
  String curlCmd; // Where we'll put our curl command
  String curlData[NUM_FIELDS]; // temp variables to store curl data
  
  // Construct curl data fields
  // Should look like: --data "fieldName=fieldData"
  for (int i=0; i<NUM_FIELDS; i++)
  {
    curlData[i] = "--data \"" + fieldName[i] + "=" + fieldData[i] + "\" ";
  }

  // Construct the curl command:
  curlCmd = "curl ";
  curlCmd += "--header "; // Put our private key in the header.
  curlCmd += "\"Phant-Private-Key: "; // Tells our server the key is coming
  curlCmd += privateKey; 
  curlCmd += "\" "; // Enclose the entire header with quotes.
  for (int i=0; i<NUM_FIELDS; i++)
  curlCmd += curlData[i]; // Add our data fields to the command
  curlCmd += phantURL + publicKey; // Add the server URL, including public key

  // Send the curl command:
  Serial.print("Sending command: ");
  Serial.println(curlCmd); // Print command for debug
  
  // String composedData = ("http://data.sparkfun.com/input/0lzWz1nqKaIqbYxlXn7l?private_key=D6n7nDkMYlFY1Exby4Mb&name=", driverName, "&number=", String(RFID_number), "&truck", String(truckID));
  // phant.runShellCommand(composedData);
  phant.runShellCommand(curlCmd); // Send command through Shell
 
  // Read out the response:
  Serial.print("Response: ");
  // Use the phant process to read in any response from Linux:
  while (phant.available())
  {
    char c = phant.read();
    Serial.write(c);
  }
  
  phant.close();
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  READ ID
/////////////////////////////////////////////////////////////////////////////////////////////////////////


  int readID()
  {
  // Serial.println("readID");
  // Send SELECT command
  Wire.beginTransmission(0x50);
  Wire.write(1);
  Wire.write(1);
  Wire.endTransmission();
  
  // ***+++++++++++++++++++++++++
  // Wait for response
  while(!digitalRead(TAG))
  {   
    // Allow some time to respond
    delay(5);
    //Serial.println("wait for response, !digitalRead(TAG)");
    // Anticipate maximum packet size
    Wire.requestFrom(0x50, 11); // 11 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if(Wire.available())
    {     
      // Get length of packet
      byte len = Wire.read();
      
      // Wait until whole packet is received
      while(Wire.available() < len)
      {
        // Quit if no response before tag left
        if(digitalRead(TAG)) return 0; 
        Serial.print(TAG);  
      }

      // Read command code, should be same as request (1)
      byte command = Wire.read();
      if(command != 1) return -1;

      // Read status
      byte status = Wire.read();
      switch(status)
      {
        case 0: // Succes, read ID and tag type
        {
          len -= 2;
          // Get tag ID
          while(--len)
          {
            byte data = Wire.read();
            if(data < 0x10) Serial.print(0);
           
            if (data == 0xF6 && 0x47 && 0x08 && 0xFC) tag1Card = true;  
            if (data == 0x56 && 0x99 && 0x0A && 0xFC) tag2Card = true;
            if (data == 0x9B && 0xD8 && 0xAF && 0x9B) tag3Card = true; 
             
            Serial.print(data, HEX);
            
            }
            Serial.println();
            checkID();
        }     
      return 1;
      
      case 0x0A: // Collision
        Serial.println("Collision detected");
        break;

      case 1: // No tag
        Serial.println("No tag found");
        break;

      default: // Unexpected
        Serial.println("Unexpected result");
      }
      return -1;
    }
  }
  // No tag found or no response
  return 0;
      Serial.println("no tag");
  }
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CHECK ID
/////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  void checkID(){
    
    if (tag1Card == true) {
                   setColor(0, 255, 0);  // green
                   driverName = "Driver 1";
                   fieldData[0] = driverName;
                   fieldData[1] = String(tag1);
                   fieldData[2] = String(truckID);
                   postData(); // the postData() function does all the work
                   delay(1000);
    } 
    
    if (tag2Card == true) {
                   setColor(0, 255, 0);  // green
                   driverName = "Driver 2";
                   fieldData[0] = driverName;
                   fieldData[1] = String(tag2);
                   fieldData[2] = String(truckID);
                   postData(); 
                   delay(1000);
    } 
   
    if (tag3Card == true) {
                   setColor(0, 255, 0);  // green
                   driverName = "Driver 3";
                   fieldData[0] = driverName;
                   fieldData[1] = String(tag3);
                   fieldData[2] = String(truckID);
                   postData();
                   delay(1000);
    }
    
    {
      Serial.println("Card detected");
  }
  }  
