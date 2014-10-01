#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_WeatherService

#include <boost/test/unit_test.hpp>

#include <httpparser/request.h>
#include <httpparser/httprequestparser.h>

#include "common.h"

BOOST_AUTO_TEST_SUITE(Simple)

struct SimpleFixture
{
    Request parse(const std::string &text)
    {
        Request request;
        HttpRequestParser parser;
        
        HttpRequestParser::ParseResult res = parser.parse(request, text.c_str(), text.c_str() + text.size());
        
        if( res != HttpRequestParser::CompletedResult )
            return Request();
        else
            return request;
    }
};

BOOST_FIXTURE_TEST_CASE(get_http_09, SimpleFixture)
{
    const char *text = "GET /uri\r\n";
    Request result = parse(text);
    Request should = RequestDsl()
        .method("GET")
        .uri("/uri")
        .version(0, 9);
    
    BOOST_CHECK_EQUAL(result, should);
}

BOOST_FIXTURE_TEST_CASE(get_http_09_uri_with_query_string, SimpleFixture)
{
    const char *text = "GET /uri?arg1=test;arg1=%20%21;arg3=test\r\n";
    Request result = parse(text);
    Request should = RequestDsl()
        .method("GET")
        .uri("/uri?arg1=test;arg1=%20%21;arg3=test")
        .version(0, 9);
    
    BOOST_CHECK_EQUAL(result, should);
}

BOOST_FIXTURE_TEST_CASE(get_http_10, SimpleFixture)
{
    const char *text = "GET /uri HTTP/1.0\r\n\r\n";
    Request result = parse(text);
    Request should = RequestDsl()
            .method("GET")
            .uri("/uri")
            .version(1, 0)
            .keepAlive(false);
    
    BOOST_CHECK_EQUAL(result, should);
}

BOOST_FIXTURE_TEST_CASE(get_http_11, SimpleFixture)
{
    const char *text = "GET /uri HTTP/1.1\r\n\r\n";
    Request result = parse(text);
    Request should = RequestDsl()
            .method("GET")
            .uri("/uri")
            .version(1, 1)
            .keepAlive(true);
    
    BOOST_CHECK_EQUAL(result, should);
}

BOOST_FIXTURE_TEST_CASE(post_http_11, SimpleFixture)
{
    const char *text = "POST /uri HTTP/1.1\r\n\r\n";
    Request result = parse(text);
    Request should = RequestDsl()
            .method("POST")
            .uri("/uri")
            .version(1, 1)
            .keepAlive(true);
    
    BOOST_CHECK_EQUAL(result, should);
}

BOOST_FIXTURE_TEST_CASE(post_http_11_with_header_field, SimpleFixture)
{
    const char *text = "POST /uri HTTP/1.1\r\n"
                       "X-Custom-Header: header value\r\n"
                       "\r\n";
    Request result = parse(text);
    Request should = RequestDsl()
            .method("POST")
            .uri("/uri")
            .version(1, 1)
            .header("x-custom-header", "header value")
            .keepAlive(true);
    
    BOOST_CHECK_EQUAL(result, should);
}

BOOST_AUTO_TEST_SUITE_END()
