/* Copyright (c) 2012-2026 James Baker
 * SPDX-License-Identifier: MIT
 */
#include "data.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

#define CHECK(condition) \
	 do { if (!(condition)) { std::cerr << "FAIL: " << __FILE__ << ':' << __LINE__ << ": " #condition << '\n'; return false; } } while (false)

template<class Value, class Document>
bool value_suite(const char *name)
{
	using namespace std::string_literals;
	std::cout << "Testing " << name << "::value/document\n";

	Value empty;
	CHECK(empty.isA() == json::JSON_VOID);
	CHECK(Value(true).isA(json::JSON_BOOLEAN) && Value(true).boolean());
	CHECK(Value(42).isA(json::JSON_NUMBER) && Value(42).integer() == 42);
	CHECK(Value("text").isA(json::JSON_STRING) && Value("text").string() == "text");

	Document doc;
	const std::string input = R"({
		"null":null,"bool":true,"false":false,"integer":-42,"decimal":1.25e2,
		"text":"line\n\u20ac","array":[null,1,"two"],"object":{"key":"value"}
	})";
	CHECK(doc.parse(input));
	CHECK(doc.parseSuccessful());
	CHECK(doc.isA(json::JSON_OBJECT) && doc.size() == 8);
	CHECK(doc["null"].isA(json::JSON_NULL));
	CHECK(doc["bool"].boolean() && !doc["false"].boolean());
	CHECK(doc["integer"].integer() == -42);
	CHECK(std::fabs(doc["decimal"].number() - 125.0) < 1e-12);
	CHECK(doc["text"].string() == "line\n€");
	CHECK(doc["array"].isA(json::JSON_ARRAY) && doc["array"].size() == 3);
	CHECK(doc["array"][2].string() == "two" && doc["object"]["key"].string() == "value");

	doc["new"] = 7;
	doc["array"].push_back(false);
	doc["array"][4] = "tail";
	CHECK(doc["new"].integer() == 7 && doc["array"].size() == 5);
	CHECK(doc["array"].back().string() == "tail");
	CHECK(doc["array"].pop_front().isA(json::JSON_NULL));
	CHECK(doc["array"].size() == 4);
	CHECK(doc.exists("new") && !doc.exists("missing"));
	CHECK(doc["new"] == Value(7));

	Document copy(doc);
	CHECK(copy == doc);
	copy["new"] = 8;
	CHECK(copy != doc);
	Document roundTrip;
	CHECK(roundTrip.parse(doc.write(false)) && roundTrip == doc);
	CHECK(!doc.write(true).empty());
	CHECK(Document(Value(std::numeric_limits<double>::quiet_NaN())).write(false) == "null");
	CHECK(Document(Value(std::numeric_limits<double>::infinity())).write(false) == "null");
	Value escaped("quote\" slash\\ control\x01");
	CHECK(Document(escaped).write(false) == "\"quote\\\" slash\\\\ control\\u0001\"");

	for (const std::string permissive : {"", "true false", "01", "1.", "1e+", "[1,]", "{\"x\":1,}"}) {
		Document relaxed;
		CHECK(relaxed.parse(permissive));
	}
	std::string binary = "{\"blob\":\"a";
	binary.push_back('\0');
	binary += "b\\\"c\"}";
	Document binaryDoc;
	CHECK(binaryDoc.parse(binary));
	CHECK(binaryDoc["blob"].string().size() == 5 && binaryDoc["blob"].string()[1] == '\0');
	Document unicode;
	CHECK(unicode.parse(R"("\u0000\u00FF\uD83D\uDE00")"));
	CHECK(unicode.string().size() == 6);
	return true;
}

template<class Value, class Document>
bool data_suite(const char *name)
{
	if (!value_suite<Value, Document>(name)) return false;
	Document doc;
	CHECK(doc.parseXML("<root><item id=\"1\">hello</item><item>world</item></root>"));
	CHECK(doc.isA(json::JSON_OBJECT));
	CHECK(!doc.writeXML("root", false).empty());
	Document roundTrip;
	CHECK(roundTrip.parseXML(doc.writeXML("root", false)));
	return true;
}

}

int main()
{
	bool ok = true;
	ok = ok && value_suite<json::value, json::document>("json");
#if defined SUPPORT_ORDERED_JSON
	ok = ok && value_suite<ojson::value, ojson::document>("ojson");
#endif
#if defined USE_DATA_DOCUMENT
	ok = ok && data_suite<json::value, data::document>("data");
#if defined SUPPORT_ORDERED_JSON
	ok = ok && data_suite<ojson::value, odata::document>("odata");
#endif
#endif
	if (ok) std::cout << "All tests passed.\n";
	return ok ? 0 : 1;
}
