#pragma once

class Time{
public:
	Time();
	Time(double _timeValueSeconds);

	float GetTimeSeconds();
	float GetTimeMilliSeconds();

	double operator-(Time _timeValueSeconds);
	double operator+(Time _timeValueSeconds);
	void operator+=(Time _timeValueSeconds);
private:
	double timeValueSeconds;
};