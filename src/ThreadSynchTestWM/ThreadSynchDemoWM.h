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

#pragma once

#include "resource.h"
#include "../ThreadSynch/ThreadSynch.h"
#include "../ThreadSynch/WMPickupPolicy.h"

// Specify that we want to use the window message pickup policy, with notifications through the wm WM_USER+1.
// See ThreadSynchdemoWM.cpp for how this is used.
typedef ThreadSynch::WMPickupPolicy<WM_USER + 1> WMPickup;

// A simple helper function for lexical casts
template<class T> std::string tostring(const T& source)
{
	std::ostringstream oss;
	oss << source;
	return oss.str();
};

// Global function declarations
LRESULT CALLBACK dlgProc(HWND, UINT, WPARAM, LPARAM);
void updateText(const std::string& textToAdd);
DWORD WINAPI demoWorkerThread(PVOID);

// Global variable declarations
HWND g_hDlg = NULL;
DWORD g_dwGuiThread = 0;
unsigned long g_dwWorkerThreads = 0;
unsigned long g_dwCallsMade = 0;
ThreadSynch::CallScheduler<WMPickup>* g_callScheduler;
