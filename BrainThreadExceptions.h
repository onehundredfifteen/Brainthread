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

  BrainThreadInvalidOptionException(const std::string &option, const std::string &inv_arg)
    : BrainThreadException(), op(option), op_arg(inv_arg)
    {}

  virtual const char* what() const throw()
  {
    cnvt.str("");
    cnvt << "Invalid argument " << op_arg << " of option \"" << op << "\".";
    s = cnvt.str();
    return s.c_str();
  }

  virtual const std::string& getOption() const {
      return op;
  }

protected:
    const std::string op;
    const std::string op_arg;
    const std::string message;
};

class BrainThreadOptionNotEligibleException : public BrainThreadInvalidOptionException {
public:

    BrainThreadOptionNotEligibleException(const std::string& option, const std::string& inv_arg, const std::string& message)
        : BrainThreadInvalidOptionException(option, inv_arg), message(message)
    {}

    virtual const char* what() const throw()
    {
        cnvt.str("");
        cnvt << "Cannot apply an option '" << op << "'=[" << op_arg << "]. Reason: " << message;
        s = cnvt.str();
        return s.c_str();
    }

protected:
    const std::string message;
};