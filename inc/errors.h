#ifndef ERRORS_H
#define ERRORS_H

#include <stdexcept>

namespace notes
{

class Error : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};

}

#endif
