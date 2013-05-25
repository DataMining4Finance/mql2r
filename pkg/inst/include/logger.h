
#pragma once

#include <memory>
#include <fstream>

struct logger
{
	static std::ofstream& instance()
	{
		if(!m_instance.get())
		{
			//FILE *fd = fopen (filename(), "a+");
			m_instance.reset(new std::ofstream());

			m_instance->open(filename(),std::ios::app);
		    freopen(logger::filename(),"a",stderr);
		    freopen(logger::filename(),"a",stdout);

			//HANDLE fhand = (HANDLE)_get_osfhandle(_fileno(fd));
			//SetStdHandle(STD_OUTPUT_HANDLE,fhand);
			//SetStdHandle(STD_ERROR_HANDLE,fhand);

		}

		return *m_instance;
	}

	static const char* filename()
	{
		return "experts/logs/MQL2R.log";
	}

private:
	static std::auto_ptr<std::ofstream> m_instance;
};

struct scope_logger
{
	scope_logger(const std::string& fn) : m_fn(fn)
	{
		logger::instance() << "Entering " << m_fn << " called from threadid " << GetCurrentThreadId() << std::endl;
	}

	~scope_logger()
	{
		logger::instance() << "Leaving " << m_fn << " called from threadid " << GetCurrentThreadId() << std::endl;
	}

	const std::string m_fn;
};

#define LOGCALL() scope_logger __logger_declare(__FUNCTION__)
