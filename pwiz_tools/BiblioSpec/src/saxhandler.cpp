//
// $Id$
//
//
// Original author: Barbara Frewen <frewen@u.washington.edu>
//
// Copyright 2012 University of Washington - Seattle, WA 98195
//
// Licensed under the Apache License, Version 2.0 (the "License"); 
// you may not use this file except in compliance with the License. 
// You may obtain a copy of the License at 
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software 
// distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and 
// limitations under the License.
//

#include "stdafx.h"
#include "saxhandler.h"
#include <cctype>

namespace BiblioSpec {

// Static callback handlers
static void startElementCallback(void *data, const XML_Char *el, const XML_Char **attr)
{
    ((SAXHandler*) data)->startElement(el, attr);
}

static void endElementCallback(void *data, const XML_Char *el)
{
    ((SAXHandler*) data)->endElement(el);
}

static void charactersCallback(void *data, const XML_Char *s, int len)
{
    ((SAXHandler*) data)->characters(s, len);
}

// Back-compat encoding shim. expat >= 2.6 rejects the non-standard label
// "ASCII" (the registered name is "US-ASCII") that older expat -- and tools such
// as idPicker -- emit in their XML declaration. Without this, the expat
// 2.0.1 -> 2.8.2 upgrade would stop BiblioSpec from reading files it read before
// ("Unknown encoding" error). Map "ASCII" to 7-bit US-ASCII; leave genuinely
// unknown encodings for expat to reject (XML_STATUS_ERROR).
static int unknownEncodingCallback(void* /*data*/, const XML_Char* name, XML_Encoding* info)
{
    static const char asciiName[] = "ASCII";
    if (name == NULL)
        return XML_STATUS_ERROR;
    size_t i = 0;
    for (; asciiName[i] != '\0' && name[i] != '\0'; ++i)
        if (toupper((unsigned char)name[i]) != asciiName[i])
            return XML_STATUS_ERROR;
    if (asciiName[i] != '\0' || name[i] != '\0')   // lengths must match exactly
        return XML_STATUS_ERROR;

    for (int b = 0; b < 128; ++b) info->map[b] = b;      // 7-bit identity
    for (int b = 128; b < 256; ++b) info->map[b] = -1;   // invalid in US-ASCII
    info->data = NULL;
    info->convert = NULL;    // single-byte encoding: map[] is sufficient
    info->release = NULL;
    return XML_STATUS_OK;
}

SAXHandler::SAXHandler()
{
    m_strFileName_.clear();
    m_bytes_ = NULL;
    m_bytesLen_ = 0;
    initParser();
}

SAXHandler::~SAXHandler()
{
    XML_ParserFree(m_parser_);
}

void SAXHandler::initParser()
{
    m_parser_ = XML_ParserCreate(NULL);
    XML_SetUserData(m_parser_, this);
    XML_SetElementHandler(m_parser_, startElementCallback, endElementCallback);
    XML_SetCharacterDataHandler(m_parser_, charactersCallback);
    XML_SetUnknownEncodingHandler(m_parser_, unknownEncodingCallback, NULL);
}

void SAXHandler::startElement(const XML_Char *el, const XML_Char **attr)
{
}

void SAXHandler::endElement(const XML_Char *el)
{
}

void SAXHandler::characters(const XML_Char *s, int len)
{
}

bool SAXHandler::parse()
{
    std::unique_ptr<ifstream> pfIn;
    if (!m_strFileName_.empty()) {
        pfIn.reset(new ifstream(m_strFileName_.c_str(), ios::binary));
        if (!*pfIn) {
            throw BlibException(true, "Failed to open input file '%s'.", 
                                m_strFileName_.c_str());
        }
    }
    else if (m_bytes_ == NULL) {
        throw BlibException(false, "Must set filename or XML data before parsing XML.");
    }
    
    bool success = true;
    string message;
    
    try {
        if (!pfIn) {
            success = (XML_Parse(m_parser_, m_bytes_, m_bytesLen_, true) != 0);
        } else {
            // HACK!! I have no idea why this is, but without this string
            // declaration, the MSVC optimizer removes the catch block below.
            // Very mysterious.
            string temp;

            char buffer[8192];
            int readBytes = 0;

            while (success) {
                pfIn->read(buffer, sizeof(buffer));
                readBytes = pfIn->gcount();
                success = (XML_Parse(m_parser_, buffer, readBytes, false) != 0);
                if (!success || readBytes < sizeof(buffer))
                    break;
            }
            success = success && (XML_Parse(m_parser_, buffer, 0, true) != 0);
        }
    }
    catch (EndEarlyException e) {
        return true;
    }
    catch(string thrown_msg) { // from parsers
        message = thrown_msg;
        success = false;
    }
    catch(BlibException e) { // probably from BuildParser
        if( e.hasFilename() ){
            Verbosity::debug(e.what());
            throw e;
        } else {
            message = e.what();
            success = false;
        }
    }
    catch(std::exception e) {  // other runtime errors (e.g. memory)
        message = e.what();
        success = false;
    }

    if (!success) {
        string error = generateError(message.empty() ? getParserError() : message);
        Verbosity::debug(error.c_str());
        throw BlibException(true, error);
    }
    
    return true;
}

boost::int64_t SAXHandler::getCurrentByteIndex() {
    return XML_GetCurrentByteIndex(m_parser_);
}

string SAXHandler::generateError(const string& message) {
    stringstream ss;
    ss << m_strFileName_
       << "(line " << XML_GetCurrentLineNumber(m_parser_) << "): "
       << message;
   return ss.str();
}

string SAXHandler::getParserError() {
    switch (XML_GetErrorCode(m_parser_)) {
        case XML_ERROR_SYNTAX:                 return "Syntax error parsing XML.";
        case XML_ERROR_INVALID_TOKEN:          return "Invalid token error parsing XML.";
        case XML_ERROR_UNCLOSED_TOKEN:         return "Unclosed token error parsing XML.";
        case XML_ERROR_NO_ELEMENTS:            return "No elements error parsing XML.";
        case XML_ERROR_TAG_MISMATCH:           return "Tag mismatch error parsing XML.";
        case XML_ERROR_DUPLICATE_ATTRIBUTE:    return "Duplicate attribute error parsing XML.";
        case XML_ERROR_UNKNOWN_ENCODING:       return "Unknown encoding XML error.";
        case XML_ERROR_INCORRECT_ENCODING:     return "Incorrect encoding XML error.";
        case XML_ERROR_UNCLOSED_CDATA_SECTION: return "Unclosed data section error parsing XML.";
        default:                               return "XML parsing error.";
    }
}

} // namespace

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * End:
 */
