#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include <httpparser/response.h>
#include <httpparser/httpresponseparser.h>

#include "common.h"

BOOST_AUTO_TEST_SUITE(Simple)

using httpparser::HttpResponseParser;
using httpparser::Response;

struct ResponseFixture
{
    Response parse(const char *s, size_t size)
    {
        Response response;
        HttpResponseParser parser;
        
        HttpResponseParser::ParseResult res = parser.parse(response, s, s + size);

        if( res != HttpResponseParser::ParsingCompleted )
        {
            std::cerr << "Incompleted: " << res  << std::endl;
            return Response();
        }
        else
            return response;
    }
};

BOOST_FIXTURE_TEST_CASE(get_http_09_uri_with_query_string, ResponseFixture)
{
    const char text[] =
            "HTTP/1.1 200 OK\r\n"
            "Server: nginx/1.2.1\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 8\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            "<html />";

    Response result = parse(text, sizeof(text));
    Response should = ResponseDsl()
        .statusCode(200)
        .status("OK")
        .version(1, 1)
        .header("Server", "nginx/1.2.1")
        .header("Content-Type", "text/html")
        .header("Content-Length", "8")
        .header("Connection", "keep-alive")
        .body("<html />");

    
    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_FIXTURE_TEST_CASE(chunkedEncoding, ResponseFixture)
{
    const char text[] =
            "HTTP/1.1 200 OK\r\n"
            "Server: nginx/1.2.1\r\n"
            "Content-Type: text/html\r\n"
            "Connection: keep-alive\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "23\r\n"
            "This is the data in the first chunk\r\n"
            "1A\r\n"
            "and this is the second one\r\n"
            "3\r\n"
            "con\r\n"
            "9\r\n"
            "sequence\0\r\n"
            "0\r\n\r\n";

    Response result = parse(text, sizeof(text));
    Response should = ResponseDsl()
        .statusCode(200)
        .status("OK")
        .version(1, 1)
        .header("Server", "nginx/1.2.1")
        .header("Content-Type", "text/html")
        .header("Connection", "keep-alive")
        .header("Transfer-Encoding", "chunked")
        .body("This is the data in the first chunk"
              "and this is the second one"
              "con"
              "sequence\0");

    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_FIXTURE_TEST_CASE(chunk_extension, ResponseFixture)
{
    const char text[] =
            "HTTP/1.1 200 OK\r\n"
            "Server: nginx/1.2.1\r\n"
            "Content-Type: text/html\r\n"
            "Connection: keep-alive\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "23; name=value\r\n"
            "This is the data in the first chunk\r\n"
            "1B; secondname=secondvalue\r\n"
            "and this is the second one\0\r\n"
            "0\r\n"
            "Trailer: value\r\n"
            "\r\n";

    Response result = parse(text, sizeof(text));
    Response should = ResponseDsl()
        .statusCode(200)
        .status("OK")
        .version(1, 1)
        .header("Server", "nginx/1.2.1")
        .header("Content-Type", "text/html")
        .header("Connection", "keep-alive")
        .header("Transfer-Encoding", "chunked")
        .body("This is the data in the first chunk"
              "and this is the second one\0");

    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_AUTO_TEST_SUITE_END()
