#ifndef COMMON_H
#define COMMON_H

#include <httpparser/request.h>
#include <httpparser/response.h>

std::ostream & operator<< (std::ostream &stream, const httpparser::Request &req)
{
    stream << req.method << " " << req.uri << " HTTP/" 
           << req.versionMajor << "." << req.versionMinor << "\n";
           
    for(std::vector<httpparser::Request::HeaderItem>::const_iterator it = req.headers.begin();
        it != req.headers.end(); ++it)
    {
        stream << it->name << ": " << it->value << "\n";
    }
    
    stream << req.content.data() << "\n";
    stream << "+ keep-alive: " << req.keepAlive << "\n";;
    
    return stream;
}

std::ostream & operator<< (std::ostream &stream, const httpparser::Response &resp)
{
    stream << "HTTP/" << resp.versionMajor << "." << resp.versionMinor
           << " " << resp.statusCode << " " << resp.status << "\n";

    for(std::vector<httpparser::Response::HeaderItem>::const_iterator it = resp.headers.begin();
        it != resp.headers.end(); ++it)
    {
        stream << it->name << ": " << it->value << "\n";
    }

    stream << resp.content.data() << "\n";

    return stream;
}

class RequestDsl
{
public:
    RequestDsl &method(const char *name)
    {
        request.method = name;
        return *this;
    }

    RequestDsl &uri(const char *name)
    {
        request.uri = name;
        return *this;
    }

    RequestDsl &version(int major, int minor)
    {
        request.versionMajor = major;
        request.versionMinor = minor;
        return *this;
    }
    
    RequestDsl &keepAlive(bool keep)
    {
        request.keepAlive = keep;
        return *this;
    }
    
    RequestDsl &header(const std::string &name, const std::string &value)
    {
        httpparser::Request::HeaderItem item = {name, value};
        request.headers.push_back(item);
        return *this; 
    }

    RequestDsl &content(const std::vector<char> &data)
    {
        request.content = data;
        return *this; 
    }

    template<size_t N>
    RequestDsl &content(const char (&buf)[N])
    {
        request.content = std::vector<char>(buf, buf + N - 1);
        return *this; 
    }
    
    operator httpparser::Request () const
    {
        return request;
    }

private:
    httpparser::Request request;
};

class ResponseDsl
{
public:
    ResponseDsl &statusCode(unsigned int statusCode)
    {
        response.statusCode = statusCode;
        return *this;
    }

    ResponseDsl &status(const std::string &status)
    {
        response.status = status;
        return *this;
    }

    ResponseDsl &version(int major, int minor)
    {
        response.versionMajor = major;
        response.versionMinor = minor;
        return *this;
    }

    ResponseDsl &header(const std::string &name, const std::string &value)
    {
        httpparser::Response::HeaderItem item = {name, value};
        response.headers.push_back(item);
        return *this;
    }

    ResponseDsl &body(const std::vector<char> &data)
    {
        response.content = data;
        return *this;
    }

    template<size_t N>
    ResponseDsl &body(const char (&buf)[N])
    {
        response.content = std::vector<char>(buf, buf + N - 1);
        return *this;
    }

    operator httpparser::Response () const
    {
        return response;
    }

private:
    httpparser::Response response;
};


#endif // COMMON_H
