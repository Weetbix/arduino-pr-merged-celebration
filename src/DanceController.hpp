#include <Servo.h>

class DanceController 
{
  public:
    DanceController(int servoPin, int minAngle, int maxAngle)
        : minAngle(minAngle), maxAngle(maxAngle)
    {
        servo.attach(servoPin);
    }

    void dance()
    {
        const int DELAY = 20;
        for (int pos = minAngle; pos <= maxAngle; pos += 1) { // goes from 0 degrees to 180 degrees
            // in steps of 1 degree
            servo.write(pos);              // tell servo to go to position in variable 'pos'
            delay(DELAY);                       // waits 15ms for the servo to reach the position
        }
        for (int pos = maxAngle; pos >= minAngle; pos -= 1) { // goes from 180 degrees to 0 degrees
            servo.write(pos);              // tell servo to go to position in variable 'pos'
            delay(DELAY);                       // waits 15ms for the servo to reach the position
        }
    }

  private:
    Servo servo;
    int minAngle;
    int maxAngle;
    int restingAngle; 
};