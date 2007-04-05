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

// Todo: Give this class a more meaningful name
class CallbackInfo
{
public:
	CallbackInfo()
		: m_bEntered(FALSE),
		  m_bCompleted(FALSE),
		  m_pReturnValue(NULL)

	{
		m_hCompletedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	~CallbackInfo()
	{
		CloseHandle(m_hCompletedEvent);
		
		// Free the return value
		if(m_pReturnValue != NULL)
		{
			delete [] m_pReturnValue;
		}

		// Deallocate functor, if one is set
		if(m_freeCallback)
		{
			m_freeCallback();
		}
	}

	template<typename T>
	void setCallback(boost::function<T (void)> func)
	{
		// Todo: Disallow multiple runs
		m_pReturnValue = new BYTE[sizeof(T)];
		FunctorSaver<T>* functor = new FunctorSaver<T>(func, m_pReturnValue);
		m_callback = boost::bind(&FunctorSaver<T>::execute, functor);
		m_freeCallback = boost::bind(&FunctorSaver<T>::free, functor);
	}

	DWORD waitForCompletion(DWORD dwTimeout) const
	{
		// Todo: Don't return even on timeout if m_bEntered is TRUE. Must wait! Possible deadlock, though...
		return WaitForSingleObject(m_hCompletedEvent, dwTimeout);
	}

	void executeCallback()
	{
		m_bEntered = TRUE;
		m_callback();
		m_bCompleted = TRUE;
		SetEvent(m_hCompletedEvent);
	}

	inline BOOL isCompleted() const
	{
		return m_bCompleted;
	}

	template<typename T>
	T getReturnValue() const
	{
		return *reinterpret_cast<T*>(m_pReturnValue);
	}

private:
	template<class T>
	class FunctorSaver
	{
	public:
		FunctorSaver(boost::function<T (void)> func, void* ptr)
			: m_func(func),
			m_ptr(ptr)
		{}

		void execute()
		{
			*reinterpret_cast<T*>(m_ptr) = m_func();
		}

		void free()
		{
			delete this;
		}

	private:
		boost::function<T (void)> m_func;
		void* m_ptr;
	};

	HANDLE m_hCompletedEvent;
	BOOL m_bEntered;
	BOOL m_bCompleted;
	void* m_pReturnValue; // Todo: use smart pointer
	boost::function<void ()> m_callback;
	boost::function<void ()> m_freeCallback; // Make the allocated class free itself
};
