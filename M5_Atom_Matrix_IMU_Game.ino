#include "M5Atom.h"

// IMU variables

float accX = 0, accY = 0, accZ = 0;
float gyroX = 0, gyroY = 0, gyroZ = 0;
float temp = 0;
float threshold = 50;

bool IMU6886Flag = false;

// display variables

int xpos = 2;
int ypos = 2;
int pos = -1;

// Gameplay variables

int gamepos = random (0,25);
unsigned long lastTime;
unsigned long turnTime;

unsigned long minTurnTime = 150;
unsigned long maxTurnTime = 1000;
unsigned long decTurnTime = 200;
uint8_t FSM = 0;

// set up the sketch

void setup()
{

    // Init the atom and peripherals 
    
    M5.begin(true, false, true);

    if (M5.IMU.Init() != 0)
    {
        IMU6886Flag = false;
    }
    else
    {
        IMU6886Flag = true;
    }

    // draw the initial position of the target
    
    M5.dis.drawpix(gamepos, 0x707070);

}

// fill the led display with the specified color

void fillpix (CRGB color)
{
   for (int i = 0; i < 25; i++)
   {
      M5.dis.drawpix(i, color);
   }
}

// Flash the screen between two colors (0 is blank)
// the specified number of times

void flashpix (int times, CRGB color1, CRGB color2)
{
   
   for (int i = 0; i < 3; i++)
   {
      fillpix (color1);
      delay(500);
      fillpix (color2);
      delay(500);
   }
   
   delay(1500);

}

// pick a new position for the target and display it.

void new_target()
{

   // Eraset the current target
     
   M5.dis.drawpix(gamepos, 0);
    
   // pick the next target position until it's
   // different than where the player is, otherwise,
   // they'd get an instant win and that's TOO EASY.

   do 
   {
      gamepos = random (0,25);
   } while (gamepos == pos);

   // draw the target
     
   M5.dis.drawpix(gamepos, 0x707070);
   
}

void set_next_turn_time()
{
        
   // calculate the amount of time you get to 
   // try to hit the target.
   
   lastTime = millis();
   turnTime = random(minTurnTime, maxTurnTime);
   
}

void loop()
{

    // If the IMU is present, read the gyro
    
    if (IMU6886Flag == true)
    {
        M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
        M5.IMU.getAccelData(&accX, &accY, &accZ);
        M5.IMU.getTempData(&temp);

        Serial.printf("%.2f,%.2f,%.2f o/s \r\n", gyroX, gyroY, gyroZ);
        //Serial.printf("%.2f,%.2f,%.2f mg\r\n", accX * 1000, accY * 1000, accZ * 1000);
        //Serial.printf("Temperature : %.2f C \r\n", temp);
    }

    // If the button (display) was pressed...
    
    if (M5.Btn.wasPressed())
    {

        // reset the player to the middle of the display

        xpos = 2;
        ypos = 2;

        // change the color of the player
            
        FSM++;
        if (FSM >= 4)
        {
            FSM = 0;
        }
        
    }
    else
    {

        // no button was pressed, use the gyro readings to 
        // move the player around the board (display) based
        // on how device movement.  Only do x and y, if we
        // get a 3d display, maybe we can incorporate z!

        // gyro detects movement, not absolute position, so
        // tilting left makes the player go left, etc.
        
        if (gyroY < (threshold * -1.0)) 
        {
            if (ypos > 0)
            {
                ypos--;
            }
        } 
        else if (gyroY > threshold) 
        {
            if (ypos < 4)
            {
                ypos++;
            }
        }
        
        if (gyroX < (threshold * -1.0)) 
        {
            if (xpos > 0)
            {
                xpos--;
            }
        } 
        else if (gyroX > (threshold * 1.0)) 
        {
            if (xpos < 4)
            {
                xpos++;
            }
        }
    }

    // If this isn't our first time through, we have
    // to erase the last position of the player by
    // blanking the led at that position.
    
    if (pos != -1)
    {
        M5.dis.drawpix(pos, 0);
    }

    // based on the calculated x, y position of the
    // player, figure out what position in the led 
    // display to light up.
    
    // The position is 0..24, as the pixels on the 
    // display are a linear array, despite being 
    // physically arranged in a 5x5 matrix.
    
    pos = (5 * (4 - xpos)) + ypos;

    // if the position of the player matches the
    // position of the target, YOU WIN!
    
    if (pos == gamepos)
    {

        // do a victory flash on the screen
        
        flashpix (3, 0x0000f0, 0);

        // pick a new position for the target and display it.
        
        new_target();

        // every time the user wins, decrease the max time
        // until it's close to or at the min time (depending
        // on the math)
        
        if (maxTurnTime > (minTurnTime + decTurnTime))
        {
           maxTurnTime -= decTurnTime; 
        }

        // set the amount of time they have to hit the target

        set_next_turn_time();

    }
    else if ((millis() - lastTime) > turnTime)
    {

        // d'oh - the player didn't catch the target in the
        // allotted time.  Too bad, so sad.

        // pick an new random spot for the target
        
        new_target();
        M5.dis.drawpix(gamepos, 0x707070);

        // set the amount of time they have to hit the target

        set_next_turn_time();
        
    }

    // set the color of the player and display it, depending 
    // on what color the player has selected.  It gets really
    // challenging when the player and the target are the same
    // color.
    
    switch (FSM)
    {
        case 0:
            M5.dis.drawpix(pos, 0xf00000);
            break;
        case 1:
            M5.dis.drawpix(pos, 0x00f000);
            break;
        case 2:
            M5.dis.drawpix(pos, 0x0000f0);
            break;
        case 3:
            M5.dis.drawpix(pos, 0x707070);
            break;
        default:
            break;
    }

    // slight delay and update the buttons and speaker (if you have one)
    
    delay(50);
    M5.update();
    
}
