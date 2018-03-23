#include "Time.h"

Time::Time() : timeSeconds(0.0) {}

Time::Time(const double _timeSeconds) : timeSeconds(_timeSeconds) {}

Time Time::FromMilliseconds(const double timeMilliseconds) {
    return Time(timeMilliseconds / 1000.0);
}

Time Time::FromSeconds(const double timeSeconds) {
    return Time(timeSeconds);
}

Time Time::FromMinutes(const double timeMinutes) {
    return Time(timeMinutes * 60.0);
}

Time Time::FromHours(const double timeHours) {
    return FromMinutes(timeHours * 60.0);
}

float Time::GetMinutes() const {
    return GetSeconds() / 60.0;
}

float Time::GetHours() const {
    return GetMinutes() / 60.0;
}

std::string Time::ToString() const {
    char buf[6];
    const int minutes = GetMinutes();
    const int seconds = GetSeconds();
    snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds % 60);
    return std::string(buf);
}

float Time::GetSeconds() const {
	return timeSeconds;
}

float Time::GetMilliseconds() const {
	return GetSeconds() * 1000.0;
}

bool Time::operator<(const Time other) const {
    return GetSeconds() < other.GetSeconds();
}

bool Time::operator<=(const Time other) const {
    return GetSeconds() <= other.GetSeconds();
}

bool Time::operator>(const Time other) const {
    return GetSeconds() > other.GetSeconds();
}

bool Time::operator>=(const Time other) const {
    return GetSeconds() >= other.GetSeconds();
}

Time Time::operator-(const Time other) const {
	return GetSeconds() - other.GetSeconds();
}

Time Time::operator+(const Time other) const {
	return GetSeconds() + other.GetSeconds();
}

Time Time::operator*(const Time other) const {
    return GetSeconds() * other.GetSeconds();
}

void Time::operator+=(const Time other) {
    timeSeconds += other.GetSeconds();
}

void Time::operator-=(const Time other) {
    timeSeconds -= other.GetSeconds();
}
