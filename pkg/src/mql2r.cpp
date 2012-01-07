
#include "mql2r.h"
#include <RInside.h>                    // for the embedded R via RInside
#include <Rembedded.h>
#include <RDefines.h>
#include <Rinternals.h>
#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/foreach.hpp>
#include <set>
#include <algorithm>
#include <fstream>
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include <FinSeries.hpp>

#define MT4R_VERSION_MAJOR 1l
#define MT4R_VERSION_MINOR 3l


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
//#define LOGCALL() logger::instance() << "Entering" << __FUNCTION__ << " called from threadid " << GetCurrentThreadId() << std::endl;


struct handle_manager
{
	handle_manager() : m_Rinstance(NULL)
	{
		   logger::instance() << "Initialising MQL2R" << std::endl;
	}

	~handle_manager()
	{
		   logger::instance() << "De-Initialising MQL2R" << std::endl;
	}

	static handle_manager& instance()
	{
		return m_instance;
	}

	int create_handle()
	{
		if(m_handles.empty())
		{
			logger::instance() << "Creating instance of R on thread " << GetCurrentThreadId() << std::endl;
			m_Rinstance = new RInside;
		}

		m_handles.insert(++m_lasthandle);

		return m_lasthandle;
	}

	void destroy_handle(int handle)
	{
		if(m_handles.find(handle) != m_handles.end())
		{
			m_handles.erase(handle);

			if(m_handles.empty())
			{
				logger::instance() << "Removing instance of R on thread " << GetCurrentThreadId() << std::endl;
				delete m_Rinstance;
				m_Rinstance = NULL;
			}
		}
	}

	int m_lasthandle;
	std::set<int> m_handles;
	RInside* m_Rinstance;
	static handle_manager m_instance;
};


std::auto_ptr<std::ofstream> logger::m_instance(0);
handle_manager handle_manager::m_instance;

std::ostream& operator << (std::ostream& ostr,MqlStr* str)
{
	return ostr << str->string;
}

std::string toStdString(MqlStr* str)
{
	return std::string(str->string,str->len);
}

