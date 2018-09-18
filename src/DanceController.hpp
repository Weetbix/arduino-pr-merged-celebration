#include <Servo.h>

typedef float (*EasingFunction)(float,float,float,float);

// All from https://github.com/jesusgollonet/ofpennereasing/blob/master/PennerEasing
float easeInSine(float time, float start, float distance, float duration) {
	return -distance * cos(time/duration * (PI/2)) + distance + start;
}

float easeOutSine(float time, float start, float distance, float duration) {
	return distance * cos(time/duration * (PI/2)) + start;
}

float easeInQuadratic(float time, float start, float distance, float duration) {
	return distance * (time /= duration)*time + start;
}

float easeOutQuadratic(float time, float start, float distance, float duration) {
    return -distance * (time/=duration)*(time-2) + start;
}

float linear(float time, float start, float distance, float duration){
    return distance * time / duration + start;
}

int restingPosition(const int min, const int max)
{
    return ((max - min)  / 2) + min;
}

void moveFrom(Servo& servo, int from, int to, const int speed, EasingFunction easingFunction)
{
    const int step = 1;
    const int steps = from > to ? from - to : to - from;
    const int distance = from < to ? steps : -steps;

    for(int i = 0; i < steps; i++ )
    {
        servo.write(easingFunction(i, from, distance, steps));
        delay(speed);
    }
}

void sweepingDance(Servo& servo, const int min, const int max, const int speed, EasingFunction easing = linear)
{
    int pos = restingPosition(min, max);

    const int mid = restingPosition(min, max);

    moveFrom(servo, mid, max, speed, easing);
    moveFrom(servo, max, min, speed, easing);
    moveFrom(servo, min, mid, speed, easing);
}

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
        sweepingDance(servo, minAngle, maxAngle, 20, easeInQuadratic);
        sweepingDance(servo, minAngle, maxAngle, 20, easeOutQuadratic);

        sweepingDance(servo, minAngle, maxAngle, 20, easeInSine);
        sweepingDance(servo, minAngle, maxAngle, 20 );
    }

  private:
    Servo servo;
    int minAngle;
    int maxAngle;
};