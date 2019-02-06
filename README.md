httpparser
==========

Simple and fast HTTP request, response and urls parser written in C++. 


Examples
-----

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
    
        HttpRequestParser::ParseResult res = parser.parse(request, text, text + strlen(text));
    
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


    #include <iostream>
    
    #include <httpparser/response.h>
    #include <httpparser/httpresponseparser.h>
    
    using namespace httpparser;
    
    int main(int, char**)
    {
        const char text[] =
                "HTTP/1.1 200 OK\r\n"
                "Server: nginx/1.2.1\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 8\r\n"
                "Connection: keep-alive\r\n"
                "\r\n"
                "<html />";
    
        Response response;
        HttpResponseParser parser;
    
        HttpResponseParser::ParseResult res = parser.parse(response, text, text + strlen(text));
    
        if( res == HttpResponseParser::ParsingCompleted )
        {
            std::cout << response.inspect() << std::endl;
            return EXIT_SUCCESS;
        }
        else
        {
            std::cerr << "Parsing failed" << std::endl;
            return EXIT_FAILURE;
        }
    }


    #include <iostream>
    #include <httpparser/urlparser.h>
    
    using namespace httpparser;
    
    int main(int, char**)
    {
        UrlParser parser;
    
        {
            const char url[] = "git+ssh://example.com/path/file";
    
            if( parser.parse(url) )
                std::cout << parser.scheme() << "://" << parser.hostname() << std::endl;
            else
                std::cerr << "Can't parse url: " << url << std::endl;
        }
    
        {
            const char url[] = "https://example.com/path/file";
    
            if( parser.parse(url) )
                std::cout << parser.scheme() << "://" << parser.hostname() << std::endl;
            else
                std::cerr << "Can't parse url: " << url << std::endl;
        }
    
        return EXIT_SUCCESS;
    }
