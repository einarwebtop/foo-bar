/************************************************************************
** ThreadSynchdemoWM
** A GUI example for the ThreadSynch library.
**
** Note that this code is factored, and not structured as Win32 GUI apps
** should be. Seeing as this is just an example, I've attempted to keep
** it as short and concise as possible.
**
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
#include "ThreadSynchDemoWM.h"

using namespace std;
using namespace ThreadSynch;

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(nCmdShow);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(hPrevInstance);

	// Obtain a call scheduler
	g_callScheduler = CallScheduler<WMPickup>::getInstance();

	// Create a dialog
	g_hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_THREADSYNCHDEMOWM_DIALOG), NULL, reinterpret_cast<DLGPROC>(dlgProc)); 
	ShowWindow(g_hDlg, SW_SHOW); 

	MSG msg;
	BOOL bRet;
	while((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) 
	{ 
		// Catch pickup notifications made to this thread's message queue.
		// Since the thread that runs this message loop is also the owner
		// of the window, calls through these pickups can safely access
		// data owned by the window.
		// See the ThreadSynchdemoWM.h header for a definition of WMPickup.
		if(msg.message == WMPickup::WM_PICKUP)
		{
			WMPickup::executeCallback(msg.wParam, msg.lParam);
			continue;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	} 

	return 0;
}

LRESULT CALLBACK dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	int i; // Counter for spawning multiple worker threads

	switch(message)
	{
	case WM_INITDIALOG:
		g_dwGuiThread = GetCurrentThreadId();
		return TRUE;
	
	case WM_COMMAND:
		if(LOWORD(wParam) == IDC_BUTTON1)
		{
			// Spawn ten more worker threads
			for(i = 0; i < 10; ++i)
			{
				CreateThread(NULL, 0, demoWorkerThread, NULL, 0, NULL);
			}
			
			// Update the button text
			g_dwWorkerThreads += 10;
			SendMessage(GetDlgItem(hDlg, IDC_BUTTON1), 
						WM_SETTEXT, 
						NULL, 
						reinterpret_cast<LPARAM>(string("Spawn more workers (currently " + tostring(g_dwWorkerThreads) + ")").c_str()));
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hDlg);
		PostQuitMessage(0);
		return TRUE;
	}

	return FALSE;
}

// A worker thread which tries to update some text in the GUI every second
DWORD WINAPI demoWorkerThread(PVOID)
{
	string myText = "Some text, from thread " + tostring(GetCurrentThreadId());

	while(TRUE)
	{
		try
		{
			// Since many workers run at the same time, there's a chance that
			// more than one thread will call updateText simultaneously. This
			// calls for synchronization if updateText performs any non-atomic
			// data updates, or the order of events within updateText is critical.
			// The worker threads won't have to worry about that though. Just
			// call updateText, and let that take care of the rest.
			updateText(myText);
		}
		catch(ThreadSynch::CallTimeoutException&)
		{
			// The call timed out. This probably means that the GUI has too much
			// to do at the moment, so instead of waiting around, we do some more
			// calculations, and try again later.
		}
		catch(ThreadSynch::CallSchedulingFailedException&)
		{
			MessageBox(0, "Call scheduling failed", "Error", MB_SETFOREGROUND);
		}

		// Do some lengthy calculations, and non-GUI related operations on
		// data owned by this worker thread
		/* ... */

		Sleep(10);
	}
}

// A function which "belongs to" the GUI thread
void updateText(const string& textToAdd)
{
	// See if the call originates from the GUI thread
	if(GetCurrentThreadId() != g_dwGuiThread)
	{
		// Since the call was not made in context of the GUI thread,
		// we'll want to schedule the call to be made through that.
		// The GUI thread will process one call at at time, so even
		// though several calls may make it here at the same time
		// (as mentioned above), the real body of this function will
		// be serialized.
		g_callScheduler->syncCall<void>(g_dwGuiThread, boost::bind(updateText, boost::ref(textToAdd)), 100);
		return;
	}

	// We are free to do some synchronized operations from here on out. 
	// All calls that reach this point are serialized, meaning that they all
    // come in context of the same thread, which means that there will be no
    // race for "shared" resources.
	/* ... */

	// Update the call counter and set some new text in the text box
	g_dwCallsMade++;
	string updatedText = "Number of synchronized calls: " + tostring(g_dwCallsMade) + ". Last string: " + textToAdd;
	SendMessage(GetDlgItem(g_hDlg, IDC_STATIC1), WM_SETTEXT, NULL, reinterpret_cast<LPARAM>(updatedText.c_str()));
}