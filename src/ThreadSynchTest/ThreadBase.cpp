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
