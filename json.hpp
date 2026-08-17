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

/*!
 * \mainpage
 * For more information on the JSON standard, visit: http://www.json.org/
 *
 * This library is designed to seamlessly integrate JSON parsing, encoding
 * and data access into c++ in a way that looks similar to Javascript or c++ structures.
 *
 * For example:
 \code
 #include "json.hpp"

 #include <iostream>

 int main(int argc, char ** argv) {
 document doc;

 for (int i = 0; i < 10; i++) {
 doc["one"][i] = true;
 }

 std::cout << doc.write(true);

 return 0;
 }
 \endcode
 * Will produce the following JSON:
 \code
 {
 "one": [
 true,
 true,
 true,
 true,
 true,
 true,
 true,
 true,
 true,
 true
 ]
 }
 \endcode
 */
#pragma once


#define JSON_DOCUMENT_VERSION "1.1.1"
#define JSON_NUMBER_PRECISION 14

namespace json
{
	class value;
	class document;
	class object;
	class array;
}	 // namespace json

#if defined SUPPORT_ORDERED_JSON
namespace ojson
{
	class value;
	class document;
	class object;
	class array;
}	 // namespace ojson

#if defined USE_DATA_DOCUMENT
namespace data
{
	class value;
	class document;
}	 // namespace ojson

namespace odata
{
	class value;
	class document;
}	 // namespace ojson
#endif

#endif

#define JSON_NAMESPACE json
#include "json_main.hpp"

#if defined SUPPORT_ORDERED_JSON

#if defined JSON_NAMESPACE
#	undef JSON_NAMESPACE
#endif

#define JSON_NAMESPACE ojson
#define DO_OJSON_STUFF
#include "json_main.hpp"

#if !defined JSON_USE_ADDED_ORDER
#if defined JSON_NAMESPACE
#	undef JSON_NAMESPACE
#endif
#if defined DO_OJSON_STUFF
#	undef DO_OJSON_STUFF
#endif
#define JSON_NAMESPACE json
#endif

#endif