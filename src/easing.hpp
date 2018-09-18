// A few different easing functions, all using
// https://github.com/jesusgollonet/ofpennereasing/blob/master/PennerEasing
// as a reference.

typedef float (*EasingFunction)(float,float,float,float);

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

const EasingFunction EASING_FUNCTIONS[] = {
    easeInSine,
    easeOutSine,
    easeInQuadratic,
    easeOutQuadratic,
    linear
};
const int NUM_EASING_FUNCTIONS = sizeof(EASING_FUNCTIONS) / sizeof(EASING_FUNCTIONS[0]);