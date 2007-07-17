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

#include "ThrowHooked.h"

namespace ThreadSynch
{
	namespace details
	{
		enum CaughtExceptionType
		{
			CaughtExceptionType_None,
			CaughtExceptionType_Expected,
			CaughtExceptionType_Unknown
		};
	}

	/*!@class ExceptionExpecter
	** @brief A helper class which can call call a functor and catch a set of expected exceptions.
	** @remark
	**   If the class catches one of the expected exceptions, this exception can be rethrown,
	**   possibly in context of another thread. The rethrower also accepts a functor, which will 
	**   be called once the rethrown exception object is destroyed.
	*/		
	template<typename E, int N = boost::mpl::size<E>::type::value>
	class ExceptionExpecter;

	// A shortened version to refer to the exception type at position n in the MPL vector
	#define EXCEPTION_TYPE(n) typename boost::mpl::at<E, boost::mpl::int_<n>>::type

	// A generic catch block
	// Todo: The catch block would be more efficient if we could 
	// store a copy of the exception object instead of copying it
	// throughout the functor copies. the same goes for the functor 
	// itself. Both the functor and exception object can possibly be
	// linked to the ExceptionExpecter class' lifetime.
	#define CATCH(rep2_z, ExpectedId, rep2_data) /*****************************************/\
		catch(EXCEPTION_TYPE(ExpectedId)& e)												\
		{																					\
			m_caughtExceptionType = details::CaughtExceptionType_Expected;					\
			m_rethrowFunctor = boost::bind(throwEx ## ExpectedId, e, _1);					\
			m_onCompleteFunctor(m_caughtExceptionType);										\
			return;																			\
		} /*********************************************************************************/

	// A catch block which deals with an exception object of one of the
	// expected types. Each type in the vector of expected exceptions will
	// be iterated and catched.
	#define THROWEX(rep2_z, ExpectedId, rep2_data) /***************************************/\
		static void rep2_data ## ExpectedId(const EXCEPTION_TYPE(ExpectedId)& exObj,		\
											boost::function<void()>& onDestroyException)	\
		{																					\
			details::throwHooked(exObj, onDestroyException);								\
		} /*********************************************************************************/

	// Generate the ExceptionExpecter class specializations for up to THREADSYNCH_MAX_EXPECTED_EXCEPTIONS
	// predicted exception types
	#define BOOST_PP_ITERATION_PARAMS_1 (3,(0,THREADSYNCH_MAX_EXPECTED_EXCEPTIONS,"ExceptionExpecter_template.h"))
		??=include BOOST_PP_ITERATE()
	#undef BOOST_PP_ITERATION_PARAMS_1
}
