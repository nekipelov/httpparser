/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: MIT
 */

#ifndef LIBAHTTP_REQUESTPARSER_H
#define LIBAHTTP_REQUESTPARSER_H

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility/enable_if.hpp>

#include "request.h"
#include "response.h"

namespace impl 
{
    // Check if a byte is an HTTP character.
    inline bool isChar(int c)
    {
        return c >= 0 && c <= 127;
    }

    // Check if a byte is an HTTP control character.
    inline bool isControl(int c)
    {
        return (c >= 0 && c <= 31) || (c == 127);
    }

    // Check if a byte is defined as an HTTP special character.
    inline bool isSpecial(int c)
    {
        switch (c)
        {
        case '(': case ')': case '<': case '>': case '@':
        case ',': case ';': case ':': case '\\': case '"':
        case '/': case '[': case ']': case '?': case '=':
        case '{': case '}': case ' ': case '\t':
            return true;
        default:
            return false;
        }
    }

    // Check if a byte is a digit.
    inline bool isDigit(int c)
    {
        return c >= '0' && c <= '9';
    }
    
    // Parser for incoming requests.
    class HttpRequestParser
    {
    public:
        HttpRequestParser()
            : state(RequestMethodStart)
        {
        }
        
        enum ParserResult {
            CompletedResult,
            IncompletedResult,
            ErrorResult
        };
    
        enum State {
            RequestMethodStart,
            RequestMethod,
            RequestUriStart,
            RequestUri,
            RequestHttpVersion_h,
            RequestHttpVersion_ht,
            RequestHttpVersion_htt,
            RequestHttpVersion_http,
            RequestHttpVersion_slash,
            RequestHttpVersion_majorStart,
            RequestHttpVersion_major,
            RequestHttpVersion_minorStart,
            RequestHttpVersion_minor,
            ExpectingNewline
        } state;
        
        std::pair<size_t, ParserResult> consume(Request &req, const char *ptr, const char *end)
        {
            const char *begin = begin;
            while( ptr != end )
            {
                char input = *ptr++;

                switch (state)
                {
                case RequestMethodStart:
                    if (!isChar(input) || isControl(input) || isSpecial(input))
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                    else
                    {
                        state = RequestMethod;
                        req.method.push_back(input);
                        continue;
                    }
                case RequestMethod:
                    if (input == ' ')
                    {
                        state = RequestUriStart;
                        continue;
                    }
                    else if (!isChar(input) || isControl(input) || isSpecial(input))
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                    else
                    {
                        req.method.push_back(input);
                        continue;
                    }
                case RequestUriStart:
                    if (isControl(input))
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                    else
                    {
                        state = RequestUri;
                        req.uri.push_back(input);
                        continue;
                    }
                case RequestUri:
                    if (input == ' ')
                    {
                        state = RequestHttpVersion_h;
                        continue;
                    }
                    else if (input == '\r')
                    {
                        req.versionMajor = 0;
                        req.versionMinor = 9;

                        state = ExpectingNewline;
                        continue;
                    }
                    else if (isControl(input))
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                    else
                    {
                        req.uri.push_back(input);
                        continue;
                    }
                case RequestHttpVersion_h:
                    if (input == 'H')
                    {
                        state = RequestHttpVersion_ht;
                        continue;
                    }
                    else
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                case RequestHttpVersion_ht:
                    if (input == 'T')
                    {
                        state = RequestHttpVersion_htt;
                        continue;
                    }
                    else
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                case RequestHttpVersion_htt:
                    if (input == 'T')
                    {
                        state = RequestHttpVersion_http;
                        continue;
                    }
                    else
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                case RequestHttpVersion_http:
                    if (input == 'P')
                    {
                        state = RequestHttpVersion_slash;
                        continue;
                    }
                    else
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                case RequestHttpVersion_slash:
                    if (input == '/')
                    {
                        req.versionMajor = 0;
                        req.versionMinor = 0;
                        state = RequestHttpVersion_majorStart;
                        continue;
                    }
                    else
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                case RequestHttpVersion_majorStart:
                    if (isDigit(input))
                    {
                        req.versionMajor = input - '0';
                        state = RequestHttpVersion_major;
                        continue;
                    }
                    else
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                case RequestHttpVersion_major:
                    if (input == '.')
                    {
                        state = RequestHttpVersion_minorStart;
                        continue;
                    }
                    else if (isDigit(input))
                    {
                        req.versionMajor = req.versionMajor * 10 + input - '0';
                        continue;
                    }
                    else
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                case RequestHttpVersion_minorStart:
                    if (isDigit(input))
                    {
                        req.versionMinor = input - '0';
                        state = RequestHttpVersion_minor;
                        continue;
                    }
                    else
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                case RequestHttpVersion_minor:
                    if (input == '\r')
                    {
                        state = ExpectingNewline;
                        continue;
                    }
                    else if (isDigit(input))
                    {
                        req.versionMinor = req.versionMinor * 10 + input - '0';
                        continue;
                    }
                    else
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                case ExpectingNewline:
                    if (input == '\n')
                    {
                        return std::make_pair(ptr - begin, CompletedResult);
                        continue;
                    }
                    else
                    {
                        return std::make_pair(ptr - begin, ErrorResult);
                    }
                default:
                    return std::make_pair(ptr - begin, ErrorResult);
                }
            }

            return std::make_pair(ptr - begin, IncompletedResult);
        }        
    };
    
