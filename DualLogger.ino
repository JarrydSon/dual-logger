//Now part of a repository on GitHub :D
//Could probably do with better code. I don't like the way I've done things. So inefficient.
/*
 The circuit:
 * analog sensors on analog 0
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
*/

#include <SPI.h>
#include <SD.h>
#include <TFT.h>

#define SD_CS 4
#define LD_CS 10
#define CD 9
#define RESET 8

TFT screen = TFT(LD_CS, CD, RESET);

//===================================GLOBAL VARIABLES===============================================================
int lineCountV = 0;
int fileCountV = 0;
int lineCountT = 0;
int fileCountT = 0;
int xPos = 0;
String fileName = "";
String dataString = "";
char charFileNameV[12] = "vltlog.log";
char charFileNameT[12] = "tmplog.log";
char choice;
char menu [100];
String strMenu = "Enter:\n\t'B' to begin\n\t'E' to exit\n\t'T' Temperature\n\t'V' Voltage";

File dataFile;


//========================================SETUP======================================================================

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  pinMode (3, INPUT_PULLUP);
  attachInterrupt (1, buttonTemperature, FALLING); //Interrupts on the buttons
  pinMode (2, INPUT_PULLUP);
  attachInterrupt(0,buttonVoltage, FALLING);


  screen.begin();
  screen.background (0, 0, 0);
  screen.stroke (0, 255, 0);
  screen.setRotation(2);
  strMenu.toCharArray(menu, 100);
  screen.text(menu, 0, 0);
  Serial.println (strMenu);
  while (choice != 'B') {
    choice = Serial.read();
  }

  // see if the card is present and can be initialized:
  screen.stroke (0, 0, 0);
  screen.text(menu, 0, 0);

  if (!SD.begin(SD_CS)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

}

//===================================================MAIN LOOP====================================================
void loop()
{
  if (choice != 'E')
  {
    getSensorData(0);
    writeToFile("Voltage");
    updateCount("Voltage");

    getSensorData(1);
    writeToFile("Temperature");
    updateCount("Temperature");

    if (choice == 'V')
    {
      displayGraph(0);
    }
    else
    {
      displayGraph(1);
    }
    delay(1000);
  }
  else {
    dataFile.close();
    while (1) {}
  }
}

//=========================================================FUNCTIONS===============================================
void buttonVoltage()
{
   choice = 'V';
   screen.background(0, 0, 0);
   xPos = 0;
}

void buttonTemperature()
{
   choice = 'T';
   screen.background(0, 0, 0);
   xPos = 0;
}
void displayGraph (int pin)
{
  int sensor = 0;
  sensor = analogRead(pin);
  int drawHeight = map(sensor, 0, 1023, 0, screen.height());
  switch (pin) {
    case 0:
      screen.stroke (255, 0, 0);
      break;
    case 1:
      screen.stroke (0, 255, 120);
      break;
    default:
      screen.stroke (255, 255, 255);
      break;
  }
  if (xPos >= screen.width()) {
    xPos = 0;
    screen.background(0, 0, 0);
  }
  else {
    // increment the horizontal position:
    xPos++;
  }

  screen.line(xPos, screen.height() - drawHeight, xPos, screen.height());

  delay(16);
}

void getSensorData(int pin)
{
  int sensor = 0;
  sensor = analogRead(pin);
  dataString = (String(millis() / 1000));
  dataString += (",");
  dataString += String(sensor);
}

void writeToFile(String fileType)
{
  if (fileType == "Voltage") {
    dataFile = SD.open(charFileNameV, FILE_WRITE);
  }
  else {
    dataFile = SD.open(charFileNameT, FILE_WRITE);
  }
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.print("Line: ");
    if (fileType == "Voltage")
    {
      Serial.print (lineCountV);
    }
    else
    { Serial.print (lineCountT);
    }
    Serial.print(" Data: ");
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.print("error opening: ");
    Serial.println(fileName);
  }
}

void updateCount(String fileType)
{
  if (fileType == "Voltage")
  {
    lineCountV ++;
    if (lineCountV >= 50) //if the line count exceeds 50 then fileCount will be incremented and thus a new file should be created.
    {
      lineCountV = 0;
      fileCountV++;
      fileName = ("vltlog");
      fileName += String(fileCountV);
      fileName += ".log";
      fileName.toCharArray (charFileNameV, 12);
      Serial.println("Saving to new file: " + fileName);
    }
  }
  else
  {
    lineCountT ++;
    if (lineCountT >= 50) //if the line count exceeds 50 then fileCount will be incremented and thus a new file should be created.
    {
      lineCountT = 0;
      fileCountT++;
      fileName = ("tmplog");
      fileName += String(fileCountT);
      fileName += ".log";
      fileName.toCharArray (charFileNameT, 12);
      Serial.println("Saving to new file: " + fileName);
    }
  }
}

void serialEvent()
{
  while (Serial.available())
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    choice = inChar;
    screen.background(0, 0, 0);
    xPos = 0;
  }
}







