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

// Boost Test headers
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

// Boost headers
#include <boost/scoped_ptr.hpp>

// ThreadSynch Headers
#include "../ThreadSynch/ThreadSynch.h"
#include "../ThreadSynch/APCPickupPolicy.h"

#ifdef _DEBUG
#pragma comment(lib, "libboost_unit_test_framework-vc80-mt-gd.lib")
#pragma comment(lib, "libboost_test_exec_monitor-vc80-mt-gd.lib")
#else
#pragma comment(lib, "libboost_unit_test_framework-vc80-mt.lib")
#pragma comment(lib, "libboost_test_exec_monitor-vc80-mt.lib")
#endif

using namespace boost::unit_test;

/************************************************************************
** Test helper classes and functions
*/

class SharedClass
{
public:
    static int refcount;
    SharedClass() { ++refcount; }
    ~SharedClass() { --refcount; }
};
int SharedClass::refcount = 0;

boost::shared_ptr<SharedClass> crossThreadPtr();
struct TestException 
{
    int magicNumber;
    TestException(int mn = 0) : magicNumber(mn) {}
};

HANDLE g_hTestThread;
HANDLE g_hCloseEvent;
HANDLE g_hTemporarilySuspendEvent;
DWORD g_dwThreadId;

DWORD WINAPI testThread(PVOID);
int crossThreadIntValue(int input);
int crossThreadIntPtr(int* input);
int crossThreadIntRef(int& input);
bool isRealException(const TestException& ex);
void crossThreadException();
void aborted();

/************************************************************************
** Synchronous Suite, Test 1: Parameters
*/

void testParametersSynch()
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
** Synchronous Suite, Test 2: Pointer to class return value
*/

void testReturnValuesSynch()
{
	ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>* scheduler = ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>::getInstance();

	boost::function<boost::shared_ptr<SharedClass>()> callback1 = crossThreadPtr;
	boost::shared_ptr<SharedClass> ptr = scheduler->syncCall(g_dwThreadId, callback1, INFINITE);
	BOOST_CHECK(ptr->refcount == 1);
}

/************************************************************************
** Synchronous Suite, Test 3: Exceptions
*/

void makeThrowCrossCall()
{
    ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>* scheduler = ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>::getInstance();
    scheduler->syncCall<void, ExceptionTypes<TestException>>(g_dwThreadId, crossThreadException, INFINITE);
}

void testExceptionsSynch()
{
    BOOST_CHECK_EXCEPTION(makeThrowCrossCall(), TestException, isRealException);
}

/************************************************************************
** Synchronous Suite, Test 4: Aborting
*/

void testAbortSynch()
{
    ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>* scheduler = ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>::getInstance();
    SetEvent(g_hTemporarilySuspendEvent);
    Sleep(100);
    BOOST_CHECK_THROW(scheduler->syncCall<void>(g_dwThreadId, aborted, 100), ThreadSynch::CallTimeoutException);
}

/************************************************************************
** Asynchronous Suite, Test 1: Parameters
*/

void testParametersAsynch()
{
    ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>* scheduler = ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>::getInstance();

    int input1 = 0x42;
    boost::function<int()> callback1 = boost::bind(crossThreadIntValue, input1);
    ThreadSynch::Future<int> f1 = scheduler->asyncCall(g_dwThreadId, callback1);
    f1.wait(INFINITE);
    BOOST_CHECK(callback1() == f1.getValue());

    boost::scoped_ptr<int> input2(new int);
    *input2 = 0x42;
    boost::function<int()> callback2 = boost::bind(crossThreadIntPtr, input2.get());
    ThreadSynch::Future<int> f2 = scheduler->asyncCall(g_dwThreadId, callback2);
    f2.wait(INFINITE);
    BOOST_CHECK(callback2() == f2.getValue());

    int input3 = 0x42;
    boost::function<int()> callback3 = boost::bind(crossThreadIntRef, boost::ref(input3));
    ThreadSynch::Future<int> f3 = scheduler->asyncCall(g_dwThreadId, callback3);
    f3.wait(INFINITE);
    BOOST_CHECK(callback3() == f3.getValue());
}

/************************************************************************
** Asynchronous Suite, Test 2: Pointer to class return value
*/

