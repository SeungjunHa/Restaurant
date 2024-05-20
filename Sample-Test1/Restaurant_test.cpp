#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../Project5/Customer.cpp"
#include "../Project5/Schedule.cpp"
#include "../Project5/BookingScheduler.cpp"

using namespace testing;

class BookingItem : public Test {
protected:
	void SetUp() override {
		notOnTheHour = getTime(2021, 3, 26, 9, 5);
		onTheHour = getTime(2021, 3, 26, 9, 0);
	}
public:
	tm getTime(int year, int mon, int day, int hour, int min) {
		tm result = { 0, min, hour, day, mon - 1, year - 1900, 0, 0, -1 };
		mktime(&result);
		return result;
	}

	tm notOnTheHour;
	tm onTheHour;
	Customer customer{ "Fake Name", "010-1234-5678" };
};

TEST_F(BookingItem, OnlyOnTheHourFail)
{
	Schedule* schedule = new Schedule{ notOnTheHour, 1, customer };
	BookingScheduler bookingScheduler{ 3 };

	EXPECT_THROW({
		bookingScheduler.addSchedule(schedule);
		}, std::runtime_error);
}

TEST_F(BookingItem, OnlyOnTheHourPass)
{
	Schedule* schedule = new Schedule{ onTheHour, 1, customer };
	BookingScheduler bookingScheduler{ 3 };

	bookingScheduler.addSchedule(schedule);

	EXPECT_EQ(true, bookingScheduler.hasSchedule(schedule));
}