/************************************************************************

  Code for self edjecting parachute for the course "Inledande ingengörskurs i teknisk fysik"

  The code read the data from the Pmod ACL2 accelerometer component. A level shifter is nessesary to convert the logic level of the accelerometer (3V) to the logic level of 
  the Aurdino Uno (5V). When the accelerometer sences that it is in free fall the servo is activated releasing the parachute.

  Once Wiring is comple, calibration is done with the help by changing the calibration values in the begining of the code. 

*************************************************************************
  Components
  1. Arduino Uno
  2. Pmod ACL2 (dowload library https://github.com/annem/ADXL362 )
  3. Adafruit TXB0108

  Wiring
  Module <---- LEVEL SHIFTER! ------> Arduino
  VCC                                  5V (3V3 without LVL SHFT)
  GND                                  GND (No LVL SHFT needed)
  SCK                                 13 (SCK)
  MOSI                                12 (MISO)
  MISO                                11 (MOSI)
  CS                                  10

  Servo <----------> Arduino
  VCC                   5V
  GND                   GND
  Signal                Pin 9


************************************************************************/

#define CS 10 //chip select pin for accelerometer
#define CS2 9 //chip select pin for SD card reader 

// Call of libraries
#include <SPI.h>
#include <ADXL362.h>_
#include <Servo.h> 
#include <SD.h>

ADXL362 accelerometer; // Accelerometer object
Servo myServo; // Servo object
File myFile; // File object

// Indexing variable, do not change
int throwNr = 0;
int index = 0;



// Variables, feel free to change
int servoPin = 8;

int releaseseAngle = 55;
int lockAngle = 100;

// Calibration 
double x_calibration = 8;
double y_calibration = -20;
double z_calibration = -176;

int freeFallThreshold = 300;

const int BufferSize = 100;

// Crearing array for Circular buffer
int CircularBuffer[BufferSize];


void setup(void)
{

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  myServo.attach(servoPin); 
  myServo.write(lockAngle);

  
  Serial.print("Initializing SD card...");

  if (!SD.begin(CS2)) {
    Serial.println("initialization failed!");
    //while (1);
  }
  Serial.println("initialization done.");
  


  accelerometer.begin(CS); // initialization of the accelerometer
  accelerometer.beginMeasure();  //initialization of the measurement
  
  
}

void loop()
{ 
  // Define data types for variables
  int x, y, z, t;
  

  accelerometer.readXYZTData(x, y, z, t); // Use libary to read data from accelerometer
  
  
  // Adjust values for calibration 
  x = x + x_calibration;
  y = y + y_calibration;
  z = z + z_calibration;

  // Calcutate total acceleration
  double totalax =sqrt((double)x*(double)x + (double)y*(double)y + (double)z*(double)z);
  
  //Print values for each axis and total calibration
  String output = "x = " + String(x) + "\t y = " + String(y) + "\t z = " + String(z) + "\t total = " + String(totalax);
  Serial.println(output);

  
  // Add totalax values to circular buffer
  CircularBuffer[index++] = totalax;
  
  if (index > BufferSize){
    index = 0;
  }

  // If statement that checks for free fall
  if (totalax < freeFallThreshold) {
    myServo.write(releaseseAngle); // Turns servo, releasing the parachute  
    Serial.println("Drop!");
    
    

    myFile = SD.open("data.txt", FILE_WRITE); //Open (and create in non existant) file called data

    // if the file opened okay, write to it:
    if (myFile) {
        
        myFile.print("Throw:");
        myFile.println(throwNr);
        throwNr++;
        myFile.print("[");
      
      for (int i = index; i < BufferSize; i++) {
        myFile.print(CircularBuffer[i]);
        myFile.print(",");
      }
      for (int i = 0; i < index; i++) {
        myFile.print(CircularBuffer[i]);
        myFile.print(",");
      }
        myFile.print("]");

      myFile.close(); // close file
      delay(1000);
    } 

    
    
    else {
      Serial.println("error opening data.txt"); //file not opening error
    }
  } 
  else {
    // Optionally, you can set the servo to another position when totalAx is not less than 500
    myServo.write(lockAngle); // Move the servo to 
  }
  
}
