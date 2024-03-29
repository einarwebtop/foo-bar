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

#include "PickupPolicyProvider.h"

namespace ThreadSynch
{
	template<unsigned int WindowMessageId>
	class WMPickupPolicy : public PickupPolicyProvider
	{
	public:
		static const unsigned int WM_PICKUP = WindowMessageId;

		static void scheduleThreadCallback(DWORD dwThreadId, PCALLBACK pCallbackFunction, ULONG_PTR ulpFunctionParameter)
		{
            // Note that a PostThreadMessage approach is unreliable if the window is in a modal loop, as the 
            // posted message will be lost. A far superior approach would be to write a custom policy targeting
            // a specific window's message queue.
			if(!PostThreadMessage(dwThreadId, WM_PICKUP, reinterpret_cast<WPARAM>(pCallbackFunction), static_cast<LPARAM>(ulpFunctionParameter)))
			{
				throw PickupSchedulingFailedException();
			}
		}

		static void executeCallback(WPARAM wParam, LPARAM lParam)
		{
			PCALLBACK pCallbackFunction = reinterpret_cast<PCALLBACK>(wParam);
			ULONG_PTR ulpFunctionParameter = static_cast<ULONG_PTR>(lParam);
			pCallbackFunction(ulpFunctionParameter);
		}
	};
}