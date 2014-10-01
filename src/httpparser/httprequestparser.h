/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: MIT
 */

#ifndef LIBAHTTP_REQUESTPARSER_H
#define LIBAHTTP_REQUESTPARSER_H

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>

#include "request.h"

// Parser for incoming requests.
class HttpRequestParser
{
public:
    HttpRequestParser()
        : state(RequestMethodStart), contentSize(0),
          chunkSize(0), chunked(false)

    {
    }

    // ParsingResult? ParsingCompleted, ParsingError...
    enum ParseResult {
        CompletedResult,
        IncompletedResult,
        ErrorResult
    };

    ParseResult parse(Request &req, const char *begin, const char *end)
    {
        return consume(req, begin, end);
    }

private:
    static bool checkIfConnection(const Request::HeaderItem &item)
    {
        return strcasecmp(item.name.c_str(), "Connection") == 0;
    }

    ParseResult consume(Request &req, const char *begin, const char *end)
    {
        while( begin != end )
        {
            char input = *begin++;

            switch (state)
            {
            case RequestMethodStart:
                if (!isChar(input) || isControl(input) || isSpecial(input))
                {
                    return ErrorResult;
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
                    return ErrorResult;
                }
                else
                {
                    req.method.push_back(input);
                    continue;
                }
            case RequestUriStart:
                if (isControl(input))
                {
                    return ErrorResult;
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

                    return CompletedResult;
                }
                else if (isControl(input))
                {
                    return ErrorResult;
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
                    return ErrorResult;
                }
            case RequestHttpVersion_ht:
                if (input == 'T')
                {
                    state = RequestHttpVersion_htt;
                    continue;
                }
                else
                {
                    return ErrorResult;
                }
            case RequestHttpVersion_htt:
                if (input == 'T')
                {
                    state = RequestHttpVersion_http;
                    continue;
                }
                else
                {
                    return ErrorResult;
                }
            case RequestHttpVersion_http:
                if (input == 'P')
                {
                    state = RequestHttpVersion_slash;
                    continue;
                }
                else
                {
                    return ErrorResult;
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
                    return ErrorResult;
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
                    return ErrorResult;
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
                    return ErrorResult;
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
                    return ErrorResult;
                }
            case RequestHttpVersion_minor:
                if (input == '\r')
                {
                    state = ExpectingNewline_1;
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
                    if( req.method == "POST" || req.method == "PUT" )
                    {
                        Request::HeaderItem &h = req.headers.back();

                        if( strcasecmp(h.name.c_str(), "Content-Length") == 0 )
                        {
                            contentSize = atoi(h.value.c_str());
                            req.content.reserve( contentSize );
                        }
                        else if( strcasecmp(h.name.c_str(), "Transfer-Encoding") == 0 )
                        {
                            if(strcasecmp(h.value.c_str(), "chunked") == 0)
                                chunked = true;
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

                if(chunked)
                {
                    state = ChunkSize;
                    continue;
                }
                else if( contentSize == 0 )
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
                --contentSize;
                req.content.push_back( input );
                if( contentSize == 0 )
                    return CompletedResult;
                else
                    continue;
            case ChunkSize:
                if(isalnum(input))
                {
                    chunkSizeStr.push_back(input);
                }
                else if(input == '\r')
                {
                    state = ChunkSizeNewLine;
                }
                else
                {
                    return ErrorResult;
                }
                break;
            case ChunkSizeNewLine:
                if( input == '\n' )
                {
                    chunkSize = strtol(chunkSizeStr.c_str(), NULL, 16);
                    chunkSizeStr.clear();
                    req.content.reserve(req.content.size() + chunkSize);

                    if( chunkSize == 0 )
                        state = ChunkSizeNewLine_2;
                    else
                        state = ChunkData;
                }
                else
                {
                    return ErrorResult;
                }
                break;
            case ChunkSizeNewLine_2:
                if( input == '\r' )
                {
                    state = ChunkSizeNewLine_3;
                }
                else
                {
                    return ErrorResult;
                }
                break;
            case ChunkSizeNewLine_3:
                if( input == '\n' )
                {
                    return CompletedResult;
                }
                else
                {
                    return ErrorResult;
                }
                break;
            case ChunkData:
                req.content.push_back(input);

                if( --chunkSize == 0 )
                {
                    state = ChunkDataNewLine_1;
                }
                break;
            case ChunkDataNewLine_1:
                if( input == '\r' )
                {
                    state = ChunkDataNewLine_2;
                }
                else
                {
                    return ErrorResult;
                }
                break;
            case ChunkDataNewLine_2:
                if( input == '\n' )
                {
                    state = ChunkSize;
                }
                else
                {
                    return ErrorResult;
                }
                break;
            default:
                return ErrorResult;
            }
        }

        return IncompletedResult;
    }

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

    // The current state of the parser.
    enum State
    {
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
        
        ResponseStatusStart,
        ResponseHttpVersion_ht,
        ResponseHttpVersion_htt,
        ResponseHttpVersion_http,
        ResponseHttpVersion_slash,
        ResponseHttpVersion_majorStart,
        ResponseHttpVersion_major,
        ResponseHttpVersion_minorStart,
        ResponseHttpVersion_minor,
        ResponseHttpVersion_spaceAfterVersion,
        ResponseHttpVersion_statusCodeStart,
        ResponseHttpVersion_spaceAfterStatusCode,
        ResponseHttpVersion_statusTextStart,
        
        ExpectingNewline_1,
        HeaderLineStart,
        HeaderLws,
        HeaderName,
        SpaceBeforeHeaderValue,
        HeaderValue,
        ExpectingNewline_2,
        ExpectingNewline_3,

        Post,
        ChunkSize,
        ChunkSizeNewLine,
        ChunkSizeNewLine_2,
        ChunkSizeNewLine_3,

        ChunkDataNewLine_1,
        ChunkDataNewLine_2,
        ChunkData,
    } state;

    size_t contentSize;
    std::string chunkSizeStr;
    size_t chunkSize;
    bool chunked;
};

#endif // LIBAHTTP_REQUESTPARSER_H
