#pragma once

#include <exception>
#include <stdexcept>
#include <sstream>
#include <string>

#include <windows.h>

/*
 * Klasy Wyj¹tków.
 * Definiuja rózne wyj¹tki, które interpreter rzuca podczas wykonywnaia kodu. 
 * (nie s¹ to b³êdy, które mozna wykryæ na etapie kompilacji)
*/

#define ERROR_CODE_NOTENOUGHMEMORY 0x3E1C
#define ERROR_CODE_TOOMANYTHREADSTOWAIT 0x3E1D
#define ERROR_CODE_RESERVED1 0x3E1E
#define ERROR_CODE_RESERVED2 0x3E1F

class BrainThreadRuntimeException: public std::runtime_error {
public:

  BrainThreadRuntimeException()
    : runtime_error("")
    {}

  virtual const char* what() const throw()
  {
	s = "Runtime Exception: ";
    return s.c_str();
  }

protected:
    static std::ostringstream cnvt;
	static std::string s;
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
    s = cnvt.str();
    return s.c_str();
  }

protected:
    unsigned ma;
	unsigned so_mc;
};

class BFRangeException: public BrainThreadRuntimeException {
public:

  BFRangeException(__int64 pointer_pos)
    : BrainThreadRuntimeException(), ppos(pointer_pos)
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
    cnvt << BrainThreadRuntimeException::what() << "The pointer tried to reach a cell out of memory range. Pointer position: " << ppos << ".";
    s = cnvt.str();
    return s.c_str();
  }

protected:
    __int64 ppos;
};

class BFUndefinedFunctionException: public BrainThreadRuntimeException {
public:

  BFUndefinedFunctionException(unsigned int fun_name)
    : BrainThreadRuntimeException(), fname(fun_name)
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
	cnvt << "Call to undefined function '" << fname << "'.";
    s = cnvt.str();
    return s.c_str();
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
	cnvt << BrainThreadRuntimeException::what() << "Function '" << fname << "' already exists.";
    s = cnvt.str();
    return s.c_str();
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
	cnvt << BrainThreadRuntimeException::what() << "Function's stack overflow.";
    s = cnvt.str();
    return s.c_str();
  }
};

class BFMemoryStackOverflowException: public BrainThreadRuntimeException {
public:

  BFMemoryStackOverflowException()
    : BrainThreadRuntimeException()
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
	cnvt << BrainThreadRuntimeException::what() << "Memory stack overflow.";
    s = cnvt.str();
    return s.c_str();
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
		case ERROR_CODE_NOTENOUGHMEMORY:
			cnvt << "out of memory"; break;
	    case ERROR_MAX_THRDS_REACHED:
			cnvt << "too many threads"; break;
		case ERROR_NOT_ENOUGH_MEMORY:
			cnvt << "not enough system memory"; break;
		case ERROR_INVALID_HANDLE:
			cnvt << "invalid handle"; break;
		default:
			cnvt << "unknown. System error code: " << err_no;
	}

     s = cnvt.str();
    return s.c_str();
  }

protected:
    unsigned err_no;
};

class BFJoinThreadException: public BrainThreadRuntimeException {
public:

  BFJoinThreadException(unsigned e)
    : BrainThreadRuntimeException(), err_no(e)
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
	cnvt << BrainThreadRuntimeException::what() << "Cannot join threads. Reason: ";

	switch(err_no)
	{
		case ERROR_CODE_TOOMANYTHREADSTOWAIT:
			cnvt << "too many threads to wait for"; break;
		default:
			cnvt << "wait failed. System error code: " << err_no;
	}

    s = cnvt.str();
    return s.c_str();
  }

protected:
    unsigned err_no;
};

class BFInvalidInputStreamException: public BrainThreadRuntimeException {
public:

  BFInvalidInputStreamException()
    : BrainThreadRuntimeException()
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
	cnvt << BrainThreadRuntimeException::what() << "Invalid input stream.";
    s = cnvt.str();
    return s.c_str();
  }
};

class BFCannotMonitorThreadsException: public BrainThreadRuntimeException {
public:

  BFCannotMonitorThreadsException()
    : BrainThreadRuntimeException()
    {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );
    cnvt << BrainThreadRuntimeException::what() << "Cannot monitor child threads: out of memory.";
    s = cnvt.str();
    return s.c_str();
  }
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
    s = cnvt.str();
    return s.c_str();
  }
};


