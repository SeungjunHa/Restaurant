#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../Project5/Customer.cpp"
#include "../Project5/Schedule.cpp"
#include "../Project5/BookingScheduler.cpp"

using namespace testing;

class BookingItem : public Test {
public:
	tm getTime(int year, int mon, int day, int hour, int min) {
		tm result = { 0, min, hour, day, mon - 1, year - 1900, 0, 0, -1 };
		mktime(&result);
		return result;
	}
};

TEST_F(BookingItem, OnlyOnTheHourFail)
{
	tm notOnTheHour = { 0 };
	notOnTheHour.tm_year = 2021 - 1900;
	notOnTheHour.tm_mon = 03 - 1;
	notOnTheHour.tm_mday = 26;
	notOnTheHour.tm_hour = 9;
	notOnTheHour.tm_min = 5;
	notOnTheHour.tm_isdst = -1;
	mktime(&notOnTheHour);

	Customer customer{ "Fake name", "010-1234-5678" };
	Schedule* schedule = new Schedule{ notOnTheHour, 1, customer };
	BookingScheduler bookingScheduler{ 3 };

	EXPECT_THROW({
		bookingScheduler.addSchedule(schedule);
		}, std::runtime_error);
}


TEST_F(BookingItem, OnlyOnTheHourPass)
{
	tm onTheHour = { 0 };
	onTheHour.tm_year = 2021 - 1900;
	onTheHour.tm_mon = 03 - 1;
	onTheHour.tm_mday = 26;
	onTheHour.tm_hour = 9;
	onTheHour.tm_min = 0;
	onTheHour.tm_isdst = -1;
	mktime(&onTheHour);

	Customer customer{ "Fake name", "010-1234-5678" };
	Schedule* schedule = new Schedule{ onTheHour, 1, customer };
	BookingScheduler bookingScheduler{ 3 };

	bookingScheduler.addSchedule(schedule);

	EXPECT_EQ(true, bookingScheduler.hasSchedule(schedule));
}