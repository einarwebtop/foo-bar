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
#include "AsyncCaller.h"
#include "CallbackInfo.h"

AsyncCaller* AsyncCaller::m_pInstance = NULL;
CriticalSection AsyncCaller::m_instanceCritSect;

AsyncCaller::AsyncCaller()
{
}

AsyncCaller* AsyncCaller::getInstance()
{
	m_instanceCritSect.enter();
	if(m_pInstance == NULL)
	{
		m_pInstance = new AsyncCaller();
	}
	return m_pInstance;
	m_instanceCritSect.leave();
}

void APIENTRY AsyncCaller::APCCallback(ULONG_PTR dwParam)
{
	AsyncCaller* pInstance = reinterpret_cast<AsyncCaller*>(dwParam);
	DWORD dwThread = GetCurrentThreadId();

	pInstance->m_threadQueueCritSect.enter();
	if(pInstance->m_threadQueue.find(dwThread) != pInstance->m_threadQueue.end())
	{
		CALLBACKINFOLIST* pList = &pInstance->m_threadQueue[dwThread];
		for(CALLBACKINFOLIST::iterator i = pList->begin(); i != pList->end(); ++i)
		{
			//if((*i)->isCompleted())
			//{
			//	continue;
			//}

			//(*i)->executeCallback();
		}
	}
	pInstance->m_threadQueueCritSect.leave();
}
