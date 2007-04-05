/************************************************************************
** Copyright 2007 Einar Otto Stangvik
** 
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
** 
**    http://www.apache.org/licenses/LICENSE-2.0
** 
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <boost/test/test_tools.hpp>
#include <boost/test/results_reporter.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/output_test_stream.hpp>
#include <boost/test/unit_test_log.hpp>
#include <boost/test/framework.hpp>
#include <boost/test/detail/unit_test_parameters.hpp>
#include <boost/test/utils/nullstream.hpp>
#include <boost/scoped_ptr.hpp>

#include "../ThreadSynch/ThreadSynch.h"
#include "../ThreadSynch/APCPickupPolicy.h"

#ifdef _DEBUG
#pragma comment(lib, "libboost_test_exec_monitor-vc80-mt-gd.lib")
#pragma comment(lib, "libboost_unit_test_framework-vc80-mt-gd.lib")
#else
#pragma comment(lib, "libboost_test_exec_monitor-vc80-mt.lib")
#pragma comment(lib, "libboost_unit_test_framework-vc80-mt.lib")
#endif

using namespace boost::unit_test;

typedef boost::onullstream onullstream_type;

/************************************************************************
** Test helper structs and functions
*/

HANDLE g_hTestThread;
HANDLE g_hCloseEvent;
DWORD g_dwThreadId;

DWORD WINAPI testThread(PVOID)
{
	while(WaitForSingleObjectEx(g_hCloseEvent, INFINITE, TRUE) != WAIT_OBJECT_0)
	{
		Sleep(1);
	}
	return 0;
}

/************************************************************************
** Test 1: Parameters
*/

int crossThreadIntValue(int input)
{
	return input * 2;
}

int crossThreadIntPtr(int* input)
{
	return *input * 2;
}

int crossThreadIntRef(int& input)
{
	return input * 2;
}

void testParameters()
{
	ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>* scheduler = ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>::getInstance();

	int input1 = 0x42;
	boost::function<int()> callback1 = boost::bind(crossThreadIntValue, input1);
	BOOST_CHECK(callback1() == scheduler->syncCall(g_dwThreadId, callback1, INFINITE));

	boost::scoped_ptr<int> input2(new int);
	*input2 = 0x42;
	boost::function<int()> callback2 = boost::bind(crossThreadIntPtr, input2.get());
	BOOST_CHECK(callback2() == scheduler->syncCall(g_dwThreadId, callback2, INFINITE));

	int input3 = 0x42;
	boost::function<int()> callback3 = boost::bind(crossThreadIntRef, boost::ref(input3));
	BOOST_CHECK(callback3() == scheduler->syncCall(g_dwThreadId, callback3, INFINITE));
}

/************************************************************************
** Test 2: Pointer to class return value
*/

class SharedClass
{
public:
	static int refcount;
	SharedClass() { ++refcount; }
	~SharedClass() { --refcount; }
};
int SharedClass::refcount = 0;

boost::shared_ptr<SharedClass> crossThreadPtr()
{
	boost::shared_ptr<SharedClass> ptr(new SharedClass);
	BOOST_CHECK(ptr->refcount == 1);
	return ptr;
}

void testReturnValues()
{
	ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>* scheduler = ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>::getInstance();

	boost::function<boost::shared_ptr<SharedClass>()> callback1 = crossThreadPtr;
	boost::shared_ptr<SharedClass> ptr = scheduler->syncCall(g_dwThreadId, callback1, INFINITE);
	BOOST_CHECK(ptr->refcount == 1);
}

/************************************************************************
** Test 3: Exceptions
*/

class TestException
{
public:
	int magicNumber;
	TestException(int mn = 0) : magicNumber(mn) {}
};

bool isRealException(const TestException& ex)
{
	return ex.magicNumber == 42;
}

void crossThreadException()
{
	throw TestException(42);
}

void testExceptions()
{
	ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>* scheduler = ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>::getInstance();

	BOOST_CHECK_EXCEPTION(scheduler->syncCall<ExceptionTypes<TestException>>(g_dwThreadId, crossThreadException, INFINITE), TestException, isRealException);
}

/************************************************************************
** Test Setup
*/

int test_main( int argc, char* argv[] ) 
{
	g_hCloseEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_hTestThread = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(testThread), NULL, 0, &g_dwThreadId);
	BOOST_REQUIRE(g_hTestThread != NULL);

	test_suite* ts = BOOST_TEST_SUITE("ThreadSynch Unit Tests");

	ts->add(BOOST_TEST_CASE(&testParameters));
	ts->add(BOOST_TEST_CASE(&testReturnValues));
	ts->add(BOOST_TEST_CASE(&testExceptions ));

	framework::run(ts);

	SetEvent(g_hCloseEvent);
	WaitForSingleObject(g_hTestThread, INFINITE);
	CloseHandle(g_hTestThread);
	CloseHandle(g_hCloseEvent);

	// Delete the singleton, for the sake of leak detection.
	// Some globals will however be detected either way, so don't be alarmed by the notification for the time beaing.
	// The key point is: The leak doesn't grow.
	delete ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>::getInstance();

	return 0;
}

