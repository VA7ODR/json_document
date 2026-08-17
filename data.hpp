/*
Copyright (c) 2012-2026 James Baker

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

The official repository for this library is at https://github.com/VA7ODR/json

*/


#pragma once
#include "json.hpp"

#if defined JSON_NAMESPACE
#	undef JSON_NAMESPACE
#endif

#define JSON_NAMESPACE json
#define DATA_NAMESPACE data

#include "data_main.hpp"

#if defined SUPPORT_ORDERED_JSON

#	if defined JSON_NAMESPACE
#		undef JSON_NAMESPACE
#	endif
#	if defined DATA_NAMESPACE
#		undef DATA_NAMESPACE
#	endif

#	define JSON_NAMESPACE ojson
#	define DATA_NAMESPACE odata
#	define DO_ODATA_STUFF

#	include "data_main.hpp"

#	if !defined JSON_USE_ADDED_ORDER
#		if defined JSON_NAMESPACE
#			undef JSON_NAMESPACE
#		endif
#		if defined DATA_NAMESPACE
#			undef DATA_NAMESPACE
#		endif
#		if defined DO_ODATA_STUFF
#			undef DO_ODATA_STUFF
#		endif

#		define JSON_NAMESPACE json
#		define DATA_NAMESPACE data
#	endif

#endif