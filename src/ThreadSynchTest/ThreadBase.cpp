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
#include "ThreadBase.h"

BOOL ThreadBase::run()
{
	m_hTread = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(startThread), this, 0, &m_dwThread);
	if(m_hTread == NULL)
		return FALSE;
	return TRUE;
}

HANDLE ThreadBase::getThreadHandle() const
{ 
	return m_hTread; 
}

DWORD ThreadBase::getThreadId() const
{ 
	return m_dwThread; 
}

void ThreadBase::join()
{
	WaitForSingleObject(getThreadHandle(), INFINITE);
}

DWORD WINAPI ThreadBase::startThread(ThreadBase* pInstance)
{
	pInstance->start();
	return 0;
}