    // Parser for responses.
    class HttpReponseParser 
    {
    public:
        HttpReponseParser()
            : state(ResponseStatusStart)
        {
        }
        
        enum ParserResult {
            CompletedResult,
            IncompletedResult,
            ErrorResult
        };
    
        enum State {
            ResponseStatusStart,
            ResponseHttpVersion_ht,
            ResponseHttpVersion_htt,
            ResponseHttpVersion_http,
            ResponseHttpVersion_slash,
            ResponseHttpVersion_majorStart,
            ResponseHttpVersion_major,
            ResponseHttpVersion_minorStart,
            ResponseHttpVersion_minor,
            ResponseHttpVersion_statusCodeStart,
            ResponseHttpVersion_statusCode,
            ResponseHttpVersion_statusTextStart,
            ResponseHttpVersion_statusText,
            ExpectingNewline
        } state;
        
        std::pair<size_t, ParserResult> consume(Response &req, const char *begin, const char *end)
        {
            /*
            while( begin != end )
            {
                char input = *begin++;

                switch (state)
                {
                case ResponseStatusStart:
                    if (input != 'H' )
                    {
                        return ErrorResult;
                    }
                    else
                    {
                        state = ResponseHttpVersion_ht;
                        continue;
                    }
                case ResponseHttpVersion_ht:
                    if (input == 'T' )
                    {
                        state = ResponseHttpVersion_htt;
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }
                case ResponseHttpVersion_htt:
                    if (input == 'T' )
                    {
                        state = ResponseHttpVersion_http;
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }                    
                case ResponseHttpVersion_http:
                    if (input == 'T' )
                    {
                        state = ResponseHttpVersion_slash;
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }                   
                case ResponseHttpVersion_slash:
                    if (input == '/')
                    {
                        req.versionMajor = 0;
                        req.versionMinor = 0;
                        state = ResponseHttpVersion_majorStart;
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }
                case ResponseHttpVersion_majorStart:
                    if (isDigit(input))
                    {
                        req.versionMajor = input - '0';
                        state = ResponseHttpVersion_major;
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }
                case ResponseHttpVersion_major:
                    if (input == '.')
                    {
                        state = ResponseHttpVersion_minorStart;
                        continue;
                    }
                    else if (isDigit(input))
                    {
                        req.versionMajor = req.versionMajor * 10 + input - '0';
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }
                case ResponseHttpVersion_minorStart:
                    if (isDigit(input))
                    {
                        req.versionMinor = input - '0';
                        state = ResponseHttpVersion_minor;
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }
                case ResponseHttpVersion_minor:
                    if (input == ' ')
                    {
                        state = ResponseHttpVersion_statusCodeStart;
                        req.statusCode = 0;
                        continue;
                    }
                    else if (isDigit(input))
                    {
                        req.versionMinor = req.versionMinor * 10 + input - '0';
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }
                case ResponseHttpVersion_statusCodeStart:
                    if (isDigit(input))
                    {
                        req.statusCode = input - '0';
                        state = ResponseHttpVersion_statusCode;
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }
                case ResponseHttpVersion_statusCode:
                    if (isDigit(input))
                    {
                        req.statusCode = req.statusCode * 10 + input - '0';
                        continue;
                    }
                    else
                    {
                        if( req.statusCode < 100 || req.statusCode > 999 )
                        {
                            return ErrorResult;
                        }
                        else if( input == ' ' )
                        {
                            state = ResponseHttpVersion_statusTextStart;
                            continue;
                        }
                        else
                        {
                            return ErrorResult;
                        }
                    }
                case ResponseHttpVersion_statusTextStart:
                    if( isChar(input) )
                    {
                        req.status += input;
                        state = ResponseHttpVersion_statusText;
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }
                case ResponseHttpVersion_statusText:
                    if( input == '\r' )
                    {
                        state = ExpectingNewline;
                        continue;
                    }
                    else if( isChar(input) )
                    {
                        req.status += input;
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }
                case ExpectingNewline:
                    if( input == '\n' )
                    {
                        return CompletedResult;
                    }
                    else
                    {
                        return ErrorResult;
                    }
                default:
                    return ErrorResult;
                }
            }

            return IncompletedResult;
            */
        }                        
    };
    
