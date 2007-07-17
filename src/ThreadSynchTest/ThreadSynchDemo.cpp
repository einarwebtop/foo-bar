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

#include "pch.h"
#include "../ThreadSynch/ThreadSynch.h"
#include "../ThreadSynch/APCPickupPolicy.h"

using namespace std;
using namespace ThreadSynch;

// Global functions
void rundemos();
DWORD WINAPI demoThread(PVOID);
void demoVoidFunction(char c);
int demoIntFunction(char c);
int demoIntFunctionDelayed(char c);
string demoFunction(char c);

// Global variables
HANDLE hExternalEvent;
char globalBuffer[20];

// Dummy exception object
class demoException
{};

// Entry point
int _tmain()
{
	rundemos();

	// Will detect the global buffer + singleton
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    return 0;
}

/*******************************************************************************
** Expected output:
**
** demoFunction returned: aaaaaaaaaaaaaaaaaaa
** demoVoidFunction called with c='a'
** The scheduled call threw a demoException
**
********************************************************************************/

// The primary thread function, which issues calls through the secondary thread
void rundemos()
{
	// Create an event for which our second thread will wait
	hExternalEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Create the secondary thread
	DWORD dwThreadId;
	HANDLE hThread = CreateThread(NULL, 0, demoThread, NULL, 0, &dwThreadId);

	// Obtain a scheduler instance, through which we can make cross thread calls
	CallScheduler<APCPickupPolicy>* scheduler = CallScheduler<APCPickupPolicy>::getInstance();

	// Do a first cross thread call, to a function returning a string
	try
	{
		string dataString = scheduler->syncCall<string, ExceptionTypes<std::exception>>(dwThreadId, boost::bind(demoFunction, 'a'), INFINITE);
		cout << "demoFunction returned: " << dataString << endl;
	}
	catch(CallTimeoutException&)
	{
		cout << "Call timeout." << endl;
	}
	catch(CallSchedulingFailedException&)
	{
		cout << "Call scheduling failed -- Probably a broken pickup policy." << endl;
	}
	catch(std::exception& e)
	{
		cout << "The scheduled call threw a std exception: " << e.what() << endl;
	}

	// Do a second cross thread call, to a function returning nothing
	try
	{
		// Expect a std::exception or demoException
		scheduler->syncCall<ExceptionTypes<std::exception, demoException>>(dwThreadId, boost::bind(demoVoidFunction, 'a'), 500);
	}
	catch(CallTimeoutException&)
	{
		cout << "Call timeout" << endl;
	}
	catch(CallSchedulingFailedException&)
	{
		cout << "Call scheduling failed -- Probably a broken pickup policy." << endl;
	}
	catch(std::exception& e)
	{
		cout << "demoVoidFunction threw a std exception: " << e.what() << endl;
	}
	catch(demoException&)
	{
		cout << "demoVoidFunction threw a demoException" << endl;
	}

    // Do a third cross thread call, to a function returning an int
    try
    {
        // Expect a std::exception or demoException
        int demoInt = scheduler->syncCall<int, ExceptionTypes<std::exception, demoException>>(dwThreadId, boost::bind(demoIntFunction, '!'), 500);
        cout << "demoIntFunction returned: " << demoInt << endl;
    }
    catch(CallTimeoutException&)
    {
        cout << "Call timeout" << endl;
    }
    catch(CallSchedulingFailedException&)
    {
        cout << "Call scheduling failed -- Probably a broken pickup policy." << endl;
    }
    catch(std::exception& e)
    {
        cout << "demoIntFunction threw a std exception: " << e.what() << endl;
    }
    catch(demoException&)
    {
        cout << "demoIntFunction threw a demoException" << endl;
    }

    // Do a fourth cross thread call, to a function returning an int
    try
    {
        // Expect a std::exception or demoException
        Future<int> futureDemoInt = scheduler->asyncCall<int, ExceptionTypes<std::exception, demoException>>(dwThreadId, boost::bind(demoIntFunctionDelayed, '!'));
        while(futureDemoInt.wait(10) == ASYNCH_CALL_PENDING)
        {
            cout << "Still waiting ..." << endl;
        }
        cout << "demoIntFunctionDelayed returned: " << futureDemoInt.getValue() << endl;
    }
    catch(CallSchedulingFailedException&)
    {
        cout << "Call scheduling failed -- Probably a broken pickup policy." << endl;
    }
    catch(std::exception& e)
    {
        cout << "demoIntFunctionDelayed threw a std exception: " << e.what() << endl;
    }
    catch(demoException&)
    {
        cout << "demoIntFunctionDelayed threw a demoException" << endl;
    }

	// Cleanup
	SetEvent(hExternalEvent);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	CloseHandle(hExternalEvent);
}

// The secondary thread function
DWORD WINAPI demoThread(PVOID)
{
	// Keep sleeping while the event is unset
	while(WaitForSingleObjectEx(hExternalEvent, INFINITE, TRUE) != WAIT_OBJECT_0) 
	{
		//Sleep(10);
	}

	// Alter the global data
	for(int i = 0; i < sizeof(globalBuffer) - 1; ++i)
	{
		globalBuffer[i] = 'b';
	}
	globalBuffer[sizeof(globalBuffer) - 1] = 0; // null terminate

	// Return and terminate the thread
	return 0;
}

// A demo function to be called from the secondary thread
void demoVoidFunction(char c)
{
	cout << "demoVoidFunction called with c='" << c << "'" << endl;
	//throw std::exception("demoing, demoing"); // Uncomment to transport exception back to the main thread
	throw demoException();
}

// A demo function to be called from the secondary thread
int demoIntFunction(char c)
{
	cout << "demoIntFunction called with c='" << c << "'" << endl;
	return static_cast<int>(c);
}

// A demo function to be called from the secondary thread
int demoIntFunctionDelayed(char c)
{
    Sleep(500);
    cout << "demoIntFunctionDelayed called with c='" << c << "'" << endl;
    return static_cast<int>(c);
}

// A demo function to be called from the secondary thread
string demoFunction(char c)
{
	for(int i = 0; i < sizeof(globalBuffer) - 1; ++i)
	{
		globalBuffer[i] = c;
	}
	globalBuffer[sizeof(globalBuffer) - 1] = 0; // null terminate
	return globalBuffer;
}