extern "C" {
  /**
   * Return the dll version. The upper 16 bit of the return value
   * are the major version and the lower 16 bit the minor. This
   * is used in RInit() to make sure that this header file and
   * the dll fit together.
   */
   int MT4_EXPFUNC RGetDllVersion()
   {
	   return (MT4R_VERSION_MAJOR << 16) + MT4R_VERSION_MINOR;
   }

   /**
   * This is not meant to be called directly, it will be
   * called by RInit() after the successful version check.
   * You should call RInit() to start a new R session.
   */
   int MT4_EXPFUNC RInit_(char* commandline, int debuglevel)
   {
	   LOGCALL();
	   try
	   {
		   return handle_manager::instance().create_handle();
	   } catch(std::exception& ex) {
		   logger::instance() << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << "Unknown exception caught" << std::endl;
		   throw;
	   }
	   return 0;
   }


   /**
   * Teminate the R session. Call this in your deinit() function.
   * After this the handle is no longer valid.
   */
   void MT4_EXPFUNC RDeinit(int handle)
   {
	   LOGCALL();

	   try
	   {
		   handle_manager::instance().destroy_handle(handle);
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }


   /**
   * return true if the R session belonging to this handle is
   * still runing. R will terminate on any fatal error in the
   * code you send it. You should check this at the beginning
   * of your start function and stop all actions. The last
   * command prior to the crash will be found in the log.
   * If R is not running anymore this library won't emit any
   * more log messages and will silently ignore all commands.
   */
   BOOL MT4_EXPFUNC RIsRunning()
   {
	   LOGCALL();

	   return false;
   }



   /**
   * return true if R is still executing a command (resulting
   * from a call to RExecuteAsync())
   */
   BOOL MT4_EXPFUNC RIsBusy()
   {
	   LOGCALL();
	   return false;
   }

   /**
   * execute code and do not wait. Any subsequent call however
   * will wait since there can only be one thread executing at
   * any given time. Use RIsBusy() to check whether it is finished
   */
   void MT4_EXPFUNC RExecuteAsync(char* code)
   {
	   LOGCALL();

	   try
	   {
		   RInside::instance().parseEvalQ(code);
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

   /**
   * execute code and wait until it is finished. This will not
   * return anything. You can basically achieve the same with
   * the RGet*() functions, evaluating the expression is also
   * just executig code, the only difference is that these
   * RGet*() functions will additionally try to parse and return
   * the output while RExecute() will just execute, wait and
   * ignore all output.
   */
   void MT4_EXPFUNC RExecute(char* code)
   {
	   LOGCALL();

	   try
	   {
		   RInside::instance().parseEvalQ(code);
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

   /**
   * assign a BOOL to the variable name. In R this type is called "logical"
   */
   void MT4_EXPFUNC RAssignBOOL(char* variable, BOOL value)
   {
	   LOGCALL();

	   try
	   {
		   RInside::instance()[variable] = bool(value);
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

   /**
   * assign an integer to the variable name.
   */
   void MT4_EXPFUNC RAssignInteger(char* variable, int value)
   {
	   LOGCALL();

	   try
	   {
		   RInside::instance()[variable] = value;
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

   /**
   * assign a double to the variable name.
   */
   void MT4_EXPFUNC RAssignDouble(char* variable, double value)
   {
	   LOGCALL();
	   try
	   {
		   RInside::instance()[variable] = value;
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

   /**
   * assign a string to the variable namd. In R this type is called "character"
   */
   void MT4_EXPFUNC RAssignString(char* variable, char* value)
   {
	   LOGCALL();
	   try
	   {
		   RInside::instance()[variable] = std::string(value);
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

   /**
   * assign a vector to the variable name. If the size does not match
   * your actual array size then bad things might happen.
   */
   void MT4_EXPFUNC RAssignVector(char* variable, double* vector, int size)
   {
	   LOGCALL();

	   try
	   {
		   const std::vector<double> values(vector, &vector[size]);

		   RInside::instance()[variable] = values;
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

   /**
   * assign a vector of character (an array of strings) to the variable. If you need
   * a factor then you should execute code to convert it after this command. In
   * recent versions of R a vector of strings does not need any more memory than
   * a factor and it is easier to append new elements to it.
   */
   void MT4_EXPFUNC RAssignStringVector(char* variable, char* vector[], int size)
   {
	   LOGCALL();

	   try
	   {
		   std::vector<std::string> value;
		   value.reserve(size);

		   for(int i=0;i<size;++i)
		   {
			   value.push_back(vector[i]);
		   }

		   RInside::instance()[variable] = value;

	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

   /**
   * assign a matrix to the variable name. The matrix must have the row number as the
   * first dimension (byrow=TRUE will be used on the raw data). This function is much
   * faster than building a huge matrix (hundreds of rows) from scratch by appending
   * new rows at the end with RRowBindVector() for every row. This function is optimized
   * for huge throughput with a single function call through using file-IO with the
   * raw binary data. For very small matrices and vectors with only a handful of elements
   * this might be too much overhead and the other functions will be faster. Once you
   * have the matrix with possibly thousands of rows transferred to R you should then
   * only use RRowBindVector() to further grow it slowly on the arrival of single new
   * data vectors instead of always sending a new copy of the entire matrix.
   */
   void MT4_EXPFUNC RAssignMatrix(char* variable, double* matrix, int rows, int cols)
   {
	   try
	   {
		   Rcpp::NumericMatrix mat(rows,cols);

		   for(int i=0;i<rows;++i)
		   {
			   for(int j=0;j<cols;++j)
			   {
				   int index = j + i * cols;

				   mat(i,j) = matrix[index];
			   }
		   }
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }

   }

   /**
   * append a row to a matrix or dataframe. This will exexute
   * variable <- rbind(variable, vector)
   * if the size does not match the actual array size bad things might happen.
   */
   void MT4_EXPFUNC RAppendMatrixRow(char* variable, double* vector, int size)
   {
	   LOGCALL();

	   try
	   {
		   Rcpp::NumericMatrix mat = RInside::instance()[variable];
		   Rcpp::NumericMatrix row();
		   //std::vector<double> vec(vector,&vector[size]);
		   //mat = mat + row;
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }

   }

   /**
   * return true if the variable exists, false otherwise.
   */
   BOOL MT4_EXPFUNC RExists(char* variable)
   {
	   LOGCALL();
	   try
	   {
		   return RInside::instance()[variable].exists();
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

   /**
   * evaluate expression and return a BOOL. Expression can be any R code
   * that will evaluate to logical. If it is a vector of logical then only
   * the first element is returned.
   */
   BOOL MT4_EXPFUNC RGetBool(char* expression)
   {
	   LOGCALL();
	   try
	   {
		   return RInside::instance().parseEval(expression);
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

   /**
   * evaluate expression and return an integer. Expression can be any R code
   * that will evaluate to an integer. If it is a floating point it will be
   * rounded, if it is a vector then only the first element will be returned.
   */
   int MT4_EXPFUNC RGetInteger(char* expression)
   {
	   LOGCALL();

	   try
	   {
		   return RInside::instance().parseEval(expression);
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

   /**
   * evaluate expression and return a double. Expression can be any R code
   * that will evaluate to a floating point number, if it is a vector then
   * only the first element is returned.
   */
   double MT4_EXPFUNC RGetDouble(char* expression)
   {
	   LOGCALL();
	   try
	   {
		   return RInside::instance().parseEval(expression);
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

   /**
   * evaluate expression and return a vector of doubles. Expression can
   * be anything that evaluates to a vector of floating point numbers.
   * Return value is the number of elements that could be copied into the
   * array. It will never be bigger than size but might be smaller.
   * warnings are output on debuglevel 1 if the sizes don't match.
   */
   int MT4_EXPFUNC RGetVector(char* expression, double* vector, int size)
   {
	   LOGCALL();

	   try
	   {
		   std::vector<double> values = RInside::instance().parseEval(expression);
		   std::copy(values.begin(),values.end(),vector);
		   return values.size();
	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

   int MT4_EXPFUNC RGetXTS(char* expression,RateInfo* rates,const int rates_total)
   {
	   try
	   {
		   FinSeries::Xts series = RInside::instance().parseEval(expression);

		   int fetch_num = std::min((size_t)rates_total,series.nrows());

		   for(int i=0;i<fetch_num;++i)
		   {
			   rates[i].open = series(i,0);
			   rates[i].high = series(i,1);
			   rates[i].low = series(i,2);
			   rates[i].close = series(i,3);
			   rates[i].vol = series(i,4);
			   rates[i].ctm = series.index(i);
		   }

		   return fetch_num;

	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }


   /**
   * do a print(expression) for debugging purposes. The outout will be
   * sent to the debug monitor on debuglevel 0.
   */
   void MT4_EXPFUNC RPrint(char* expression)
   {
	   logger::instance() << expression << std::endl;
   }

   void MT4_EXPFUNC RAssignXTS(char* expression,const RateInfo* rates,const int rates_total)
   {
	   LOGCALL();

	   try
	   {
		   std::vector<double> 			opens;
		   std::vector<double> 			highs;
		   std::vector<double> 			lows;
		   std::vector<double> 			closes;
		   std::vector<double> 			volumes;
		   std::vector<Rcpp::Datetime> 	times;

		   opens.resize(rates_total);
		   highs.resize(rates_total);
		   lows.resize(rates_total);
		   closes.resize(rates_total);
		   volumes.resize(rates_total);
		   times.resize(rates_total);

		   logger::instance() << "Assigning " << rates_total << " candles to XTS object " << expression << std::endl;

		   for(int i=0;i<rates_total;++i)
		   {
			   opens[i] = rates[i].open;
			   highs[i] = rates[i].high;
			   lows[i] = rates[i].low;
			   closes[i] = rates[i].close;
			   volumes[i] = rates[i].vol;
			   times[i] = rates[i].ctm;
		   }

		   FinSeries::Xts series(opens,highs,lows,closes,volumes,times);

		   RInside::instance()[expression] = series;

	   } catch(std::exception& ex) {
		   logger::instance() << __FUNCTION__ << "Exception caught: " << ex.what() << std::endl;
		   throw;
	   } catch(...) {
		   logger::instance() << __FUNCTION__ << "Unknown exception caught" << std::endl;
		   throw;
	   }
   }

}