    template<typename TStorage, typename TParser>
    class HttpParserImpl
    {
    public:
        HttpParserImpl()
            : state(Head),
              postSize(0)
        {
        }

        enum ParserResult {
            CompletedResult,
            IncompletedResult,
            ErrorResult
        };
        
        ParserResult parse(TStorage &data, const char *begin, const char *end)
        {
            ParserResult result;
            parse(data, result, begin, end);
            return result;
        }

    private:   
        template<typename U>
        typename boost::enable_if< boost::is_same<U, Request> >::type 
        parse(U &req, ParserResult &result, const char *begin, const char *end)
        {
            if( state == Head )
            {
                std::pair<size_t, HttpRequestParser::ParserResult> res =
                        headParser.consume(req, begin, end);
                
                switch(res.second)
                {
                case HttpRequestParser::CompletedResult:
                    assert(begin + res.first <= end);
                    begin += res.first;
                    state = HeaderLineStart;
                    break;
                case HttpRequestParser::IncompletedResult:
                    result = IncompletedResult;
                    return;
                case HttpRequestParser::ErrorResult:
                    result = ErrorResult;
                    return;
                }
            }

            result = consume(req, begin, end);
            //return consume(req, begin, end);
        }

        template<typename U>
        typename boost::enable_if< boost::is_same<U, Response> >::type 
        parse(U &resp, ParserResult &result, const char *begin, const char *end)
        {
            std::cerr << "response!" << std::endl;
            result = ErrorResult;
            //return consume(req, begin, end);
        }
        
        
        //template< typename = typename boost::enable_if< boost::is_same<T, Response>::value >::type >
        /*template<typename T, typename boost::enable_if< boost::is_same<T, Response>, void >::type >
        ParserResult parse(T &resp, const char *begin, const char *end)
        {
            std::cerr << "response!" << std::endl;
            //return consume(req, begin, end);
        }
        */
    private:
        static bool checkIfConnection(const Request::HeaderItem &item)
        {
            return strcasecmp(item.name.c_str(), "Connection") == 0;
        }

