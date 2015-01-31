#include "tcp_exception.h"
const char* tcp_exception::what() const throw()
{
    return description.c_str();
}

tcp_exception::tcp_exception(const std::string& description)
{
    this->description = description;
}
