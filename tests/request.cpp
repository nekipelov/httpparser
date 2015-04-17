#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include <httpparser/request.h>
#include <httpparser/httprequestparser.h>

#include "common.h"

BOOST_AUTO_TEST_SUITE(Simple)

using httpparser::HttpRequestParser;
using httpparser::Request;

struct RequestFixture
{
    Request parse(const std::string &text)
    {
        Request request;
        HttpRequestParser parser;
        
        HttpRequestParser::ParseResult res = parser.parse(request, text.c_str(), text.c_str() + text.size());
        
        if( res != HttpRequestParser::ParsingCompleted )
            return Request();
        else
            return request;
    }
};

BOOST_FIXTURE_TEST_CASE(get_http_09, RequestFixture)
{
    const char *text = "GET /uri\r\n";
    Request result = parse(text);
    Request should = RequestDsl()
        .method("GET")
        .uri("/uri")
        .version(0, 9);
    
    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_FIXTURE_TEST_CASE(get_http_09_uri_with_query_string, RequestFixture)
{
    const char *text = "GET /uri?arg1=test;arg1=%20%21;arg3=test\r\n";
    Request result = parse(text);
    Request should = RequestDsl()
        .method("GET")
        .uri("/uri?arg1=test;arg1=%20%21;arg3=test")
        .version(0, 9);
    
    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_FIXTURE_TEST_CASE(get_http_10, RequestFixture)
{
    const char *text = "GET /uri HTTP/1.0\r\n\r\n";
    Request result = parse(text);
    Request should = RequestDsl()
            .method("GET")
            .uri("/uri")
            .version(1, 0)
            .keepAlive(false);
    
    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_FIXTURE_TEST_CASE(get_http_11, RequestFixture)
{
    const char *text = "GET /uri HTTP/1.1\r\n\r\n";
    Request result = parse(text);
    Request should = RequestDsl()
            .method("GET")
            .uri("/uri")
            .version(1, 1)
            .keepAlive(true);
    
    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_FIXTURE_TEST_CASE(post_http_11, RequestFixture)
{
    const char *text = "POST /uri HTTP/1.1\r\n\r\n";
    Request result = parse(text);
    Request should = RequestDsl()
            .method("POST")
            .uri("/uri")
            .version(1, 1)
            .keepAlive(true);
    
    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_FIXTURE_TEST_CASE(post_http_11_with_header_field, RequestFixture)
{
    const char *text = "POST /uri HTTP/1.1\r\n"
                       "X-Custom-Header: header value\r\n"
                       "\r\n";
    Request result = parse(text);
    Request should = RequestDsl()
            .method("POST")
            .uri("/uri")
            .version(1, 1)
            .header("X-Custom-Header", "header value")
            .keepAlive(true);
    
    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_AUTO_TEST_SUITE_END()
