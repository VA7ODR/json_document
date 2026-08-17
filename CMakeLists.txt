# Copyright (c) 2012-2026 James Baker
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# The official repository for this library is at https://github.com/VA7ODR/json

cmake_minimum_required(VERSION 3.5)

#cmake_policy(SET CMP0077 NEW)							#silence CMAKE warning:
##For compatibility with older versions of CMake, option is clearing the normal variable 'JSON_STAND_ALONE'.

project(json_document)

set (CMAKE_CXX_STANDARD 23)

option(JSON_ORDERED "Is the ordered json (ojson::document) class required" ON)
option(JSON_DATA_DOCUMENT "Is the xml functionality (data/odata::document) class(es) required" ON)
option(JSON_SAMPLES "Create sample programs" OFF)
option(JSON_TEST_EXAMPLE "Creates a sample used for testing features." OFF)
option(JSON_USE_TEMP_FILES "Makes json::document and derived classes use temp files for data protection." OFF)
option(JSON_RESTORE_TEMP_FILES "Makes json::document and derived classes automatically restore from temp files if they exist." OFF)
option(ADDRESSSANITIZER "Use AddressSanitizer" OFF)

if (ADDRESSSANITIZER)
         set(ADDRESSSANITIZERFLAGS " -fsanitize=address -fno-omit-frame-pointer ")
endif(ADDRESSSANITIZER)

set(CMAKE_CXX_FLAGS                " ${CMAKE_CXX_FLAGS_INIT} ${ADDRESSSANITIZERFLAGS} -fPIC -rdynamic ")

if(JSON_USE_TEMP_FILES)
    if (JSON_RESTORE_TEMP_FILES)
        set(JSON_TEMP JSON_TEMP_FILES_ JSON_RESTORE_TEMP_FILES_)
    else (JSON_RESTORE_TEMP_FILES)
        set(JSON_TEMP JSON_TEMP_FILES_)
    endif (JSON_RESTORE_TEMP_FILES)
endif(JSON_USE_TEMP_FILES)

if (JSON_ORDERED)
    set (JSON_TARGET_DEFINES PUBLIC SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
    add_library(ojson STATIC json.cpp json.hpp json_main.hpp SDString/sdstring.hpp ArbitraryOrderMap/arbitrary_order_map.hpp)
    target_compile_definitions(ojson PUBLIC ${JSON_TARGET_DEFINES} JSON_USE_ADDED_ORDER)
    SET(OJSON_OBJECTS $<TARGET_OBJECTS:ojson>)
else (JSON_ORDERED)
    set (JSON_TARGET_DEFINES PUBLIC _CRT_SECURE_NO_WARNINGS)
endif (JSON_ORDERED)

if (JSON_DATA_DOCUMENT)
    set(DATA_DOCUMENT_DEFINES USE_DATA_DOCUMENT)
endif(JSON_DATA_DOCUMENT)

add_library(ajson STATIC json.cpp json.hpp json_main.hpp SDString/sdstring.hpp)
target_compile_definitions(ajson PUBLIC ${JSON_TARGET_DEFINES})

if(JSON_DATA_DOCUMENT)
    add_library(tinyxml STATIC tinyxml/tinystr.cpp tinyxml/tinyxml.cpp tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp)
    if(JSON_ORDERED)
        add_library(odata STATIC data.cpp data.hpp data_main.hpp SDString/sdstring.hpp ArbitraryOrderMap/arbitrary_order_map.hpp)
        target_compile_definitions(odata PUBLIC ${JSON_TARGET_DEFINES} JSON_USE_ADDED_ORDER)
        SET(ODATA_OBJECTS $<TARGET_OBJECTS:odata>)
    endif(JSON_ORDERED)
    add_library(adata STATIC data.cpp data.hpp data_main.hpp SDString/sdstring.hpp)
    target_compile_definitions(adata PUBLIC ${JSON_TARGET_DEFINES})
    SET(ADATA_OBJECTS $<TARGET_OBJECTS:adata> $<TARGET_OBJECTS:tinyxml>)
endif(JSON_DATA_DOCUMENT)

add_library(${PROJECT_NAME} STATIC $<TARGET_OBJECTS:ajson> ${OJSON_OBJECTS} ${ADATA_OBJECTS} ${ODATA_OBJECTS})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${JSON_TARGET_DEFINES})

if(JSON_TEST_EXAMPLE)
    add_executable(testbed test.cpp)
    target_compile_definitions(testbed PUBLIC ${JSON_TARGET_DEFINES} ${DATA_DOCUMENT_DEFINES})
    target_link_libraries(testbed PUBLIC ${PROJECT_NAME})
endif(JSON_TEST_EXAMPLE)

if(JSON_SAMPLES)
    if(JSON_ORDERED)
        add_executable(csv2json csv2json.cpp)
        target_compile_definitions(csv2json PUBLIC ${JSON_TARGET_DEFINES})
        target_link_libraries(csv2json PUBLIC ${PROJECT_NAME})

        add_executable(jsonpretty json2xml.cpp)
        target_compile_definitions(jsonpretty PUBLIC ${JSON_TARGET_DEFINES} ${DATA_DOCUMENT_DEFINES})
        target_link_libraries(jsonpretty PUBLIC ${PROJECT_NAME})

        add_executable(json2csv json2csv.cpp)
        target_compile_definitions(json2csv PUBLIC __STANDALONE_CSV__ ${JSON_TARGET_DEFINES})
        target_link_libraries(json2csv PUBLIC ${PROJECT_NAME})

        install(TARGETS jsonpretty json2csv csv2json RUNTIME DESTINATION bin)
    endif(JSON_ORDERED)

endif(JSON_SAMPLES)

if(JSON_DATA_DOCUMENT)
    if(JSON_SAMPLES)
        add_executable(json2xml json2xml.cpp)
        target_compile_definitions(json2xml PUBLIC ${JSON_TARGET_DEFINES} ${DATA_DOCUMENT_DEFINES})
        target_link_libraries(json2xml PUBLIC ${PROJECT_NAME})

        add_executable(xml2json json2xml.cpp)
        target_compile_definitions(xml2json PUBLIC ${JSON_TARGET_DEFINES} ${DATA_DOCUMENT_DEFINES})
        target_link_libraries(xml2json PUBLIC ${PROJECT_NAME})

        add_executable(xmlpretty json2xml.cpp)
        target_compile_definitions(xmlpretty PUBLIC ${JSON_TARGET_DEFINES} ${DATA_DOCUMENT_DEFINES})
        target_link_libraries(xmlpretty PUBLIC ${PROJECT_NAME})

        add_executable(csv2xml csv2json.cpp)
        target_compile_definitions(csv2xml PUBLIC ${JSON_TARGET_DEFINES})
        target_link_libraries(csv2xml PUBLIC ${PROJECT_NAME})

        install(TARGETS json2xml xml2json xmlpretty csv2xml RUNTIME DESTINATION bin)
    endif(JSON_SAMPLES)

    install(FILES data.hpp DESTINATION include/${PROJECT_NAME})
endif(JSON_DATA_DOCUMENT)

install(FILES json.hpp DESTINATION include/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib)
