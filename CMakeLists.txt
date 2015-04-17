PROJECT(httparser)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

FIND_PACKAGE(Boost COMPONENTS unit_test_framework system REQUIRED)

SET(CMAKE_CXX_FLAGS "-Wextra -Wall -pthread" )
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" )
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -g" )

SET(HEADERS
    src/httpparser/httprequestparser.h
    src/httpparser/httpresponseparser.h
    src/httpparser/request.h
    src/httpparser/response.h
    src/httpparser/urlparser.h
    tests/common.h
)

SET(SOURCES
)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

ADD_EXECUTABLE(keepalive tests/keepalive.cpp  ${HEADERS})
TARGET_LINK_LIBRARIES(keepalive ${Boost_LIBRARIES})
ADD_TEST(keepalive keepalive)

ADD_EXECUTABLE(post tests/post.cpp ${HEADERS})
TARGET_LINK_LIBRARIES(post ${Boost_LIBRARIES})
ADD_TEST(post post)

ADD_EXECUTABLE(response tests/response.cpp ${HEADERS})
TARGET_LINK_LIBRARIES(response ${Boost_LIBRARIES})
ADD_TEST(response response)

ADD_EXECUTABLE(request tests/request.cpp  ${HEADERS})
TARGET_LINK_LIBRARIES(request ${Boost_LIBRARIES})
ADD_TEST(request response)

ADD_EXECUTABLE(urlparser tests/urlparser.cpp ${HEADERS})
TARGET_LINK_LIBRARIES(urlparser ${Boost_LIBRARIES})
ADD_TEST(urlparser urlparser)

ENABLE_TESTING()

INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/src/httpparser DESTINATION include)
