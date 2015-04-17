#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include <httpparser/request.h>
#include <httpparser/httprequestparser.h>

#include "common.h"

BOOST_AUTO_TEST_SUITE(Post)

using httpparser::HttpRequestParser;
using httpparser::Request;

struct PostFixture
{
    Request parse(const char *s, size_t size)
    {
        Request request;
        HttpRequestParser parser;
        
        HttpRequestParser::ParseResult res = parser.parse(request, s, s + size);
        
        if( res != HttpRequestParser::ParsingCompleted )
            return Request();
        else
            return request;
    }
};

BOOST_FIXTURE_TEST_CASE(post_with_body, PostFixture)
{
    const char text[] = "POST /uri.cgi HTTP/1.1\r\n"
                        "From: user@example.com\r\n"
                        "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:18.0) Gecko/20100101 Firefox/18.0\r\n"
                        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                        "Accept-Language: en-US,en;q=0.5\r\n"
                        "Accept-Encoding: gzip, deflate\r\n"
                        "Content-Type: application/x-www-form-urlencoded\r\n"
                        "Content-Length: 31\r\n"
                        "Host: 127.0.0.1\r\n"
                        "\r\n"
                        "arg1=test;arg1=%20%21;arg3=test";
                       
    Request result = parse(text, sizeof(text));
    Request should = RequestDsl()
            .method("POST")
            .uri("/uri.cgi")
            .version(1, 1)
            .header("From", "user@example.com")
            .header("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:18.0) Gecko/20100101 Firefox/18.0")
            .header("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8")
            .header("Accept-Language", "en-US,en;q=0.5")
            .header("Accept-Encoding", "gzip, deflate")
            .header("Content-Type", "application/x-www-form-urlencoded")
            .header("Content-Length", "31")
            .header("Host", "127.0.0.1")
            .content("arg1=test;arg1=%20%21;arg3=test")
            .keepAlive(true);
    
    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_FIXTURE_TEST_CASE(post_chunked, PostFixture)
{
    const char text[] = "POST /uri.cgi HTTP/1.1\r\n"
                        "From: user@example.com\r\n"
                        "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:18.0) Gecko/20100101 Firefox/18.0\r\n"
                        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                        "Accept-Language: en-US,en;q=0.5\r\n"
                        "Accept-Encoding: gzip, deflate\r\n"
                        "Content-Type: text/plain\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "Host: 127.0.0.1\r\n"
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

    Request result = parse(text, sizeof(text));
    Request should = RequestDsl()
            .method("POST")
            .uri("/uri.cgi")
            .version(1, 1)
            .header("From", "user@example.com")
            .header("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:18.0) Gecko/20100101 Firefox/18.0")
            .header("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8")
            .header("Accept-Language", "en-US,en;q=0.5")
            .header("Accept-Encoding", "gzip, deflate")
            .header("Content-Type", "text/plain")
            .header("Transfer-Encoding", "chunked")
            .header("Host", "127.0.0.1")
            .content("This is the data in the first chunk"
                     "and this is the second one"
                     "con"
                     "sequence\0")
            .keepAlive(true);

    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_FIXTURE_TEST_CASE(post_chunked_extension, PostFixture)
{
    const char text[] = "POST /uri.cgi HTTP/1.1\r\n"
                        "From: user@example.com\r\n"
                        "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:18.0) Gecko/20100101 Firefox/18.0\r\n"
                        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                        "Accept-Language: en-US,en;q=0.5\r\n"
                        "Accept-Encoding: gzip, deflate\r\n"
                        "Content-Type: text/plain\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "Host: 127.0.0.1\r\n"
                        "\r\n"
                        "23; name=value\r\n"
                        "This is the data in the first chunk\r\n"
                        "1B; secondname=secondvalue\r\n"
                        "and this is the second one\0\r\n"
                        "0\r\n"
                        "Trailer: value\r\n"
                        "\r\n";

    Request result = parse(text, sizeof(text));
    Request should = RequestDsl()
            .method("POST")
            .uri("/uri.cgi")
            .version(1, 1)
            .header("From", "user@example.com")
            .header("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:18.0) Gecko/20100101 Firefox/18.0")
            .header("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8")
            .header("Accept-Language", "en-US,en;q=0.5")
            .header("Accept-Encoding", "gzip, deflate")
            .header("Content-Type", "text/plain")
            .header("Transfer-Encoding", "chunked")
            .header("Host", "127.0.0.1")
            .content("This is the data in the first chunk"
                     "and this is the second one\0")
            .keepAlive(true);

    BOOST_CHECK_EQUAL(result.inspect(), should.inspect());
}

BOOST_AUTO_TEST_SUITE_END()
