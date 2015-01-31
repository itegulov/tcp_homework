#ifndef TCP_EXCEPTION_H
#define TCP_EXCEPTION_H
#include <exception>
#include <string>


struct tcp_exception: public std::exception
{
public:
    tcp_exception(const std::string& description);
    virtual const char* what() const throw();
private:
    std::string description;
};
#endif //TCP_EXCEPTION_H