        ParseState consume(TStorage &data, const char *begin, const char *end)
        {
            while( begin != end )
            {
                char input = *begin++;

                switch (state)
                {
                case ExpectingNewline_1:
                    if (input == '\n')
                    {
                        state = HeaderLineStart;
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }
                case HeaderLineStart:
                    if (input == '\r')
                    {
                        state = ExpectingNewline_3;
                        continue;
                    }
                    else if (!req.headers.empty() && (input == ' ' || input == '\t'))
                    {
                        state = HeaderLws;
                        continue;
                    }
                    else if (!isChar(input) || isControl(input) || isSpecial(input))
                    {
                        return ErrorResult;
                    }
                    else
                    {
                        req.headers.push_back(Request::HeaderItem());
                        req.headers.back().name.reserve(16);
                        req.headers.back().value.reserve(16);
                        req.headers.back().name.push_back(input);
                        state = HeaderName;
                        continue;
                    }
                case HeaderLws:
                    if (input == '\r')
                    {
                        state = ExpectingNewline_2;
                        continue;
                    }
                    else if (input == ' ' || input == '\t')
                    {
                        continue;
                    }
                    else if (isControl(input))
                    {
                        return ErrorResult;
                    }
                    else
                    {
                        state = HeaderValue;
                        req.headers.back().value.push_back(input);
                        continue;
                    }
                case HeaderName:
                    if (input == ':')
                    {
                        state = SpaceBeforeHeaderValue;
                        continue;
                    }
                    else if (!isChar(input) || isControl(input) || isSpecial(input))
                    {
                        return ErrorResult;
                    }
                    else
                    {
                        req.headers.back().name.push_back(input);
                        continue;
                    }
                case SpaceBeforeHeaderValue:
                    if (input == ' ')
                    {
                        state = HeaderValue;
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }
                case HeaderValue:
                    if (input == '\r')
                    {
                        if( req.method == "POST" )
                        {
                            Request::HeaderItem &h = req.headers.back();

                            if( strcasecmp(h.name.c_str(), "Content-Length") == 0 )
                            {
                                postSize = atoi(h.value.c_str());
                                req.postData.reserve( postSize );
                            }
                        }
                        state = ExpectingNewline_2;
                        continue;
                    }
                    else if (isControl(input))
                    {
                        return ErrorResult;
                    }
                    else
                    {
                        req.headers.back().value.push_back(input);
                        continue;
                    }
                case ExpectingNewline_2:
                    if (input == '\n')
                    {
                        state = HeaderLineStart;
                        continue;
                    }
                    else
                    {
                        return ErrorResult;
                    }
                case ExpectingNewline_3: {
                    std::vector<Request::HeaderItem>::iterator it = std::find_if(req.headers.begin(),
                                                                        req.headers.end(),
                                                                        checkIfConnection);

                    if( it != req.headers.end() )
                    {
                        if( strcasecmp(it->value.c_str(), "Keep-Alive") == 0 )
                        {
                            req.keepAlive = true;
                        }
                        else  // == Close
                        {
                            req.keepAlive = false;
                        }
                    }
                    else
                    {
                        if( req.versionMajor > 1 || (req.versionMajor == 1 && req.versionMinor == 1) )
                            req.keepAlive = true;
                    }

                    if( postSize == 0 )
                    {
                        if( input == '\n')
                            return CompletedResult;
                        else
                            return ErrorResult;
                    }
                    else
                    {
                        state = Post;
                        continue;
                    }
                }
                case Post:
                    --postSize;
                    req.postData.push_back( input );
                    if( postSize == 0 )
                        return CompletedResult;
                    else
                        continue;
                default:
                    return ErrorResult;
                }
            }

            return IncompletedResult;
        }

        // The current state of the parser.
        enum State
        {
            Head,
            ExpectingNewline_1,
            HeaderLineStart,
            HeaderLws,
            HeaderName,
            SpaceBeforeHeaderValue,
            HeaderValue,
            ExpectingNewline_2,
            ExpectingNewline_3,
            Post
        } state;

        size_t postSize;
        TParser headParser;
    };

    
}

typedef impl::HttpParserImpl<Request,  impl::HttpRequestParser> HttpRequestParser;
typedef impl::HttpParserImpl<Response, impl::HttpReponseParser> HttpResponseParser;


#endif // LIBAHTTP_REQUESTPARSER_H
