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

/************************************************************************
** This is a stub class which will be included multiple times in 
** ExceptionExpecter.h. Expansions of preprocessor parameters and
** variables comes from the boost preprocessor helpers used in said
** header.
*/

template<typename E>
class ExceptionExpecter<E, BOOST_PP_ITERATION()>
{
public:
	ExceptionExpecter(boost::function<void(details::CaughtExceptionType)> onCompleteFunctor)
		: m_onCompleteFunctor(onCompleteFunctor),
		  m_caughtExceptionType(details::CaughtExceptionType_None)
	{}
	
	~ExceptionExpecter() {}
	
	// Generate functions to throw an exception of the iterated type,
	// from the expected exceptions type vector
	BOOST_PP_REPEAT_2ND(BOOST_PP_ITERATION(), THROWEX, throwEx)

	void execute(boost::function<void()> functor)
	{
		try
		{
			functor();
		}
		// Generate blocks to catch an exception of the iterated type,
		// from the expected exceptions type vector
		BOOST_PP_REPEAT_2ND(BOOST_PP_ITERATION(), CATCH, 0)
		// Catch other exceptions
		catch(...)
		{
			m_caughtExceptionType = details::CaughtExceptionType_Unknown;
			m_rethrowFunctor = boost::bind(throwExUnexpected, _1);
			m_onCompleteFunctor(m_caughtExceptionType);
			return;
		}

		// No exceptions caught, call the completion functor
		m_onCompleteFunctor(m_caughtExceptionType);
	}

	void rethrow(boost::function<void()> onDestroyException)
	{
		if(m_caughtExceptionType == details::CaughtExceptionType_Expected ||
		   m_caughtExceptionType == details::CaughtExceptionType_Unknown)
		{
			m_rethrowFunctor(onDestroyException);
		}
	}

	/*! 
	** @brief releases any resources currently held by an ExceptionExpecter
	**   instance, and deletes the instance itself.
	*/
	inline void free()
	{ delete this; }

private:
	static void throwExUnexpected(boost::function<void()>& onExceptionDestroyed)
	{
		details::throwHooked(UnexpectedException(), onExceptionDestroyed);
	}

	boost::function<void(details::CaughtExceptionType)> m_onCompleteFunctor;
	boost::function<void(boost::function<void()>&)> m_rethrowFunctor;
	details::CaughtExceptionType m_caughtExceptionType;
};
