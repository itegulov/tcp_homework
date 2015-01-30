#include "http_parser.h"

#include "boost/algorithm/string.hpp"

http_parser::http_parser()
{
}

void http_parser::parse(std::string data)
{
    std::cout << "DATA: " << "\"" << data << "\"" << std::endl;
    std::cout << pos_ << std::endl;
    int n = data.length();
    char p1 = 'a';
    char p2 = 'a';
    char p3 = 'a';
    int beg = -1;
    if (pos_ == STARTING_LINE)
    {
        std::string starting_line;
        for (int i = 0; i < n; i++)
        {
            if (data[i] == '\n')
            {
                starting_line = data.substr(0, i);
                parse_starting_line(starting_line);
                data = data.substr(i - 1);
                pos_ = HEAD;
                break;
            }
        }
    }

    if (pos_ == BODY)
    {
        std::cout << "I'm on body now" << std::endl;
        on_body(data);
    }
    else
    {
        std::cout << "LEFT: " << "\"" << data << "\"" << std::endl;
        n = data.length();
        std::cout << n << std::endl;
        for (int i = 0; i < n; i++)
        {
            std::cout << "!\"->" << data[i] << "<-\"!" << std::endl;
            if ((p3 == '\r' && p2 == '\n' && p1 == '\r' && data[i] == '\n') // CRLF CRLF
            || (p1 == '\n' && data[i] == '\n')) { // LF LF
                pos_ = BODY;
                beg = i + 1;
                parse_headers(headers_buffer_ + data.substr(0, beg));
                on_headers_end();
                break;
            }
            p3 = p2;
            p2 = p1;
            p1 = data[i];
        }
        if (beg == -1)
        {
            headers_buffer_ += data;
        }
        else if (beg != n)
        {
            on_body(data.substr(beg, n));
        }
    }
}

void http_parser::parse_starting_line(const std::string &starting_line)
{
    std::vector<std::string> strings;
    boost::split(strings, starting_line, boost::is_any_of("\t "));
    std::cout << strings[0] << " " << strings[1] << " " << strings[2] << std::endl;
    if (strings.size() == 2)
    {
        on_method(strings[0]);
        on_url(strings[1]);
        on_http_version(0, 9);
    }
    else if (strings.size() == 3)
    {
        on_method(strings[0]);
        on_url(strings[1]);
        std::vector<std::string> http_versions;
        std::string version = strings[2].substr(5);
        boost::split(http_versions, version, boost::is_any_of("."));
        assert(http_versions.size() == 2);
        on_http_version(std::stoi(http_versions[0]), std::stoi(http_versions[1]));
    }
    else
    {
        throw std::runtime_error("unkonwn type of http");
    }
}

void http_parser::parse_headers(const std::string &headers)
{
    std::vector<std::string> lines;
    boost::split(lines, headers, boost::is_any_of("\n"));
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        size_t pos = it->find_first_of(':');
        if (pos == std::string::npos) continue; //BIDLOKOD
        assert(pos != std::string::npos);
        on_header(it->substr(0, pos), it->substr(pos + 2));
    }
}

//HTTP CLIENT PARSER

http_client_parser::http_client_parser()
{

}

void http_client_parser::parse(std::string data)
{
    std::cout << "DATA: " << "\"" << data << "\"" << std::endl;
    std::cout << pos_ << std::endl;
    int n = data.length();
    char p1 = 'a';
    char p2 = 'a';
    char p3 = 'a';
    int beg = -1;
    if (pos_ == STARTING_LINE)
    {
        std::string starting_line;
        for (int i = 0; i < n; i++)
        {
            if (data[i] == '\n')
            {
                starting_line = data.substr(0, i);
                parse_starting_line(starting_line);
                data = data.substr(i - 1);
                pos_ = HEAD;
                break;
            }
        }
    }

    if (pos_ == BODY)
    {
        on_body(data);
    }
    else
    {
        std::cout << "LEFT: " << "\"" << data << "\"" << std::endl;
        n = data.length();
        std::cout << n << std::endl;
        for (int i = 0; i < n; i++)
        {
            std::cout << "!\"->" << data[i] << "<-\"!" << std::endl;
            if ((p3 == '\r' && p2 == '\n' && p1 == '\r' && data[i] == '\n') // CRLF CRLF
            || (p1 == '\n' && data[i] == '\n')) { // LF LF
                pos_ = BODY;
                beg = i + 1;
                parse_headers(headers_buffer_ + data.substr(0, beg));
                on_headers_end();
                break;
            }
            p3 = p2;
            p2 = p1;
            p1 = data[i];
        }
        if (beg == -1)
        {
            headers_buffer_ += data;
        }
        else if (beg != n)
        {
            on_body(data.substr(beg, n));
        }
    }
}

void http_client_parser::parse_starting_line(const std::string &starting_line)
{
    std::vector<std::string> strings;
    boost::split(strings, starting_line, boost::is_any_of("\t "));
    if (strings.size() == 3)
    {
        on_code(strings[1]);
        on_code_info(strings[2]);
        std::vector<std::string> http_versions;
        std::string version = strings[0].substr(5);
        boost::split(http_versions, version, boost::is_any_of("."));
        assert(http_versions.size() == 2);
        on_http_version(std::stoi(http_versions[0]), std::stoi(http_versions[1]));
    }
    else
    {
        throw std::runtime_error("unkonwn type of http");
    }
}

void http_client_parser::parse_headers(const std::string &headers)
{
    std::vector<std::string> lines;
    boost::split(lines, headers, boost::is_any_of("\n"));
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        size_t pos = it->find_first_of(':');
        if (pos == std::string::npos) continue; //BIDLOKOD
        assert(pos != std::string::npos);
        on_header(it->substr(0, pos), it->substr(pos + 2));
    }
}
