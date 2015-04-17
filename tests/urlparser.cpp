#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include <httpparser/urlparser.h>

BOOST_AUTO_TEST_SUITE(UrlParserTest)

using httpparser::UrlParser;

BOOST_AUTO_TEST_CASE(http_url)
{
    const char *text = "http://www.example.com/dir/subdir?param=1&param=2;param%20=%20#fragment";
    UrlParser parser(text);

    BOOST_CHECK_EQUAL(parser.isValid(), true);
    BOOST_CHECK_EQUAL(parser.scheme(), "http");
    BOOST_CHECK_EQUAL(parser.hostname(), "www.example.com");
    BOOST_CHECK_EQUAL(parser.path(), "/dir/subdir");
    BOOST_CHECK_EQUAL(parser.query(), "param=1&param=2;param%20=%20");
    BOOST_CHECK_EQUAL(parser.fragment(), "fragment");
    BOOST_CHECK_EQUAL(parser.httpPort(), 80);
}

BOOST_AUTO_TEST_CASE(http_only_hostname)
{
    const char *text = "http://www.example.com";
    UrlParser parser(text);

    BOOST_CHECK_EQUAL(parser.isValid(), true);
    BOOST_CHECK_EQUAL(parser.scheme(), "http");
    BOOST_CHECK_EQUAL(parser.hostname(), "www.example.com");
    BOOST_CHECK_EQUAL(parser.path(), "/");
}

BOOST_AUTO_TEST_CASE(http_url_with_username)
{
    const char *text = "http://username@www.example.com/dir/subdir?param=1&param=2;param%20=%20#fragment";
    UrlParser parser(text);

    BOOST_CHECK_EQUAL(parser.isValid(), true);
    BOOST_CHECK_EQUAL(parser.scheme(), "http");
    BOOST_CHECK_EQUAL(parser.username(), "username");
    BOOST_CHECK_EQUAL(parser.hostname(), "www.example.com");
    BOOST_CHECK_EQUAL(parser.path(), "/dir/subdir");
    BOOST_CHECK_EQUAL(parser.query(), "param=1&param=2;param%20=%20");
    BOOST_CHECK_EQUAL(parser.fragment(), "fragment");
    BOOST_CHECK_EQUAL(parser.httpPort(), 80);
}

BOOST_AUTO_TEST_CASE(http_url_with_username_and_password)
{
    const char *text = "http://username:passwd@www.example.com/dir/subdir?param=1&param=2;param%20=%20#fragment";
    UrlParser parser(text);

    BOOST_CHECK_EQUAL(parser.isValid(), true);
    BOOST_CHECK_EQUAL(parser.scheme(), "http");
    BOOST_CHECK_EQUAL(parser.username(), "username");
    BOOST_CHECK_EQUAL(parser.password(), "passwd");
    BOOST_CHECK_EQUAL(parser.hostname(), "www.example.com");
    BOOST_CHECK_EQUAL(parser.path(), "/dir/subdir");
    BOOST_CHECK_EQUAL(parser.query(), "param=1&param=2;param%20=%20");
    BOOST_CHECK_EQUAL(parser.fragment(), "fragment");
    BOOST_CHECK_EQUAL(parser.httpPort(), 80);
}

BOOST_AUTO_TEST_CASE(http_url_with_port)
{
    const char *text = "http://www.example.com:8080/dir/subdir?param=1&param=2;param%20=%20#fragment";
    UrlParser parser(text);

    BOOST_CHECK_EQUAL(parser.isValid(), true);
    BOOST_CHECK_EQUAL(parser.scheme(), "http");
    BOOST_CHECK_EQUAL(parser.hostname(), "www.example.com");
    BOOST_CHECK_EQUAL(parser.path(), "/dir/subdir");
    BOOST_CHECK_EQUAL(parser.query(), "param=1&param=2;param%20=%20");
    BOOST_CHECK_EQUAL(parser.fragment(), "fragment");
    BOOST_CHECK_EQUAL(parser.httpPort(), 8080);
}

BOOST_AUTO_TEST_CASE(http_url_with_username_and_port)
{
    const char *text = "http://username:passwd@www.example.com:8080/dir/subdir?param=1&param=2;param%20=%20#fragment";
    UrlParser parser(text);

    BOOST_CHECK_EQUAL(parser.isValid(), true);
    BOOST_CHECK_EQUAL(parser.scheme(), "http");
    BOOST_CHECK_EQUAL(parser.username(), "username");
    BOOST_CHECK_EQUAL(parser.password(), "passwd");
    BOOST_CHECK_EQUAL(parser.hostname(), "www.example.com");
    BOOST_CHECK_EQUAL(parser.path(), "/dir/subdir");
    BOOST_CHECK_EQUAL(parser.query(), "param=1&param=2;param%20=%20");
    BOOST_CHECK_EQUAL(parser.fragment(), "fragment");
    BOOST_CHECK_EQUAL(parser.httpPort(), 8080);
}

BOOST_AUTO_TEST_CASE(ftp_url)
{
    const char *text = "ftp://username:passwd@ftp.example.com/dir/filename.ext";
    UrlParser parser(text);

    BOOST_CHECK_EQUAL(parser.isValid(), true);
    BOOST_CHECK_EQUAL(parser.scheme(), "ftp");
    BOOST_CHECK_EQUAL(parser.username(), "username");
    BOOST_CHECK_EQUAL(parser.password(), "passwd");
    BOOST_CHECK_EQUAL(parser.hostname(), "ftp.example.com");
    BOOST_CHECK_EQUAL(parser.path(), "/dir/filename.ext");
    BOOST_CHECK_EQUAL(parser.query(), "");
    BOOST_CHECK_EQUAL(parser.fragment(), "");
}

BOOST_AUTO_TEST_CASE(email_url)
{
    const char *text = "mailto:username@example.com";
    UrlParser parser(text);

    BOOST_CHECK_EQUAL(parser.isValid(), true);
    BOOST_CHECK_EQUAL(parser.scheme(), "mailto");
    BOOST_CHECK_EQUAL(parser.username(), "username");
    BOOST_CHECK_EQUAL(parser.hostname(), "example.com");
}

BOOST_AUTO_TEST_CASE(svn_over_ssh)
{
    const char *text = "svn+ssh://hostname-01.org/path/to/file";
    UrlParser parser(text);

    BOOST_CHECK_EQUAL(parser.isValid(), true);
    BOOST_CHECK_EQUAL(parser.scheme(), "svn+ssh");
    BOOST_CHECK_EQUAL(parser.hostname(), "hostname-01.org");
    BOOST_CHECK_EQUAL(parser.path(), "/path/to/file");
}

BOOST_AUTO_TEST_SUITE_END()
