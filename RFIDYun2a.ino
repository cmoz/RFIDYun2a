language:c
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
// Process.h gives us access to the Process class, which can be
// used to construct Shell commands and read the response.
#include <Process.h>

/////////////////
// Phant Stuff //
/////////////////
// URL to phant server (only change if you're not using data.sparkfun
String phantURL = "http://data.sparkfun.com/input/";
// Public key (the one you see in the URL):
String publicKey = "6JZbNolApzF4om2l9yYK";
// Private key, which only someone posting to the stream knows
String privateKey = "Ww0vPW1yrkUNDqWPV9jE";
// How many data fields are in your stream?
const int NUM_FIELDS = 3;
// What are the names of your fields?
String fieldName[NUM_FIELDS] = {"name", "number", "truck"};
// We'll use this array later to store our field data
String fieldData[NUM_FIELDS];

////////////////
// Pin Inputs //
////////////////
const int triggerPin = 3;
const int lightPin = A0;
const int switchPin = 5;

String name = "Yun-anon";
boolean newName = true;

void setup() 
{
  Bridge.begin();
  Serial.begin(115200);

  // Setup Input Pins:
  pinMode(triggerPin, INPUT_PULLUP);
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(lightPin, INPUT_PULLUP);

  Serial.println("=========== Ready to Stream ===========");
  Serial.println("Press the button (D3) to send an update");
  Serial.println("Type your name, followed by '!' to update name");
}

void loop()
{  
  // If the trigger pin (3) goes low, send the data.
  if (!digitalRead(triggerPin))
  {
    // Gather Data
    fieldData[0] = String(analogRead(lightPin));
    fieldData[1] = String(digitalRead(switchPin));
    fieldData[2] = name;

    // Post Data
    Serial.println("Posting Data!");
    postData(); // the postData() function does all the work, 
                // see below.
    delay(1000);
  }

  // Check for a new name input:
  if (Serial.available())
  {
    char c = Serial.read();
    if (c == '!')
    {
      newName = true;
      Serial.print("Your name is ");
      Serial.println(name);
    }
    else if (newName)
    {
      newName = false;
      name = "";
      name += c;
    }
    else
    {
      name += c;
    }
  }
}

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
  phant.runShellCommand(curlCmd); // Send command through Shell

  // Read out the response:
  Serial.print("Response: ");
  // Use the phant process to read in any response from Linux:
  while (phant.available())
  {
    char c = phant.read();
    Serial.write(c);
  }
}
