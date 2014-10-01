/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: MIT
 */

#ifndef HTTPPARSER_URLPARSER_H
#define HTTPPARSER_URLPARSER_H

#include <string>
#include <stdint.h>

class UrlParser
{
public:
    explicit UrlParser(const std::string &url)
        : valid(true)
    {
        parse(url);
    }

    bool isValid() const
    {
        return valid;
    }

    std::string scheme() const
    {
        return url.scheme;
    }

    std::string username() const
    {
        return url.username;
    }

    std::string password() const
    {
        return url.password;
    }

    std::string hostname() const
    {
        return url.hostname;
    }

    std::string port() const
    {
        return url.port;
    }

    std::string path() const
    {
        return url.path;
    }

    std::string query() const
    {
        return url.query;
    }

    std::string fragment() const
    {
        return url.fragment;
    }

    uint16_t httpPort() const
    {
        const uint16_t defaultHttpPort = 80;
        const uint16_t defaultHttpsPort = 443;

        if( url.port.empty() )
        {
            if( scheme() == "https" )
                return defaultHttpsPort;
            else
                return defaultHttpPort;
        }
        else
        {
            return url.integerPort;
        }
    }

private:
    bool isUnreserved(char ch)
    {
        if( isalnum(ch) )
            return true;

        switch(ch)
        {
        case '-':
        case '.':
        case '_':
        case '~':
            return true;
        }

        return false;
    }

    void parse(const std::string &str)
    {
        enum {
            Scheme,
            SlashAfterScheme1,
            SlashAfterScheme2,
            UsernameOrHostname,
            Password,
            Hostname,
            IPV6Hostname,
            PortOrPassword,
            Port,
            Path,
            Query,
            Fragment
        } state = Scheme;

        std::string usernameOrHostname;
        std::string portOrPassword;

        url.path = "/";
        url.integerPort = 0;

        for(size_t i = 0; i < str.size() && valid; ++i)
        {
            char ch = str[i];

            switch(state)
            {
            case Scheme:
                if( isalpha(ch) || ch == '+' || ch == '-' || ch == '.')
                {
                    url.scheme += ch;
                }
                else if( ch == ':' )
                {
                    state = SlashAfterScheme1;
                }
                else
                {
                    valid = false;
                    url = Url();
                }
                break;
            case SlashAfterScheme1:
                if( ch == '/' )
                {
                    state = SlashAfterScheme2;
                }
                else if( isalnum(ch) )
                {
                    usernameOrHostname = ch;
                    state = UsernameOrHostname;
                }
                else
                {
                    valid = false;
                    url = Url();
                }
                break;
            case SlashAfterScheme2:
                if( ch == '/' )
                {
                    state = UsernameOrHostname;
                }
                else
                {
                    valid = false;
                    url = Url();
                }
                break;
            case UsernameOrHostname:
                if( isUnreserved(ch) || ch == '%' )
                {
                    usernameOrHostname += ch;
                }
                else if( ch == ':' )
                {
                    state = PortOrPassword;
                }
                else if( ch == '@' )
                {
                    state = Hostname;
                    std::swap(url.username, usernameOrHostname);
                }
                else if( ch == '/' )
                {
                    state = Path;
                    std::swap(url.hostname, usernameOrHostname);
                }
                else
                {
                    valid = false;
                    url = Url();
                }
                break;
            case Password:
                if( isalpha(ch) || ch == '%' )
                {
                    url.password += ch;
                }
                else if( ch == '@' )
                {
                    state = Hostname;
                }
                else
                {
                    valid = false;
                    url = Url();
                }
                break;
            case Hostname:
                if( ch == '[' && url.hostname.empty() )
                {
                    state = IPV6Hostname;
                }
                else if(isUnreserved(ch) || ch == '%')
                {
                    url.hostname += ch;
                }
                else if(ch == ':')
                {
                    state = Port;
                }
                else if(ch == '/')
                {
                    state = Path;
                }
                else
                {
                    valid = false;
                    url = Url();
                }
                break;
            case IPV6Hostname:
                abort(); // TODO
            case PortOrPassword:
                if( isdigit(ch) )
                {
                    portOrPassword += ch;
                }
                else if( ch == '/' )
                {
                    std::swap(url.hostname, usernameOrHostname);
                    std::swap(url.port, portOrPassword);
                    url.integerPort = atoi(url.port.c_str());
                    state = Path;
                }
                else if( isalpha(ch) || ch == '%' )
                {
                    std::swap(url.username, usernameOrHostname);
                    std::swap(url.password, portOrPassword);
                    url.password += ch;
                    state = Password;
                }
                else
                {
                    valid = false;
                    url = Url();
                }
                break;
            case Port:
                if( isdigit(ch) )
                {
                    portOrPassword += ch;
                }
                else if(ch == '/')
                {
                    std::swap(url.port, portOrPassword);
                    url.integerPort = atoi(url.port.c_str());
                    state = Path;
                }
                else
                {
                    valid = false;
                    url = Url();
                }
                break;
            case Path:
                if( ch == '#')
                {
                    state = Fragment;
                }
                else if( ch == '?' )
                {
                    state = Query;
                }
                else
                {
                    url.path += ch;
                }
                break;
            case Query:
                if( ch == '#')
                {
                    state = Fragment;
                }
                else if( ch == '?' )
                {
                    state = Query;
                }
                else
                {
                    url.query += ch;
                }
                break;
            case Fragment:
                url.fragment += ch;
                break;
            }
        }

        assert(portOrPassword.empty());

        if( !usernameOrHostname.empty() )
            url.hostname = usernameOrHostname;
    }


    bool valid;

    struct Url
    {
        std::string scheme;
        std::string username;
        std::string password;
        std::string hostname;
        std::string port;
        std::string path;
        std::string query;
        std::string fragment;
        uint16_t integerPort;
    } url;
};

#endif // HTTPPARSER_URLPARSER_H
