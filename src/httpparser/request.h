/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: MIT
 */

#ifndef HTTPPARSER_REQUEST_H
#define HTTPPARSER_REQUEST_H

#include <string>
#include <vector>

struct Request {
    Request()
        : keepAlive(false)
    {}
    
    struct HeaderItem
    {
        std::string name;
        std::string value;
    };

    std::string method;
    std::string uri;
    int versionMajor;
    int versionMinor;
    std::vector<HeaderItem> headers;
    std::vector<char> content;
    bool keepAlive;
};

inline bool operator == (const Request::HeaderItem &lhs, const Request::HeaderItem &rhs)
{
    if( strcasecmp(lhs.name.c_str(), rhs.name.c_str()) != 0 )
        return false;
    else
        return lhs.value == rhs.value;
}

inline bool operator == (const Request &lhs, const Request &rhs)
{
    if( lhs.method != rhs.method )
        return false;
    else if( lhs.uri != rhs.uri )
        return false;
    else if( lhs.versionMajor != rhs.versionMajor )
        return false;
    else if( lhs.versionMinor != rhs.versionMinor )
        return false;
    else if( lhs.headers != rhs.headers )
        return false;
    else if( lhs.content != rhs.content )
        return false;
        
    return lhs.keepAlive == rhs.keepAlive;
}


#endif // HTTPPARSER_REQUEST_H
