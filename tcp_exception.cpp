#include "tcp_exception.h"
const char* tcp_exception::what() const throw()
{
    return description;
}

tcp_exception::tcp_exception(const char * description)
{
    this->description = description;
}
