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

#include "stdafx.h"
#include "ThreadBase.h"
#include "CallScheduler.h"
#include "APCPickupPolicy.h"

using namespace std;

HANDLE hExternalEvent;
char globalBuffer[20];

void testVoidFunction(char c)
{
	cout << "testVoidFunction called with c='" << c << "'" << endl;
}

string testFunction(char c)
{
	for(int i = 0; i < sizeof(globalBuffer) - 1; ++i)
	{
		globalBuffer[i] = c;
	}
	globalBuffer[sizeof(globalBuffer) - 1] = 0; // null terminate
	return globalBuffer;
}

DWORD WINAPI testThread(PVOID)
{
	// Keep sleeping while the event is unset
	while(WaitForSingleObjectEx(hExternalEvent, INFINITE, TRUE) != WAIT_OBJECT_0) 
	{
		Sleep(10);
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

void runProgram()
{
	hExternalEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	DWORD dwThreadId;
	HANDLE hThread = CreateThread(NULL, 0, testThread, NULL, 0, &dwThreadId);
	
	CallScheduler<APCPickupPolicy>* scheduler = CallScheduler<APCPickupPolicy>::getInstance();

	try
	{
		string dataString = scheduler->syncCall<string>(dwThreadId, boost::bind(testFunction, 'a'), 500);
		cout << "testFunction returned: " << dataString << endl;
	}
	catch(CallTimeoutException&)
	{
		cout << "Call timeout" << endl;
	}
	catch(CallSchedulingFailedException&)
	{
		cout << "Call scheduling failed" << endl;
	}
	
	try
	{
		scheduler->syncCall(dwThreadId, boost::bind(testVoidFunction, 'a'), 500);
	}
	catch(CallTimeoutException&)
	{
		cout << "Call timeout" << endl;
	}
	catch(CallSchedulingFailedException&)
	{
		cout << "Call scheduling failed" << endl;
	}

	SetEvent(hExternalEvent);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	CloseHandle(hExternalEvent);
}

int _tmain(int argc, _TCHAR* argv[])
{
	runProgram();
	_CrtDumpMemoryLeaks(); // Will detect the singleton
	cin.get();
	return 0;
}
