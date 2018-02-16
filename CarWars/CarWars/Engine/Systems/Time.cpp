#include "Time.h"

#include <iostream>
using namespace std;

Time::Time() {
	timeValueSeconds = 0;
}

Time::Time(double _timeValueSeconds) {
	timeValueSeconds = _timeValueSeconds;
}

float Time::GetTimeSeconds() {
	return (float)timeValueSeconds;
}

/*
Converts stored time value (seconds) to milliseconds
*/
float Time::GetTimeMilliSeconds() {
	return (float)timeValueSeconds * 1000.f;
}

/*
Overloaded operator to allow for subtraction between Time data types
*/
double Time::operator-(Time _timeValueSeconds) {
	return timeValueSeconds - _timeValueSeconds.GetTimeSeconds();
}

/*
Overloaded operator to allow for addition between Time data types
*/
double Time::operator+(Time _timeValueSeconds) {
	return timeValueSeconds + _timeValueSeconds.GetTimeSeconds();
}

void Time::operator+=(Time _timeValueSeconds) {
	timeValueSeconds += _timeValueSeconds.GetTimeSeconds();
}
