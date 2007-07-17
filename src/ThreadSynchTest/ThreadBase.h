#pragma once

class ThreadBase
{
public:
	BOOL run();
	inline HANDLE getThreadHandle() const;
	inline DWORD getThreadId() const;
	void join();

private:
	HANDLE m_hTread;
	DWORD m_dwThread;

	virtual void start() = 0; // Pure virtual
	static DWORD WINAPI startThread(ThreadBase* pInstance);
};