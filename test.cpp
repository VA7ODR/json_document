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
#include "data.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdarg.h>

using namespace std;
using namespace std::string_literals;

const char szASCIIChars[257] =
	"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";

void Debug(const char * format, ...)
{
	std::string s;

	size_t n, size = 100;

	bool b = false;

	va_list marker;

	while (!b) {
		s.resize(size);
		va_start(marker, format);
		n = vsnprintf((char *)s.c_str(), size, format, marker);
		va_end(marker);
		b = (n < size);
		if (n > 0 && n != (size_t)-1 && b) {
			size = n;
		} else if (n == (size_t)-1) {
			size = size * 2;	// stupid nonconformant microsoft
		} else {
			size = n * 2;
		}
	}
	printf("%s", s.c_str());
}

sdstring dePretty(const sdstring & in)
{
	sdstring output;
	output.reserve(in.size());
	bool bString = false;
	char cLast	 = '\0';
	for (auto & c : in) {
		switch (c) {
			default:
				output.push_back(c);
				break;

			case ' ':
			case '\t':
			case '\r':
			case '\n':
				if (bString) {
					output.push_back(c);
				}
				break;

			case '\"':
				if (bString) {
					if (cLast != '\\') {
						bString = false;
					}
				} else {
					bString = true;
				}
				output.push_back(c);
				break;
		}
		cLast = c;
	}
	output.shrink_to_fit();
	return output;
}

sdstring escapeChar(const char & ch)
{
	std::string output;
	switch (ch) {
		case '"':
			output += "\\\"";
			break;
		case '\\':
			output += "\\\\";
			break;
		case '\b':
			output += "\\b";
			break;
		case '\f':
			output += "\\f";
			break;
		case '\n':
			output += "\\n";
			break;
		case '\r':
			output += "\\r";
			break;
		case '\t':
			output += "\\t";
			break;
		default:
			if (ch < 0x20 || ch >= 0x7F) {
				char szHEX[32];
				sprintf(szHEX, "%.2X", (unsigned int)ch & 0xFF);
				output += "\\u00";
				output += szHEX;
			} else {
				output += ch;
			}
			break;
	}
	return output;
}

std::string escape_json(const std::string & input)
{
	std::string output;
	for (char ch : input) {
		output.append(escapeChar(ch));
	}
	return output;
}

#define NUMBERTEST(x, y, a, z)                                                          \
	if ((a)x.y != (a)z) {                                                               \
		std::cout << "FAILED!" << std::endl;                                            \
		std::cerr << #x << "." << #y << " != " << z << " (" << x.y << ")" << std::endl; \
		exit(-1);                                                                       \
	}
#define NUMBERSTRTEST(x, y, z)                                                          \
	if (x.y != z) {                                                                     \
		std::cout << "FAILED!" << std::endl;                                            \
		std::cerr << #x << "." << #y << " != " << z << " (" << x.y << ")" << std::endl; \
		exit(-1);                                                                       \
	}
#define NUMBERTESTS(x, y) numberTests(x, y, #y)

