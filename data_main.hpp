/*
Copyright (c) 2012-2024 James Baker

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


#include "SDString/sdstring.hpp"

#if !defined JSON_NAMESPACE
#	pragma message("What the heck?")
#endif

class TiXmlNode;

namespace DATA_NAMESPACE
{
	sdstring XMLEscape(const sdstring & in, bool bAttribute = false);

	class document : public JSON_NAMESPACE::document
	{
		public:
			using JSON_NAMESPACE::document::document;
			using JSON_NAMESPACE::document::value;

			document() : JSON_NAMESPACE::document()
			{
				bForceXMLHeader = false;
				bNoXMLHeader	= false;
				bStandAlone		= true;
			}


			document(const JSON_NAMESPACE::value & V) : JSON_NAMESPACE::document(V)
			{
				bForceXMLHeader = false;
				bNoXMLHeader	= false;
				bStandAlone		= true;
			}
#if defined SUPPORT_ORDERED_JSON && !defined DO_ODATA_STUFF
			document(const ojson::value & V) : json::document(V)
			{
				bForceXMLHeader = false;
				bNoXMLHeader	= false;
				bStandAlone		= true;
			}
#elif defined DO_ODATA_STUFF
			document(const json::value & V) : ojson::document(V)
			{
				bForceXMLHeader = false;
				bNoXMLHeader	= false;
				bStandAlone		= true;
			}
#endif
			void standAlone(bool bSetTo)
			{
				bStandAlone = bSetTo;
			}

			bool standAlone()
			{
				return bStandAlone;
			}

			typedef sdstring & (*PREPARSEPTR)(const sdstring & in, sdstring & out, sdstring fileName);
			typedef sdstring & (*PREWRITEPTR)(const sdstring & in, sdstring & out);

			void forceXMLHeader(bool bSetTo)
			{
				bForceXMLHeader = bSetTo;
			}

			bool forceXMLHeader()
			{
				return bForceXMLHeader;
			}

			void noXMLHeader(bool bSetTo)
			{
				bNoXMLHeader = bSetTo;
			}

			bool noXMLHeader()
			{
				return bNoXMLHeader;
			}

			bool parseXML_old(const sdstring & inStr, PREPARSEPTR = nullptr, const sdstring & preParseFileName = "");
			bool parseXML(const sdstring & inStr, PREPARSEPTR = nullptr, const sdstring & preParseFileName = "");

			bool parseXMLFile_old(const sdstring & inStr, PREPARSEPTR = nullptr, bool bReWriteFile = false);
			bool parseXMLFile(const sdstring & inStr, PREPARSEPTR = nullptr, bool bReWriteFile = false);

			sdstring writeXML(const sdstring & rootElem, bool bPretty = true, bool bTabs = true, PREWRITEPTR = nullptr);
			sdstring writeXML(bool bPretty = true, bool bTabs = true, PREWRITEPTR = nullptr);

			bool writeXMLFile(const sdstring & inStr, const sdstring & rootElem, bool bPretty = true, bool bTabs = true, PREWRITEPTR = nullptr);
			bool writeXMLFile(const sdstring & inStr, bool bPretty = true, bool bTabs = true, PREWRITEPTR = nullptr);

			sdstring rootTag()
			{
				return sRootTag;
			}

			void rootTag(const sdstring & rootElem)
			{
				sRootTag = rootElem;
			}

			static void stripNameSpaces(JSON_NAMESPACE::value & jDoc);
			static void stripNameSpaces(JSON_NAMESPACE::value & jDoc, JSON_NAMESPACE::document & jNameSpaces, bool begin = true);
			static void stripNameSpace(JSON_NAMESPACE::value & jDoc, sdstring sNameSpace, bool begin = true);
			static void addNameSpace(JSON_NAMESPACE::value & jDoc, sdstring sNameSpace, bool begin = true);

			void stripMyNameSpaces();
			void stripMyNameSpaces(JSON_NAMESPACE::document & jNameSpaces);
			void stripMyNameSpace(sdstring sNameSpace);
			void addMyNameSpace(sdstring sNameSpace);

		private:
			bool fastParse(JSON_NAMESPACE::instring & in, JSON_NAMESPACE::value & out, sdstring & parseResult);
			void parseXMLElement(JSON_NAMESPACE::value & ret, const TiXmlNode * elem);
			static void writeXML(sdstring & str, JSON_NAMESPACE::value & ret, size_t depth, bool bPretty = true, bool bTabs = true);

			sdstring sRootTag;
			bool bForceXMLHeader;
			bool bNoXMLHeader;
			bool bStandAlone;
	};
#if !defined DO_ODATA_STUFF
#	define DATA_ATT(x)			 sdstring("@") + x
#	define DATA_VAL			 "#value"
#	define DATAENUMKEY(x, y)	 json::enumKey<x>(y, #y)
#	define DATAATTENUMKEY(x, y) json::enumKey<x>(y, "@" #y)
#endif
}	 // namespace DATA_NAMESPACE

// #endif //_DATA_HPP
