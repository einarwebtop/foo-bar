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

#include "CiritcalSection.h"
#include "CallbackInfo.h"

// Predecl
//template<typename T> 
//class CallbackInfo;

class AsyncCaller
{
public:
	// Functions
	template<typename T>
	T syncCall(DWORD dwThreadId, boost::function<T (void)> func, DWORD dwTimeout)
	{
		m_threadQueueCritSect.enter();
		BOOL bMustQueueApc = (m_threadQueue.find(dwThreadId) == m_threadQueue.end());
		CallbackInfo* ci = new CallbackInfo();
		ci->setCallback(func);
		ci->executeCallback();
		T retVal = ci->getReturnValue<T>();
		delete ci;
		return retVal;
		//m_threadQueue[dwThreadId].push_back(ci);
		//m_threadQueueCritSect.leave();
		// Todo: Wait for timeout
		// Todo: See if the call was made
		// Todo: Return value or throw exception
		throw std::exception();
	}

	static AsyncCaller* getInstance();

private:
	// Types
	typedef std::list<CallbackInfo*> CALLBACKINFOLIST;
	typedef std::map<DWORD, CALLBACKINFOLIST> THREADQUEUE;

	// Variables
	static AsyncCaller* m_pInstance;
	static CriticalSection m_instanceCritSect;
	CriticalSection m_threadQueueCritSect;
	THREADQUEUE m_threadQueue;
	
	// Functions
	AsyncCaller();
	AsyncCaller(const AsyncCaller&); // not implemented
	const AsyncCaller& operator =(const AsyncCaller&); // not implemented
	static void APIENTRY APCCallback(ULONG_PTR dwParam);
};