template <class T>
void test(const sdstring & type, const sdstring & sGroundTruth)
{
	std::cout << "Running test: " << type << std::endl;
	std::cout << "Parsing from string... ";

	T parseTest;
	if (parseTest.parse(sGroundTruth)) {
		std::cout << "Success!" << std::endl;

		auto numberTests = [&](const sdstring & sTag, auto val, const sdstring & sExpectedStr)
		{
			std::cout << "Testing " << sTag << "... ";
			NUMBERTEST(parseTest[sTag], number(), double, val);
			NUMBERTEST(parseTest[sTag], _double(), double, val);
			NUMBERTEST(parseTest[sTag], _float(), float, val);
			NUMBERTEST(parseTest[sTag], integer(), int64_t, val);
			NUMBERTEST(parseTest[sTag], _int64(), int64_t, val);
			NUMBERTEST(parseTest[sTag], _uint64(), uint64_t, val);
			NUMBERTEST(parseTest[sTag], _size_t(), size_t, val);
			NUMBERTEST(parseTest[sTag], _long(), long, val);
			NUMBERTEST(parseTest[sTag], _ulong(), unsigned long, val);
			NUMBERTEST(parseTest[sTag], _int(), int, val);
			NUMBERTEST(parseTest[sTag], _int32(), int32_t, val);
			NUMBERTEST(parseTest[sTag], _uint(), uint32_t, val);
			NUMBERTEST(parseTest[sTag], _short(), short, val);
			NUMBERTEST(parseTest[sTag], _ushort(), unsigned short, val);
			NUMBERTEST(parseTest[sTag], _int16(), int16_t, val);
			NUMBERTEST(parseTest[sTag], _uint16(), uint16_t, val);
			NUMBERTEST(parseTest[sTag], _char(), char, val);
			NUMBERTEST(parseTest[sTag], _uchar(), unsigned char, val);
			NUMBERTEST(parseTest[sTag], _int8(), int8_t, val);
			NUMBERTEST(parseTest[sTag], _uint8(), uint8_t, val);
			NUMBERTEST(parseTest[sTag], boolean(), bool, (val != 0));
			NUMBERSTRTEST(parseTest[sTag], _sdstring(), sExpectedStr);
			if (parseTest[sTag].isA() != (int)json::JSON_NUMBER) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest[sTag].isA() != json::JSON_NUMBER. got: " << parseTest[sTag].isA() << " expected " << json::JSON_NUMBER << std::endl;
				exit(-1);
			}
			if (!parseTest[sTag].isA((int)json::JSON_NUMBER)) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest[sTag].isA(json::JSON_NUMBER) returned not a number." << std::endl;
				exit(-1);
			}
			std::cout << "Success!" << std::endl;
		};

		NUMBERTESTS("integer", 42);
		NUMBERTESTS("negative_integer", -123);
		NUMBERTESTS("float", 3.14);
		NUMBERTESTS("negative_float", -5.67);

		auto stringTests = [&](const sdstring & sTag, const char * szExpected, size_t expectedLen)
		{
			std::cout << "Testing " << sTag << "... ";
			if (parseTest[sTag].isA() != json::JSON_STRING) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest[sTag].isA() != json::JSON_STRING. got: " << parseTest[sTag].isA() << " expected " << json::JSON_STRING << std::endl;
				exit(-1);
			}
			if (!parseTest[sTag].isA(json::JSON_STRING)) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest[sTag].isA(json::JSON_STRING) returned not a number." << std::endl;
				exit(-1);
			}
			sdstring & sSDVal  = parseTest[sTag]._sdstring();
			std::string & sVal = parseTest[sTag].string();
			size_t stSDSize	   = sSDVal.size();
			size_t stSize	   = sVal.size();
			if (expectedLen != stSDSize) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "sdstring size mismatch " << stSDSize << " != expected " << expectedLen << std::endl;
				exit(-1);
			}
			if (expectedLen != stSDSize) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "std::string size mismatch " << stSize << " != expected " << expectedLen << std::endl;
				exit(-1);
			}
			auto itSDVal   = sSDVal.begin();
			auto itVal	   = sVal.begin();
			const char * d = sVal.c_str();
			const char * c = szExpected;
			for (size_t i = 0; i < expectedLen; ++i, ++itSDVal, ++itVal, ++c, ++d) {
				if ((unsigned char)*c != (unsigned char)*itSDVal) {
					std::cout << "FAILED!" << std::endl;
					std::cerr << "sdstring character mismatch at " << i << " " << escapeChar(*itSDVal) << " != expected " << escapeChar(*c) << std::endl;
					std::cerr << escape_json(std::string(szExpected, i)) << std::endl;
					exit(-1);
				}
				if ((unsigned char)*c != (unsigned char)*itVal) {
					std::cout << "FAILED!" << std::endl;
					std::cerr << "std::string character mismatch at " << i << " " << escapeChar(*itVal) << " != expected " << escapeChar(*c) << std::endl;
					std::cerr << escape_json(std::string(szExpected, i)) << std::endl;
					exit(-1);
				}
				if ((unsigned char)*c != (unsigned char)*d) {
					std::cout << "FAILED!" << std::endl;
					std::cerr << "c_str character mismatch at " << i << " " << escapeChar(*d) << " != expected " << escapeChar(*c) << std::endl;
					std::cerr << escape_json(std::string(szExpected, i)) << std::endl;
					exit(-1);
				}
			}
			std::cout << "Success!" << std::endl;
		};
		stringTests("string", szASCIIChars, 256);

		std::cout << "Testing "
				  << "nested_object"
				  << "... ";
		if (parseTest["nested_object"].isA() != json::JSON_OBJECT) {
			std::cout << "FAILED!" << std::endl;
			std::cerr << "parseTest[\"nested_object\"].isA() says is is not an object: " << parseTest["nested_object"].isA() << " expected " << json::JSON_OBJECT << std::endl;
			exit(-1);
		}
		if (!parseTest["nested_object"].isA(json::JSON_OBJECT)) {
			std::cout << "FAILED!" << std::endl;
			std::cerr << "parseTest[\"nested_object\"].isA(json::JSON_OBJECT) says is is not an object." << std::endl;
			exit(-1);
		}
		if (parseTest["nested_object"].size() != 3) {
			std::cout << "FAILED!" << std::endl;
			std::cerr << "parseTest[\"nested_object\"].size() reported an improper value: " << parseTest["nested_object"].size() << " expected " << 3 << std::endl;
			exit(-1);
		}

		std::map<sdstring, size_t> expectedTags = {
			{"nested_integer", 0},
			   {	"nested_float", 0},
			{ "nested_string", 0}
		};
		std::map<sdstring, size_t> expectedTypes = {
			{"nested_integer", json::JSON_NUMBER},
			   {	"nested_float", json::JSON_NUMBER},
			{ "nested_string", json::JSON_STRING}
		};
		auto it		 = parseTest["nested_object"].begin();
		auto end	 = parseTest["nested_object"].end();
		size_t count = 0;
		for (auto & val : parseTest["nested_object"]) {
			auto sValKey = val.key();
			if (parseTest["nested_object"][sValKey] != val) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest[\"nested_object\"][\"" << sValKey << "\"] != val " << parseTest["nested_object"][sValKey]._sdstring() << " vs. " << val._sdstring() << std::endl;
				exit(-1);
			}
			if (count > 3) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "auto & val did not stop after 3." << std::endl;
				exit(-1);
			}
			if (expectedTags.find(sValKey) == expectedTags.end()) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "auto & val unexpected key " << sValKey << "." << std::endl;
				exit(-1);
			}
			if (expectedTags[sValKey]) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "auto & val key " << sValKey << " already found." << std::endl;
				exit(-1);
			}
			expectedTags[sValKey]++;
			if (it == end) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "*it unexpected end after " << count << "." << std::endl;
				exit(-1);
			}
			auto sItKey = (*it).key();
			if (sItKey != sValKey) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "(*it).key() != val.key() " << sItKey << " vs " << sValKey << std::endl;
				exit(-1);
			}
			if (parseTest["nested_object"][sItKey] != *it) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest[\"nested_object\"][\"" << sValKey << "\"] != *it " << parseTest["nested_object"][sValKey]._sdstring() << " vs. " << (*it)._sdstring() << std::endl;
				exit(-1);
			}
			if (val != *it) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "val(" << sValKey << ") != *it " << parseTest["nested_object"][sValKey]._sdstring() << " vs. " << val._sdstring() << std::endl;
				exit(-1);
			}
			if (expectedTags.find(sItKey) == expectedTags.end()) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "*it unexpected key " << sItKey << "." << std::endl;
				exit(-1);
			}
			if (expectedTags[sItKey] > 1) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "auto & val key " << sItKey << " already found." << std::endl;
				exit(-1);
			}
			if (expectedTags[sItKey] < 1) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "auto & val key " << sItKey << " should have already been found." << std::endl;
				exit(-1);
			}
			expectedTags[sItKey]++;
			++it;
			++count;
		}
		if (it != end) {
			std::cout << "FAILED!" << std::endl;
			std::cerr << "it != end after 3." << std::endl;
			exit(-1);
		}

		if (type == "ojson" || type == "odata") {
			auto it	 = parseTest["nested_object"].begin();
			auto end = parseTest["nested_object"].end();
			if (it == end) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest[\"nested_object\"].begin() == : parseTest[\"nested_object\"].end()" << std::endl;
				exit(-1);
			}
			if (it.key() != "nested_integer") {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "*parseTest[\"nested_object\"].begin().key() != nested_integer: " << it.key().c_str() << std::endl;
				std::cerr << "TypeInfo: " << parseTest.classInfo() << std::endl;
				exit(-1);
			}
			if (*it != 789) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest[\"nested_object\"][\"nested_integer\"] != 789: " << (*it)._int() << std::endl;
				exit(-1);
			}
			++it;
			if (it == end) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest[\"nested_object\"].begin() + 1 == : parseTest[\"nested_object\"].end()" << std::endl;
				exit(-1);
			}
			if (it.key() != "nested_float") {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "*parseTest[\"nested_object\"].begin().key() != nested_float: " << it.key().c_str() << std::endl;
				std::cerr << "TypeInfo: " << parseTest.classInfo() << std::endl;
				exit(-1);
			}
			++it;
			if (it == end) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest[\"nested_object\"].begin() + 1 == : parseTest[\"nested_object\"].end()" << std::endl;
				exit(-1);
			}
			if (it.key() != "nested_string") {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "*parseTest[\"nested_object\"].begin().key() != nested_string: " << it.key().c_str() << std::endl;
				std::cerr << "TypeInfo: " << parseTest.classInfo() << std::endl;
				exit(-1);
			}
		}
		std::cout << "Success!" << std::endl;

		std::cout << "Testing "
				  << "nested_array"
				  << "... ";
		if (parseTest["nested_array"].isA() != json::JSON_ARRAY) {
			std::cout << "FAILED!" << std::endl;
			std::cerr << "parseTest[\"nested_array\"].isA() says is is not an object: " << parseTest["nested_array"].isA() << " expected " << json::JSON_ARRAY << std::endl;
			exit(-1);
		}
		if (!parseTest["nested_array"].isA(json::JSON_ARRAY)) {
			std::cout << "FAILED!" << std::endl;
			std::cerr << "parseTest[\"nested_array\"].isA(json::JSON_ARRAY) says is is not an object." << std::endl;
			exit(-1);
		}
		if (parseTest["nested_array"].size() != 3) {
			std::cout << "FAILED!" << std::endl;
			std::cerr << "parseTest[\"nested_array\"].size() reported an improper value: " << parseTest["nested_array"].size() << " expected " << 3 << std::endl;
			exit(-1);
		}

		it											   = parseTest["nested_array"].begin();
		end											   = parseTest["nested_array"].end();
		count										   = 0;
		std::deque<json::JSONTypes> expectedArrayTypes = {json::JSON_NULL, json::JSON_STRING, json::JSON_BOOLEAN};
		std::deque<sdstring> expectedArrayStrings	   = {"", "two", "true"};
		std::deque<T> arrayValues					   = {(const char *)nullptr,"two", true, 4, 4.6f};
		for (auto & val : parseTest["nested_array"]) {
			if (val.isA() != expectedArrayTypes[count]) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << count << " val.isA() incorrect: got: " << json::value::typeName(val.isA()) << " expected " << json::value::typeName(expectedArrayTypes[count]) << std::endl;
				exit(-1);
			}
			if (!val.isA(expectedArrayTypes[count])) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << count << " !val.isA(" << expectedArrayTypes[count] << ") incorrect." << std::endl;
				exit(-1);
			}
			if ((*it).isA() != expectedArrayTypes[count]) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << count << " (*it).isA() incorrect: got: " << (*it).isA() << " expected " << expectedArrayTypes[count] << std::endl;
				exit(-1);
			}
			if (!(*it).isA(expectedArrayTypes[count])) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << count << " !(*it).isA(" << expectedArrayTypes[count] << ") incorrect." << std::endl;
				exit(-1);
			}
			if (val != (*it)) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << count << " val != (*it): " << val << " vs " << (*it) << std::endl;
				exit(-1);
			}
			if (val._sdstring() != expectedArrayStrings[count]) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << count << " val != \"" << expectedArrayStrings[count] << "\" got \"" << val._sdstring() << "\"" << std::endl;
				exit(-1);
			}
			if ((*it)._sdstring() != expectedArrayStrings[count]) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << count << " (*it) != \"" << expectedArrayStrings[count] << "\" got \"" << (*it)._sdstring() << "\"" << std::endl;
				exit(-1);
			}
			if (val != arrayValues[count]) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << count << " val != : " << arrayValues[count] << " vs " << val << std::endl;
				exit(-1);
			}
			if ((*it) != arrayValues[count]) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << count << " (*it) != : " << arrayValues[count] << " vs " << (*it) << std::endl;
				exit(-1);
			}

			++count;
			++it;
		}
		std::cout << "Success!" << std::endl;

		std::cout << "Testing "
				  << "writeFile"
				  << "... ";
		if (parseTest.writeFile(type + "First.json", true)) {
			std::error_code ec;
			if (!std::filesystem::is_regular_file((type + "First.json").c_str(), ec)) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "failed to write " << type << "First.json: " << ec.message() << std::endl;
				exit(-1);
			}
			auto fs = std::filesystem::file_size((type + "First.json").c_str(), ec);
			if (fs != sGroundTruth.size()) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "File Size Mismatch on " << type << "First.json: " << fs << " vs " << sGroundTruth.size() << std::endl;
				exit(-1);
			}

			std::cout << "Success!" << std::endl;
		} else {
			std::cout << "FAILED!" << std::endl;
			std::cerr << "failed to write " << type << "First.json" << std::endl;
			exit(-1);
		}

		std::cout << "Testing "
				  << "parseFile"
				  << "... ";
		T parseTest2;
		if (parseTest2.parseFile("ground_truth.json")) {
			if (parseTest != parseTest2) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest != parseTest2" << std::endl;
				exit(-1);
			}
			if (parseTest.write(true) != parseTest2.write(true)) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest.write(true) != parseTest2.write(true)" << std::endl;
				exit(-1);
			}
			if (parseTest.write(false) != parseTest2.write(false)) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest.write(false) != parseTest2.write(false)" << std::endl;
				exit(-1);
			}
			if (dePretty(parseTest.write(true)) != dePretty(parseTest2.write(true))) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "dePretty(parseTest.write(true)) != dePretty(parseTest2.write(true))" << std::endl;
				exit(-1);
			}
			if (parseTest.write(false) != dePretty(parseTest2.write(true))) {
				std::cout << "FAILED!" << std::endl;
				std::cerr << "parseTest.write(false) != dePretty(parseTest2.write(true))" << std::endl;
				std::cerr << parseTest.write(false) << std::endl << std::endl;
				std::cerr << dePretty(parseTest2.write(true)) << std::endl;
				exit(-1);
			}
			if (type == "ojson" || type == "odata") {
				if (parseTest.write(true) != sGroundTruth) {
					std::cout << "FAILED!" << std::endl;
					std::cerr << "parseTest.write(true) != sGroundTruth" << std::endl;
					exit(-1);
				}
				if (parseTest.write(false) != dePretty(sGroundTruth)) {
					std::cout << "FAILED!" << std::endl;
					std::cerr << "parseTest.write(false) != dePretty(sGroundTruth)" << std::endl;
					std::cerr << parseTest.write(false) << std::endl << std::endl;
					std::cerr << dePretty(sGroundTruth) << std::endl;
					exit(-1);
				}
				if (dePretty(parseTest.write(true)) != dePretty(sGroundTruth)) {
					std::cout << "FAILED!" << std::endl;
					std::cerr << "dePretty(parseTest.write(true)) != dePretty(sGroundTruth)" << std::endl;
					exit(-1);
				}
			}
			std::cout << "Success!" << std::endl;
		} else {
			std::cout << "FAILED!" << std::endl;
			std::cerr << "ground_truth.json failed to parse: " << parseTest.parseResult() << std::endl;
			exit(-1);
		}
	} else {
		std::cout << "FAILED!" << std::endl;
		std::cerr << "sGroundTruth failed to parse: " << parseTest.parseResult() << std::endl;
		exit(-1);
	}
	std::cout << std::endl;
}

