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

// Adjustable parameters

#ifndef THREADSYNCH_MAX_EXPECTED_EXCEPTIONS
#define THREADSYNCH_MAX_EXPECTED_EXCEPTIONS 10
#endif 

// Windows headers and defines

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// STL headers

#include <map>
#include <list>
#include <algorithm>

// Boost headers

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/int.hpp>
#include <boost/preprocessor/repetition/repeat.hpp> 
#include <boost/mpl/size.hpp>
#include <boost/type_traits.hpp>

// ThreadSynch headers

#include "CallScheduler.h"
