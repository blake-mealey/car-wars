#pragma once

#include <string>

class Time{
public:
    // Constructors
	Time();
	Time(const double _timeSeconds);

    // Builders
    static Time FromMilliseconds(const double timeMilliseconds);
    static Time FromSeconds(const double timeSeconds);
    static Time FromMinutes(const double timeMinutes);
    static Time FromHours(const double timeHours);

    // Unit getters
    float GetMilliseconds() const;
	float GetSeconds() const;
    float GetMinutes() const;
    float GetHours() const;

    std::string ToString() const;

    // Operator overloads
	bool operator<(const Time other) const;
	bool operator<=(const Time other) const;
    bool operator>(const Time other) const;
    bool operator>=(const Time other) const;
	Time operator-(const Time other) const;
	Time operator+(const Time other) const;
    Time operator*(const Time other) const;
	void operator+=(const Time other);
private:
	double timeSeconds;
};
