
#include <RInside.h>                    // for the embedded R via RInside
#include <Rembedded.h>
#include <RDefines.h>
#include <Rinternals.h>
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <string>
#include <iostream>

#define MT4_EXPFUNC __declspec(dllexport) __stdcall
//__stdcall

#pragma pack(push,1)
struct RateInfo
  {
   unsigned int      ctm;
   double            open;
   double            low;
   double            high;
   double            close;
   double            vol;
  };
#pragma pack(pop)
//----
struct MqlStr
  {
   int               len;
   char             *string;

   operator std::string()
   {
	   return std::string(string,len);
   }

  };

std::ostream& operator << (std::ostream& ostr,MqlStr* str);

extern "C" {

  /**
   * Return the dll version. The upper 16 bit of the return value
   * are the major version and the lower 16 bit the minor. This
   * is used in RInit() to make sure that this header file and
   * the dll fit together.
   */
   int MT4_EXPFUNC RGetDllVersion();

   /**
   * This is not meant to be called directly, it will be
   * called by RInit() after the successful version check.
   * You should call RInit() to start a new R session.
   */
   int MT4_EXPFUNC RInit_(char* commandline, int debuglevel);

   /**
   * Teminate the R session. Call this in your deinit() function.
   * After this the handle is no longer valid.
   */
   void MT4_EXPFUNC RDeinit(int handle);

   /**
   * return true if the R session belonging to this handle is
   * still runing. R will terminate on any fatal error in the
   * code you send it. You should check this at the beginning
   * of your start function and stop all actions. The last
   * command prior to the crash will be found in the log.
   * If R is not running anymore this library won't emit any
   * more log messages and will silently ignore all commands.
   */
   BOOL MT4_EXPFUNC RIsRunning();


   /**
   * return true if R is still executing a command (resulting
   * from a call to RExecuteAsync())
   */
   BOOL MT4_EXPFUNC RIsBusy();

   /**
   * execute code and do not wait. Any subsequent call however
   * will wait since there can only be one thread executing at
   * any given time. Use RIsBusy() to check whether it is finished
   */
   void MT4_EXPFUNC RExecuteAsync(char* code);

   /**
   * execute code and wait until it is finished. This will not
   * return anything. You can basically achieve the same with
   * the RGet*() functions, evaluating the expression is also
   * just executig code, the only difference is that these
   * RGet*() functions will additionally try to parse and return
   * the output while RExecute() will just execute, wait and
   * ignore all output.
   */
   void MT4_EXPFUNC RExecute(char* code);

   /**
   * assign a BOOL to the variable name. In R this type is called "logical"
   */
   void MT4_EXPFUNC RAssignBOOL(char* variable, BOOL value);

   /**
   * assign an integer to the variable name.
   */
   void MT4_EXPFUNC RAssignInteger(char* variable, int value);

   /**
   * assign a double to the variable name.
   */
   void MT4_EXPFUNC RAssignDouble(char* variable, double value);

   /**
   * assign a string to the variable namd. In R this type is called "character"
   */
   void MT4_EXPFUNC RAssignString(char* variable, char* value);

   /**
   * assign a vector to the variable name. If the size does not match
   * your actual array size then bad things might happen.
   */
   void MT4_EXPFUNC RAssignVector(char* variable, double* vector, int size);

   void MT4_EXPFUNC RAssignXTS(char* expression,const RateInfo* rates,const int rates_total);

   /**
   * assign a vector of character (an array of strings) to the variable. If you need
   * a factor then you should execute code to convert it after this command. In
   * recent versions of R a vector of strings does not need any more memory than
   * a factor and it is easier to append new elements to it.
   */
   void MT4_EXPFUNC RAssignStringVector(char* variable, char* vector[], int size);

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
   void MT4_EXPFUNC RAssignMatrix(char* variable, double* matrix, int rows, int cols);

   /**
   * append a row to a matrix or dataframe. This will exexute
   * variable <- rbind(variable, vector)
   * if the size does not match the actual array size bad things might happen.
   */
   void MT4_EXPFUNC RAppendMatrixRow(char* variable, double* vector, int size);

   /**
   * return true if the variable exists, false otherwise.
   */
   BOOL MT4_EXPFUNC RExists(char* variable);

   /**
   * evaluate expression and return a BOOL. Expression can be any R code
   * that will evaluate to logical. If it is a vector of logical then only
   * the first element is returned.
   */
   BOOL MT4_EXPFUNC RGetBool(char* expression);

   /**
   * evaluate expression and return an integer. Expression can be any R code
   * that will evaluate to an integer. If it is a floating point it will be
   * rounded, if it is a vector then only the first element will be returned.
   */
   int MT4_EXPFUNC RGetInteger(char* expression);

   /**
   * evaluate expression and return a double. Expression can be any R code
   * that will evaluate to a floating point number, if it is a vector then
   * only the first element is returned.
   */
   double MT4_EXPFUNC RGetDouble(char* expression);

   /**
   * evaluate expression and return a vector of doubles. Expression can
   * be anything that evaluates to a vector of floating point numbers.
   * Return value is the number of elements that could be copied into the
   * array. It will never be bigger than size but might be smaller.
   * warnings are output on debuglevel 1 if the sizes don't match.
   */
   int MT4_EXPFUNC RGetVector(char* expression, double* vector, int size);


   int MT4_EXPFUNC RGetXTS(char* expression,RateInfo* rates,const int rates_total);


   /**
   * do a print(expression) for debugging purposes. The outout will be
   * sent to the debug monitor on debuglevel 0.
   */
   void MT4_EXPFUNC RPrint(char* expression);


}
