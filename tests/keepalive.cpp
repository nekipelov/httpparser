#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include <httpparser/request.h>
#include <httpparser/httprequestparser.h>

#include "common.h"

BOOST_AUTO_TEST_SUITE(Keepalive)

using httpparser::HttpRequestParser;
using httpparser::Request;

struct KeepaliveFixture
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

BOOST_FIXTURE_TEST_CASE(http_10_connection_default, KeepaliveFixture)
{
    const char *text = 
        "GET /uri HTTP/1.0\r\n"
        "\r\n";

    Request result = parse(text);
    Request should = RequestDsl()
            .method("GET")
            .uri("/uri")
            .version(1, 0)
            .keepAlive(false);
    
    result.headers.clear();
    
    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_FIXTURE_TEST_CASE(http_11_connection_default, KeepaliveFixture)
{
    const char *text = 
        "GET /uri HTTP/1.1\r\n"
        "\r\n";

    Request result = parse(text);
    Request should = RequestDsl()
            .method("GET")
            .uri("/uri")
            .version(1, 1)
            .keepAlive(true);
    
    result.headers.clear();
    
    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}


BOOST_FIXTURE_TEST_CASE(http_10_connection_keepalive, KeepaliveFixture)
{
    const char *text = 
        "GET /uri HTTP/1.0\r\n"
        "Connection: Keep-Alive\r\n"
        "\r\n";

    Request result = parse(text);
    Request should = RequestDsl()
            .method("GET")
            .uri("/uri")
            .version(1, 0)
            .keepAlive(true);
    
    result.headers.clear();
    
    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_FIXTURE_TEST_CASE(http_11_connection_close, KeepaliveFixture)
{
    const char *text = 
        "GET /uri HTTP/1.1\r\n"
        "Connection: close\r\n"
        "\r\n";

    Request result = parse(text);
    Request should = RequestDsl()
            .method("GET")
            .uri("/uri")
            .version(1, 1)
            .keepAlive(false);
            
    result.headers.clear();
    
    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_AUTO_TEST_SUITE_END()
