#pragma once

#include <exception>
#include <stdexcept>
#include <sstream>

class BrainThreadRuntimeException: public std::runtime_error {
public:

  BrainThreadRuntimeException()
    : runtime_error("")
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
    cnvt << "BrainThread RuntimeException ";
    return cnvt.str().c_str();
  }

protected:
    static std::ostringstream cnvt;
};

class BFAllocException: public BrainThreadRuntimeException {
public:

  BFAllocException(unsigned mem_amount, unsigned cell_size)
    : BrainThreadRuntimeException(), ma(mem_amount), so_mc(cell_size)
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
    cnvt << BrainThreadRuntimeException::what() << "Cannot allocate " << ma << " cells of memory (" << so_mc * ma << " bytes).";
    return cnvt.str().c_str();
  }

protected:
    unsigned ma;
	unsigned so_mc;
};

class BFRangeException: public BrainThreadRuntimeException {
public:

  BFRangeException(unsigned int pointer_pos)
    : BrainThreadRuntimeException(), ppos(pointer_pos)
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
    cnvt << BrainThreadRuntimeException::what() << "The pointer tried to reach a cell out of memory range. Pointer position: " << ppos << ".";
    return cnvt.str().c_str();
  }

protected:
    unsigned ppos;
};

class BFUndefinedFunctionException: public BrainThreadRuntimeException {
public:

  BFUndefinedFunctionException(unsigned int fun_name)
    : BrainThreadRuntimeException(), fname(fun_name)
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
	cnvt << BrainThreadRuntimeException::what() << "Call to undefined function #" << fname << ".";
    return cnvt.str().c_str();
  }

protected:
    unsigned fname;
};

class BFExistantFunctionException: public BrainThreadRuntimeException {
public:

  BFExistantFunctionException(unsigned int fun_name)
    : BrainThreadRuntimeException(), fname(fun_name)
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
	cnvt << BrainThreadRuntimeException::what() << "Function #" << fname << " already exists.";
    return cnvt.str().c_str();
  }

protected:
    unsigned fname;
};

class BFFunctionStackOverflowException: public BrainThreadRuntimeException {
public:

  BFFunctionStackOverflowException()
    : BrainThreadRuntimeException()
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
	cnvt << BrainThreadRuntimeException::what() << "Functions stack overflow.";
    return cnvt.str().c_str();
  }
};

class BFForkThreadException: public BrainThreadRuntimeException {
public:

  BFForkThreadException(unsigned e)
    : BrainThreadRuntimeException(), err_no(e)
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
	cnvt << BrainThreadRuntimeException::what() << "Cannot fork a thread. Reason: ";

	switch(err_no)
	{
		case 11:
			cnvt << "too many threads"; break;
		case 13:
			cnvt << "not enough memory"; break;
		case 22:
			cnvt << "invalid argument"; break;
		default:
			cnvt << "unkown (errno=" << err_no << ")";
	}

    return cnvt.str().c_str();
  }

protected:
    unsigned err_no;
};

class BFUnkownException: public BrainThreadRuntimeException {
public:

  BFUnkownException()
    : BrainThreadRuntimeException()
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
    cnvt << BrainThreadRuntimeException::what() << "An unkown exception occured.";
    return cnvt.str().c_str();
  }
};