void testReturnValuesAsynch()
{
    ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>* scheduler = ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>::getInstance();
    boost::function<boost::shared_ptr<SharedClass>()> callback1 = crossThreadPtr;
    ThreadSynch::Future<boost::shared_ptr<SharedClass>> f_ptr = scheduler->asyncCall(g_dwThreadId, callback1);
    f_ptr.wait(INFINITE);
    BOOST_CHECK(f_ptr.getValue()->refcount == 1);
}

/************************************************************************
** Asynchronous Suite, Test 3: Exceptions
*/

void testExceptionsAsynch()
{
    ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>* scheduler = ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>::getInstance();
    ThreadSynch::Future<void> f = scheduler->asyncCall<void, ExceptionTypes<TestException>>(g_dwThreadId, crossThreadException);
    f.wait(INFINITE);
    BOOST_CHECK_EXCEPTION(f.abort(), TestException, isRealException);
}

/************************************************************************
** Asynchronous Suite, Test 4: Aborting
*/

void testAbortAsynch()
{
    ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>* scheduler = ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>::getInstance();
    SetEvent(g_hTemporarilySuspendEvent);
    Sleep(50);
    ThreadSynch::Future<void> f = scheduler->asyncCall<void>(g_dwThreadId, aborted);
    f.wait(100);
    // abort by letting the Future-object fall out of scope
}

/************************************************************************
** Test Setup
*/

struct ThreadSynchTestSuite : test_suite
{
    ThreadSynchTestSuite()
    {
        g_hCloseEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        g_hTemporarilySuspendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        g_hTestThread = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(testThread), NULL, 0, &g_dwThreadId);

        // Synchronous test cases
        add(BOOST_TEST_CASE(&testAbortSynch));
        add(BOOST_TEST_CASE(&testParametersSynch));
        add(BOOST_TEST_CASE(&testReturnValuesSynch));
        add(BOOST_TEST_CASE(&testExceptionsSynch));

        // Asynchronous test cases
        add(BOOST_TEST_CASE(&testAbortAsynch));
        add(BOOST_TEST_CASE(&testParametersAsynch));
        add(BOOST_TEST_CASE(&testReturnValuesAsynch));
        add(BOOST_TEST_CASE(&testExceptionsAsynch));
    }

    ~ThreadSynchTestSuite()
    {
        SetEvent(g_hCloseEvent);
        WaitForSingleObject(g_hTestThread, INFINITE);
        CloseHandle(g_hTestThread);
        CloseHandle(g_hCloseEvent);

        // Delete the singleton, for the sake of leak detection.
        // Some globals will however be detected either way, so don't be alarmed by the notification for the time being.
        // The key point is: The leak doesn't grow.
        delete ThreadSynch::CallScheduler<ThreadSynch::APCPickupPolicy>::getInstance();
    }
};

test_suite* init_unit_test_suite(int argc, char * argv[]) 
{
    test_suite* test(BOOST_TEST_SUITE("ThreadSynch test suite"));
    test->add(new ThreadSynchTestSuite);
    return test;
}

/************************************************************************
** Test helper structs and functions
*/

DWORD WINAPI testThread(PVOID)
{
    HANDLE handles[] = {g_hCloseEvent, g_hTemporarilySuspendEvent};
    DWORD dwWaitResult;
    while((dwWaitResult = WaitForMultipleObjectsEx(2, handles, FALSE, INFINITE, TRUE)) != WAIT_OBJECT_0)
    {
        if(dwWaitResult == WAIT_OBJECT_0 + 1)
        {
            // The thread was signaled to sleep for a few seconds
            BOOST_MESSAGE("Worker thread sleeping a few seconds ...");
            Sleep(3000);
            BOOST_MESSAGE("Worker thread resuming");
        }
        else
        {
            // The thread was woken from an error or more likely an alertable event
            Sleep(1);
        }
    }
    return 0;
}

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

boost::shared_ptr<SharedClass> crossThreadPtr()
{
    boost::shared_ptr<SharedClass> ptr(new SharedClass);
    BOOST_CHECK(ptr->refcount == 1);
    return ptr;
}

bool isRealException(const TestException& ex)
{
    return ex.magicNumber == 42;
}

void crossThreadException()
{
    throw TestException(42);
}

void aborted()
{
    BOOST_FAIL("Aborted cross thread call was executed -- Failing hard!");
}