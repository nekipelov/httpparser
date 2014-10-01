/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: MIT
 */

#ifndef HTTPPARSER_RESPONSE_H
#define HTTPPARSER_RESPONSE_H

#include <string>
#include <vector>

struct Response {
    Response()
        : keepAlive(false)
    {}
    
    struct HeaderItem
    {
        std::string name;
        std::string value;
    };

    int versionMajor;
    int versionMinor;
    std::vector<HeaderItem> headers;
    std::vector<char> content;
    bool keepAlive;
    
    unsigned int statusCode;
    std::string status;
};

inline bool operator == (const Response::HeaderItem &lhs, const Response::HeaderItem &rhs)
{
    if( strcasecmp(lhs.name.c_str(), rhs.name.c_str()) != 0 )
        return false;
    else
        return lhs.value == rhs.value;
}

inline bool operator == (const Response &lhs, const Response &rhs)
{
    
    if( lhs.statusCode != rhs.statusCode )
        return false;
    else if( lhs.status != rhs.status )
        return false;
    else if( lhs.versionMajor != rhs.versionMajor )
        return false;
    else if( lhs.versionMinor != rhs.versionMinor )
        return false;
    else if( lhs.headers != rhs.headers )
        return false;
    else if( lhs.content != rhs.content )
        return false;
    return true;
}


#endif // HSERV_RESPONSE_H

