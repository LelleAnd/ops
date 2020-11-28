/**
*
* Copyright (C) 2018-2019 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/

#include <sstream>

#include "gtest/gtest.h"

#include "Error.h"
#include "ErrorService.h"
#include "BasicError.h"
#include "BasicWarning.h"
#include "ErrorWriter.h"

using namespace ops;

// ===============================
// Helper classes

class MyErrorListener : public Listener<Error*>
{
public:
	int counter;
	ErrorMessage_T error;
	Error::Severity_T severity;
	int errorCode;
	MyErrorListener() : counter(0), severity(Error::warning), errorCode(0) {}
	virtual void onNewEvent(Notifier<Error*>* , Error* const arg)  override
	{ 
		counter++;
		error = arg->getMessage();
		severity = arg->getSeverity();
		errorCode = arg->getErrorCode();
	}

	~MyErrorListener() = default;
	MyErrorListener(const MyErrorListener& other) = delete;
	MyErrorListener& operator= (const MyErrorListener& other) = delete;
	MyErrorListener(MyErrorListener&& other) = delete;
	MyErrorListener& operator =(MyErrorListener&& other) = delete;
};

// ===============================

TEST(Test_ErrorServiceReport, Test) {

	MyErrorListener listener1;
	ErrorService notifier;

	EXPECT_EQ(notifier.getNrOfListeners(), 0);
	EXPECT_EQ(listener1.counter, 0);

	notifier.report("class", "method", "message");

	EXPECT_EQ(listener1.counter, 0);

	notifier.addListener(&listener1);

	EXPECT_EQ(notifier.getNrOfListeners(), 1);
	
	notifier.report("class", "method", "message");

	EXPECT_EQ(listener1.counter, 1);
	EXPECT_EQ(listener1.severity, Error::error);
	EXPECT_EQ(listener1.errorCode, 1);
	EXPECT_STREQ(listener1.error.c_str(), "class::method(): message");

	notifier.report("class", "method", "message2", Error::warning);

	EXPECT_EQ(listener1.counter, 2);
	EXPECT_EQ(listener1.severity, Error::warning);
	EXPECT_EQ(listener1.errorCode, 1);
	EXPECT_STREQ(listener1.error.c_str(), "class::method(): message2");

	BasicError err1("err1", "test", "message3");

	EXPECT_EQ(err1.getSeverity(), Error::error);
	EXPECT_EQ(err1.getErrorCode(), 1);
	EXPECT_STREQ(err1.getMessage().c_str(), "err1::test(): message3");

	notifier.report(&err1);

	EXPECT_EQ(listener1.counter, 3);
	EXPECT_EQ(listener1.severity, Error::error);
	EXPECT_EQ(listener1.errorCode, 1);
	EXPECT_STREQ(listener1.error.c_str(), "err1::test(): message3");

	BasicWarning warn1("warn1", "test", "message4");

	EXPECT_EQ(warn1.getSeverity(), Error::warning);
	EXPECT_EQ(warn1.getErrorCode(), 1);
	EXPECT_STREQ(warn1.getMessage().c_str(), "warn1::test(): message4");

	notifier.report(&warn1);

	EXPECT_EQ(listener1.counter, 4);
	EXPECT_EQ(listener1.severity, Error::warning);
	EXPECT_EQ(listener1.errorCode, 1);
	EXPECT_STREQ(listener1.error.c_str(), "warn1::test(): message4");
}

// ===============================
// Helper classes


TEST(Test_ErrorServiceReport, TestErrorWriter) {

	std::ostringstream ss;

	ErrorWriter ew(ss);
	ErrorService notifier;

	notifier.addListener(&ew);

	EXPECT_EQ(notifier.getNrOfListeners(), 1);

	notifier.report("class", "method", "message");

	// ... - Error, code: 1. Message: class::method(): message.

	// Skip leading time field before compare
	std::string str = ss.str();
	auto pos = str.find_last_of("-");
	EXPECT_NE(pos, std::string::npos);
	str = str.substr(pos);
	EXPECT_STREQ(str.c_str(), "- Error, code: 1. Message: class::method(): message.\n");

	notifier.report("class", "method", "message2", Error::warning);

	// ... - Warning, code: 1. Message: class::method(): message2.

	// Skip leading time field and previous data before compare
	str = ss.str();
	pos = str.find_last_of("-");
	EXPECT_NE(pos, std::string::npos);
	str = str.substr(pos);
	EXPECT_STREQ(str.c_str(), "- Warning, code: 1. Message: class::method(): message2.\n");
}