std::string & lpad(std::string & in, char with, size_t length)
{
	if (length > in.size()) {
		return in.insert(0, length - in.size(), with);
	} else {
		return in;
	}
}

#define TEST(x, y) test<x::document>(#x, y)

int main(int, char **)
{
	std::string ground_truth_original = R"({
	"integer": 42,
	"negative_integer": -123,
	"float": 3.14,
	"negative_float": -5.67,
	"string": ")" + escape_json(sdstring(szASCIIChars, 256)) +
										R"(",
	"nested_object": {
		"nested_integer": 789,
		"nested_float": -456.789,
		"nested_string": ")" + escape_json("Nested String with special characters: \t\n\r\"") +
										R"("
	},
	"nested_array": [
		null,
		"two",
		true
	],
	"array_of_objects": [
		{
			"name": "John",
			"age": 30
		},
		{
			"name": "Jane",
			"age": 25
		}
	],
	"object_of_arrays": {
		"numbers": [
			1,
			2,
			3
		],
		"colors": [
			"red",
			"green",
			"blue"
		]
	},
	"array_of_arrays": [
		[
			1,
			false,
			3
		],
		[
			"apple",
			"orange",
			"banana"
		]
	]
})";

	std::ofstream out("ground_truth.json");
	out << std::setw(2) << ground_truth_original << std::endl;

	TEST(json, ground_truth_original);
#if defined SUPPORT_ORDERED_JSON
	TEST(ojson, ground_truth_original);
#endif
#if defined USE_DATA_DOCUMENT
	TEST(data, ground_truth_original);
#if defined SUPPORT_ORDERED_JSON
	TEST(odata, ground_truth_original);
#endif
#endif

	std::cout << "All tests successful!" << std::endl;
	return 0;
}
