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

#include "PickupPolicyProviderExceptions.h"

namespace ThreadSynch
{
	/*!@class PicupPolicyProvider
	** @brief Interface of all pickup policies.
	*/
	class PickupPolicyProvider
	{
	public:
		typedef void (APIENTRY *PCALLBACK)(ULONG_PTR ulpFunctionParameter);
		static void scheduleThreadCallback(DWORD dwThreadId, PCALLBACK pCallbackFunction, ULONG_PTR ulpFunctionParameter);

	private:
		PickupPolicyProvider();
	};
}