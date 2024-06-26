#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../Project5/Customer.cpp"
#include "../Project5/Schedule.cpp"
#include "../Project5/BookingScheduler.cpp"

using namespace testing;
using namespace std;

class TestableBookingScheduler : public BookingScheduler {
public:
	TestableBookingScheduler(int capacityPerHour, tm dateTime) :
		BookingScheduler{ capacityPerHour },
		dateTime{ dateTime } { }

	time_t getNow() override {
		return mktime(&dateTime);
	}

private:
	tm dateTime;
};

class TestableMailSender : public MailSender {
public:
	void sendMail(Schedule* schedule) override {
		countSendMailMethodIsCalled++;
	}

	int getCountSendMailMethodIsCalled() {
		return countSendMailMethodIsCalled;
	}

private:
	int countSendMailMethodIsCalled = 0;
};

class TestableSmsSender : public SmsSender {
public:
	void send(Schedule* schedule) override {
		cout << "테스트용 SmsSender class의 send에서도 실행됨" << endl;
		sendMethodIsCalled = true;
	}

	bool isSendMethodIsCalled() {
		return sendMethodIsCalled;
	}

private:
	bool sendMethodIsCalled;
};

class BookingItem : public Test {
protected:
	void SetUp() override {
		NOT_ON_THE_HOUR = getTime(2021, 3, 26, 9, 5);
		ON_THE_HOUR = getTime(2021, 3, 26, 9, 0);
		bookingScheduler.setSmsSender(&testableSmsSender);
	}
public:
	tm getTime(int year, int mon, int day, int hour, int min) {
		tm result = { 0, min, hour, day, mon - 1, year - 1900, 0, 0, -1 };
		mktime(&result);
		return result;
	}

	tm plusHour(tm base, int hour)
	{
		base.tm_hour += hour;
		mktime(&base);
		return base;
	}

	tm NOT_ON_THE_HOUR;
	tm ON_THE_HOUR;
	tm MONDAY = getTime(2021, 6, 3, 17, 0);
	tm SUNDAY = getTime(2021, 3, 28, 17, 0);
	Customer CUSTOMER{ "Fake Name", "010-1234-5678" };
	Customer CUSTOMER_WITH_MAIL{ "Fake Name", "010-1234-5678", "test@test.com" };
	const int UNDER_CAPACITY = 1;
	const int CAPACITY_PER_HOUR = 3;
	BookingScheduler bookingScheduler{ CAPACITY_PER_HOUR };
	TestableSmsSender testableSmsSender;
};

TEST_F(BookingItem, OnlyOnTheHourFail)
{
	Schedule* schedule = new Schedule{ NOT_ON_THE_HOUR, UNDER_CAPACITY, CUSTOMER };

	EXPECT_THROW({
		bookingScheduler.addSchedule(schedule);
		}, std::runtime_error);
}

TEST_F(BookingItem, OnlyOnTheHourPass)
{
	Schedule* schedule = new Schedule{ ON_THE_HOUR, UNDER_CAPACITY, CUSTOMER };

	bookingScheduler.addSchedule(schedule);

	EXPECT_EQ(true, bookingScheduler.hasSchedule(schedule));
}

TEST_F(BookingItem, OverCapacity)
{
	Schedule* schedule = new Schedule{ ON_THE_HOUR, CAPACITY_PER_HOUR, CUSTOMER };

	bookingScheduler.addSchedule(schedule);

	try {
		Schedule* newSchedule = new Schedule{ ON_THE_HOUR, UNDER_CAPACITY, CUSTOMER };
		bookingScheduler.addSchedule(newSchedule);
		FAIL();
	}
	catch (std::runtime_error& e) {
		EXPECT_EQ(string{ e.what() }, string{ "Number of people is over restaurant capacity per hour" });
	}
}

TEST_F(BookingItem, OverCapacityButDifferentHour)
{
	Schedule* schedule = new Schedule{ ON_THE_HOUR, CAPACITY_PER_HOUR, CUSTOMER };
	bookingScheduler.addSchedule(schedule);

	tm differentHour = plusHour(ON_THE_HOUR, 1);
	Schedule* newSchedule = new Schedule{ differentHour, CAPACITY_PER_HOUR, CUSTOMER };
	bookingScheduler.addSchedule(newSchedule);

	EXPECT_EQ(true, bookingScheduler.hasSchedule(schedule));
	EXPECT_EQ(true, bookingScheduler.hasSchedule(newSchedule));
}

TEST_F(BookingItem, SendSmsWhenSuccessToReserve)
{
	Schedule* schedule = new Schedule{ ON_THE_HOUR, CAPACITY_PER_HOUR, CUSTOMER };

	bookingScheduler.addSchedule(schedule);

	EXPECT_EQ(true, testableSmsSender.isSendMethodIsCalled());
}

TEST_F(BookingItem, NotSendEmailWhenNoAddress)
{
	TestableMailSender testableMailSender;
	Schedule* schedule = new Schedule{ ON_THE_HOUR, CAPACITY_PER_HOUR, CUSTOMER };
	bookingScheduler.setMailSender(&testableMailSender);
	bookingScheduler.addSchedule(schedule);

	EXPECT_EQ(0, testableMailSender.getCountSendMailMethodIsCalled());
}

TEST_F(BookingItem, SendEmailWhenAddress)
{
	TestableMailSender testableMailSender;
	Schedule* schedule = new Schedule{ ON_THE_HOUR, CAPACITY_PER_HOUR, CUSTOMER_WITH_MAIL };
	bookingScheduler.setMailSender(&testableMailSender);
	bookingScheduler.addSchedule(schedule);

	EXPECT_EQ(1, testableMailSender.getCountSendMailMethodIsCalled());
}

TEST_F(BookingItem, CannotReserveOnSunday)
{
	BookingScheduler* bookingScheduler = new TestableBookingScheduler(CAPACITY_PER_HOUR, SUNDAY);

	try {
		Schedule* schedule = new Schedule(ON_THE_HOUR, UNDER_CAPACITY, CUSTOMER_WITH_MAIL);
		bookingScheduler->addSchedule(schedule);
		FAIL();
	}
	catch (std::runtime_error& e) {
		EXPECT_EQ(string{ e.what() }, string{ "Booking system is not available on sunday" });
	}
}

TEST_F(BookingItem, CanReserveOnMonday)
{
	BookingScheduler* bookingScheduler = new TestableBookingScheduler(CAPACITY_PER_HOUR, MONDAY);

	Schedule* schedule = new Schedule(ON_THE_HOUR, UNDER_CAPACITY, CUSTOMER_WITH_MAIL);
	bookingScheduler->addSchedule(schedule);

	EXPECT_EQ(true, bookingScheduler->hasSchedule(schedule));
}