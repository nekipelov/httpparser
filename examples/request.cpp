#include <iostream>

#include <httpparser/request.h>
#include <httpparser/httprequestparser.h>

using namespace httpparser;

int main(int, char**)
{
    const char text[] = "GET /uri.cgi HTTP/1.1\r\n"
                        "User-Agent: Mozilla/5.0\r\n"
                        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                        "Host: 127.0.0.1\r\n"
                        "\r\n";

    Request request;
    HttpRequestParser parser;

    HttpRequestParser::ParseResult res = parser.parse(request, text, text + sizeof(text));

    if( res == HttpRequestParser::ParsingCompleted )
    {
        std::cout << request.inspect() << std::endl;
        return EXIT_SUCCESS;
    }
    else
    {
        std::cerr << "Parsing failed" << std::endl;
        return EXIT_FAILURE;
    }
}
