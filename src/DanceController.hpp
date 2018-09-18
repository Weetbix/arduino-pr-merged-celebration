#include <Servo.h>
#include "easing.hpp"

// Given an min and max angle, find the resting angle
// we should use (ie the middle value..)
int restingPosition(const int min, const int max)
{
    return ((max - min)  / 2) + min;
}

// Moves the servo from one value to another value, given a certain speed
// and using a given easing function
void moveFrom(Servo& servo, int from, int to, const int speed, EasingFunction easingFunction)
{
    const int steps = from > to ? from - to : to - from;
    const int distance = from < to ? steps : -steps;

    for(int i = 0; i < steps; i++ )
    {
        servo.write(easingFunction(i, from, distance, steps));
        delay(speed);
    }
}

typedef void (*DancingFunction)(Servo&, const int, const int, const int, EasingFunction);

// Move arms from mid to max, then to min, then back to mid.
void sweepingDance(Servo& servo, const int min, const int max, const int speed, EasingFunction easing = linear)
{
    const int mid = restingPosition(min, max);

    moveFrom(servo, mid, max, speed, easing);
    moveFrom(servo, max, min, speed, easing);
    moveFrom(servo, min, mid, speed, easing);
}

// Wave one side of the robot
void waveDance(Servo& servo, const int min, const int max, const int speed, EasingFunction easing = linear)
{
    const int mid = restingPosition(min, max);

    moveFrom(servo, mid, max, speed, easing);
    moveFrom(servo, max, mid, speed, easing);
}

// Do a kind of robot style dance, moving from mid to max, to mid, to min, to mid, but pausing inbetween
void robotDance(Servo& servo, const int min, const int max, const int speed, EasingFunction easing = linear)
{
    const int mid = restingPosition(min, max);

    moveFrom(servo, mid, max, speed, easing);
    delay(250);
    moveFrom(servo, max, mid, speed, easing);
    delay(250);
    moveFrom(servo, mid, min, speed, easing);
    delay(250);
    moveFrom(servo, min, mid, speed, easing);
}

// Wiggle left arms up a couple fo times, then wiggle right arms up a couple of times
void wiggleDance(Servo& servo, const int min, const int max, int speed, EasingFunction easing = linear)
{
    // Chill this dance out a bit
    if(speed < 5 )
        speed = 5;

    const int mid = restingPosition(min, max);

    moveFrom(servo, mid, max, speed, easing);
    moveFrom(servo, max, mid, speed, easing);
    moveFrom(servo, mid, max, speed, easing);
    moveFrom(servo, max, mid, speed, easing);
    delay(100);
    moveFrom(servo, mid, min, speed, easing);
    moveFrom(servo, min, mid, speed, easing);
    moveFrom(servo, mid, min, speed, easing);
    moveFrom(servo, min, mid, speed, easing);
    delay(100);
}

// An array of all our dance functions, so that we cna
// easily choose a random one
const DancingFunction DANCES[] = {
    sweepingDance,
    waveDance,
    robotDance,
    wiggleDance
};
const int NUM_DANCES = sizeof(DANCES) / sizeof(DANCES[0]);

// Class to help you controll a servo which is attached to
// some kind of paper hinged robot.
// Allows configurations of things such as min and max angle
// as this may vary for your setup.
//
// minDanceItems/maxDanceItems - These configure how many
// individual dance peices will be chained together when
// generating a full random dance
//
// minSpeed/maxSpeed - These configure the minimum and maximum
// delay values when moving the servo from one position to the
// next (1 degree).
class DanceController
{
  public:
    DanceController(
        int servoPin,
        int minAngle,
        int maxAngle,
        int minDanceItems = 2,
        int maxDanceItems = 5,
        int minSpeed = 5,
        int maxSpeed = 35
    )
        : minAngle(minAngle), maxAngle(maxAngle),
          minDanceItems(minDanceItems), maxDanceItems(maxDanceItems),
          minSpeed(minSpeed), maxSpeed(maxSpeed)
    {
        servo.attach(servoPin);
    }

    // Generate a random dance by chaining together several other
    // dances with random parameters.
    void randomDance()
    {
        for(int i = 0; i < random(minDanceItems, maxDanceItems + 1); i++)
        {
            singleRandomDance();

            // Sometimes add a delay between dances
            // for a bit of variety
            if( random(0,4) == 3 )
            {
                delay(random(200, 500));
            }
        }

        // When we are finished ensure the servo is back
        // at its resting postion
        servo.write(restingPosition(minAngle, maxAngle));
    }

    void singleRandomDance()
    {
        const int speed = random(minSpeed, maxSpeed);

        const EasingFunction easing =
            EASING_FUNCTIONS[random(0, NUM_EASING_FUNCTIONS)];

        DANCES[random(0, NUM_DANCES)](
            servo, minAngle, maxAngle, speed, easing
        );
    }

  private:
    Servo servo;

    int minAngle;
    int maxAngle;
    int minDanceItems;
    int maxDanceItems;
    int minSpeed;
    int maxSpeed;
};