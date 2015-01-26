/*
 The circuit:
 *The battery voltage is to be measured on A0. The 24V (25.2V ~ maximum) bus voltage must be stepped down to a range between 0-5V before using it as an input.
 *A suggested method is to use a resistor voltage divider to divide the voltage by 6. A 5V zener can also be used to clamp the voltage if it does exceed the 5V maximum. 
 *An LM35 temperature sensor output on A1. The voltage is a linear function of temperature. The datasheet states 10mV/degreeCelcius.
 * Arduino TFT + SD module is attached to the SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** SD_CS - pin 4
 ** LD_CS - pin 10
 ** CD - pin 9
 ** RESET - pin 8
 ** BL - +5V
*/

#include <SPI.h>
#include <SD.h>
#include <TFT.h>

//Pin defines
#define SD_CS 4
#define LD_CS 10
#define CD 9
#define RESET 8

//Defining options as constants
#define VOLTAGE 0
#define TEMPERATURE 1

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
char state;
char menu [100];
String strMenu = "Enter:\n\t'B' to begin\n\t'E' to exit\n\t'T' Temperature\n\t'V' Voltage";




//========================================SETUP======================================================================

void setup()
{
  initPins();
  initScreen();

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println (strMenu);

  while (state != 'B') //Wait for user to enter option to begin
  {
    state = Serial.read();
  }

  // Erase menu
  screen.stroke (0, 0, 0);
  screen.text(menu, 0, 0);

  //Initialise SD card if present
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
  if (state != 'E')
  {
    writeToSD();
    displayGraph();
    delay(2000);
  }
  else
    while (1) {}//Do nothing

}

//=========================================================FUNCTIONS===============================================

//Function to initialise the necessary pins
void initPins ()
{
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  pinMode (3, INPUT_PULLUP);
  pinMode (2, INPUT_PULLUP);

  attachInterrupt (1, buttonTemperature, FALLING); //Interrupts on the buttons
  attachInterrupt(0, buttonVoltage, FALLING);
}

//Function to initialise the LCD screen
void initScreen()
{
  screen.begin();
  screen.background (0, 0, 0); //Black background
  screen.stroke (0, 255, 0); //Green font
  screen.setRotation(2); //Verticle screen orientation
  strMenu.toCharArray(menu, 100);
  screen.text(menu, 0, 0);//Diplay the menu on the LCD screen
}

//Function to display the data on the LCD screen
void displayGraph ()
{
  int pin = 0;
  int sensor = 0;

  if (state == 'V')
  {
    pin = VOLTAGE;
  }
  else
    pin = TEMPERATURE;

  sensor = analogRead(pin);

  int drawHeight = map(sensor, 0, 1023, 0, screen.height() - 20);

  //Change stroke colours
  switch (pin)
  {
    case VOLTAGE:
      screen.stroke (255, 0, 0);
      break;
    case TEMPERATURE:
      screen.stroke (0, 255, 120);
      break;
    default:
      screen.stroke (255, 255, 255);
      break;
  }

  //Refresh screen if the graph has reached the edge
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

//Reads the sensor data and formats this along with the time in a comma delimited format.
String formatSensorData(int pin)
{
  float sensor = 0;
  sensor = analogRead(pin);
  if(pin == VOLTAGE)
  {
    sensor = (float)map (sensor, 0, 1023, 0, 25.2);
  }
  else
  {
    sensor = (float)(map (sensor, 0,1023,0,5.0)); //maps bits to voltage range on the LM35
    sensor = (float)sensor/0.01; //Convert to temperature by dividing by 0.01 (10mV)
  }
  
  dataString = (String(millis() / 1000));
  dataString += (",");
  dataString += String(sensor);
  return dataString;
}

//Open, write and update files on the SD card
void writeToSD()
{
  File dataFile;

  dataFile = SD.open(charFileNameV, FILE_WRITE);
  writeToFile (formatSensorData(VOLTAGE), dataFile);
  updateCount (VOLTAGE);

  dataFile = SD.open(charFileNameT, FILE_WRITE);
  writeToFile (formatSensorData(TEMPERATURE), dataFile);
  updateCount (TEMPERATURE);
}

//Function for writeToSD to write the data to a file
void writeToFile (String data, File currentFile)
{
  if (currentFile)
  {
    currentFile.println(data);
    currentFile.close();
  }
  else
    Serial.println ("Error opening the file");
}

//Function for writeToSD to increment the line count and change the name of the file if required
void updateCount(int fileType)//I don't like this section
{
  if (fileType == VOLTAGE)
  {
    lineCountV ++;
    if (lineCountV >= 50) //if the line count exceeds 50 then fileCount will be incremented and thus a new file should be created.
    {
      lineCountV = 0;
      do
      {
        fileCountV++;
        fileName = ("vltlog");
        fileName += String(fileCountV);
        fileName += ".log";
        fileName.toCharArray (charFileNameV, 12);
      } while (SD.exists(charFileNameV));
      Serial.println("Saving to new file: " + fileName);
    }
  }
  else
  {
    lineCountT ++;
    if (lineCountT >= 50) //if the line count exceeds 50 then fileCount will be incremented and thus a new file should be created.
    {
      lineCountT = 0;      
      do
      {
        fileCountT++;
        fileName = ("tmplog");
        fileName += String(fileCountT);
        fileName += ".log";
        fileName.toCharArray (charFileNameT, 12);
      }while(SD.exists(charFileNameT));
      
      Serial.println("Saving to new file: " + fileName);
    }
  }
}

//==========================================================INTERRUPTS=============================================================================

void serialEvent()
{
  while (Serial.available())
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    state = inChar;
    screen.background(0, 0, 0);
    xPos = 0;
  }
}

void buttonVoltage()
{
  state = 'V';
  screen.background(0, 0, 0);
  xPos = 0;

  screen.stroke (255, 255, 255);
  screen.text ("Voltage", (screen.width() / 2) - 3, 2);
}

void buttonTemperature()
{
  state = 'T';
  screen.background(0, 0, 0);
  xPos = 0;

  screen.stroke(255, 255, 255);
  screen.text ("Temperature", (screen.width() / 2) - 5, 2);
}





