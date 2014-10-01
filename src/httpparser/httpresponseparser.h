/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: MIT
 */

#ifndef HTTPPARSER_RESPONSEPARSER_H
#define HTTPPARSER_RESPONSEPARSER_H

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>

#include "response.h"

// FIXME: chunk-extension, chunk trailer (??)

// Parser for incoming requests.
class HttpResponseParser
{
public:
    HttpResponseParser()
        : state(ResponseStatusStart),
          contentSize(0),
          chunkSize(0),
          chunked(false)
    {
    }

    // ParsingResult? ParsingCompleted, ParsingError...
    enum ParseResult {
        CompletedResult,
        IncompletedResult,
        ErrorResult
    };


    ParseResult parse(Response &resp, const char *begin, const char *end)
    {
        return consume(resp, begin, end);
    }

private:
    static bool checkIfConnection(const Response::HeaderItem &item)
    {
        return strcasecmp(item.name.c_str(), "Connection") == 0;
    }

    ParseResult consume(Response &resp, const char *begin, const char *end)
    {
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
                if (input == 'P' )
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
                    resp.versionMajor = 0;
                    resp.versionMinor = 0;
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
                    resp.versionMajor = input - '0';
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
                    resp.versionMajor = resp.versionMajor * 10 + input - '0';
                    continue;
                }
                else
                {
                    return ErrorResult;
                }
            case ResponseHttpVersion_minorStart:
                if (isDigit(input))
                {
                    resp.versionMinor = input - '0';
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
                    resp.statusCode = 0;
                    continue;
                }
                else if (isDigit(input))
                {
                    resp.versionMinor = resp.versionMinor * 10 + input - '0';
                    continue;
                }
                else
                {
                    return ErrorResult;
                }
            case ResponseHttpVersion_statusCodeStart:
                if (isDigit(input))
                {
                    resp.statusCode = input - '0';
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
                    resp.statusCode = resp.statusCode * 10 + input - '0';
                    continue;
                }
                else
                {
                    if( resp.statusCode < 100 || resp.statusCode > 999 )
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
                    resp.status += input;
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
                    state = ExpectingNewline_1;
                    continue;
                }
                else if( isChar(input) )
                {
                    resp.status += input;
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
                else if (!resp.headers.empty() && (input == ' ' || input == '\t'))
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
                    resp.headers.push_back(Response::HeaderItem());
                    resp.headers.back().name.reserve(16);
                    resp.headers.back().value.reserve(16);
                    resp.headers.back().name.push_back(input);
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
                    resp.headers.back().value.push_back(input);
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
                    resp.headers.back().name.push_back(input);
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
                    Response::HeaderItem &h = resp.headers.back();

                    if( strcasecmp(h.name.c_str(), "Content-Length") == 0 )
                    {
                        contentSize = atoi(h.value.c_str());
                        resp.content.reserve( contentSize );
                    }
                    else if( strcasecmp(h.name.c_str(), "Transfer-Encoding") == 0 )
                    {
                        if(strcasecmp(h.value.c_str(), "chunked") == 0)
                            chunked = true;
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
                    resp.headers.back().value.push_back(input);
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
                std::vector<Response::HeaderItem>::iterator it = std::find_if(resp.headers.begin(),
                                                                    resp.headers.end(),
                                                                    checkIfConnection);

                if( it != resp.headers.end() )
                {
                    if( strcasecmp(it->value.c_str(), "Keep-Alive") == 0 )
                    {
                        resp.keepAlive = true;
                    }
                    else  // == Close
                    {
                        resp.keepAlive = false;
                    }
                }
                else
                {
                    if( resp.versionMajor > 1 || (resp.versionMajor == 1 && resp.versionMinor == 1) )
                        resp.keepAlive = true;
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
                resp.content.push_back(input);
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
                    resp.content.reserve(resp.content.size() + chunkSize);

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
                resp.content.push_back(input);

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

#endif // HTTPPARSER_RESPONSEPARSER_H
