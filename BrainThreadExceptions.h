#pragma once

#include <exception>
#include <stdexcept>
#include <sstream>
#include <string>

/*
 * Klasy Wyj¹tków.
 * Definiuja rózne wyj¹tki, które uzywa program.
*/

class BrainThreadException: public std::exception {
public:

  BrainThreadException()
    : exception("")
    {}

  virtual const char* what() const throw()
  {
	s = "BrainThread Exception: ";
    return s.c_str();
  }

protected:
    static std::ostringstream cnvt;
	static std::string s;
};

class BrainThreadInvalidOptionException: public BrainThreadException {
public:

  BrainThreadInvalidOptionException(std::string option, std::string inv_arg)
    : BrainThreadException(), op(option), op_arg(inv_arg)
    {}

  virtual const char* what() const throw()
  {
    cnvt.str("");
    cnvt << "Invalid argument " << op_arg << " of option \"" << op << "\".";
    s = cnvt.str();
    return s.c_str();
  }

protected:
    std::string op;
	std::string op_arg;
};