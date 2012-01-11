#property copyright "© 2011 James Bates"
#define MQL2R_VERSION_MAJOR 1
#define MQL2R_VERSION_MINOR 3  // must change to 4

#import "kernel32.dll"
   int GetEnvironmentVariableA(string lpName, int& lpBuffer[], int nSize);
   int SetEnvironmentVariableA(string name,string value);
   int CreateProcessA(int    lpApplicationName,
                   string lpCommandLine,
                   int    lpProcessAttributes,
                   int    lpThreadAttributes,
                   int    bInheritHandles,
                   int    dwCreationFlags,
                   int    lpEnvironment,
                   int    lpCurrentDirectory,
                   int    &lpStartupInfo[],
                   int    &lpProcessInformation[]);
   int CreatePipe(int &hReadPipe[],int &hWritePipe[],int &lpPipeAttributes[],int nSize);
   int SetHandleInformation(int hObject,int dwMask,int dwFlags);
   int WriteFile( int fileHandle, string buffer, int bytes, int& numOfBytes[], int overlapped );
   int ReadFile( int fileHandle, int& buffer[], int bytes, int& numOfBytes[], int overlapped );
   int CloseHandle( int handle);
#import

#define SW_HIDE             0
#define SW_SHOWNORMAL       1
#define SW_NORMAL           1
#define SW_SHOWMINIMIZED    2
#define SW_SHOWMAXIMIZED    3
#define SW_MAXIMIZE         3
#define SW_SHOWNOACTIVATE   4
#define SW_SHOW             5
#define SW_MINIMIZE         6
#define SW_SHOWMINNOACTIVE  7
#define SW_SHOWNA           8
#define SW_RESTORE          9
#define SW_SHOWDEFAULT      10
#define SW_FORCEMINIMIZE    11
#define SW_MAX              11

#import "shell32.dll"
   int ShellExecuteA(int hWnd,int lpVerb,string lpFile,int lpParameters,int lpDirectory,int nCmdShow);
#import

#import "libMQL2R.dll"
   /**
   * Return the dll version. The upper 16 bit of the return value
   * are the major version and the lower 16 bit the minor. This
   * is used in RInit() to make sure that this header file and 
   * the dll fit together.
   */ 
   int RGetDllVersion();
   
   /**
   * This is not meant to be called directly, it will be
   * called by RInit() after the successful version check. 
   * You should call RInit() to start a new R session.
   */
   int RInit_(string commandline, int debuglevel);
   
   /**
   * Teminate the R session. Call this in your deinit() function.
   * After this the handle is no longer valid.
   */
   void RDeinit(int handle);
   
   /**
   * return true if the R session belonging to this handle is 
   * still runing. R will terminate on any fatal error in the 
   * code you send it. You should check this at the beginning
   * of your start function and stop all actions. The last
   * command prior to the crash will be found in the log.
   * If R is not running anymore this library won't emit any
   * more log messages and will silently ignore all commands.
   */
   bool RIsRunning();
   
   
   /**
   * return true if R is still executing a command (resulting 
   * from a call to RExecuteAsync())
   */
   bool RIsBusy();
   
   /**
   * execute code and do not wait. Any subsequent call however
   * will wait since there can only be one thread executing at
   * any given time. Use RIsBusy() to check whether it is finished
   */
   void RExecuteAsync(string code);
   
   /**
   * execute code and wait until it is finished. This will not
   * return anything. You can basically achieve the same with
   * the RGet*() functions, evaluating the expression is also
   * just executig code, the only difference is that these
   * RGet*() functions will additionally try to parse and return 
   * the output while RExecute() will just execute, wait and 
   * ignore all output.
   */
   void RExecute(string code);
   
   /**
   * assign a bool to the variable name. In R this type is called "logical"
   */
   void RAssignBool(string variable, bool value);
   
   /**
   * assign an integer to the variable name.
   */
   void RAssignInteger(string variable, int value);
   
   /**
   * assign a double to the variable name.
   */
   void RAssignDouble(string variable, double value);
   
   /**
   * assign a string to the variable namd. In R this type is called "character"
   */
   void RAssignString(string variable, string value);
   
   /** 
   * assign a vector to the variable name. If the size does not match
   * your actual array size then bad things might happen.
   */
   void RAssignVector(string variable, double &vector[], int size);
  
     /** 
   * assign a vector to the variable name. If the size does not match
   * your actual array size then bad things might happen.
   */
   void RAssignXTS(string variable, double rates[][6], int size);
   
   /**
   * assign a vector of character (an array of strings) to the variable. If you need
   * a factor then you should execute code to convert it after this command. In
   * recent versions of R a vector of strings does not need any more memory than
   * a factor and it is easier to append new elements to it.
   */ 
   void RAssignStringVector(string variable, string &vector[], int size);
   
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
   void RAssignMatrix(string variable, double &matrix[][], int rows, int cols);
   
   /** 
   * append a row to a matrix or dataframe. This will exexute 
   * variable <- rbind(variable, vector)
   * if the size does not match the actual array size bad things might happen.
   */
   void RAppendMatrixRow(string variable, double &vector[], int size);
   
   /**
   * return true if the variable exists, false otherwise.
   */
   bool RExists(string variable);
   
   /**
   * evaluate expression and return a bool. Expression can be any R code 
   * that will evaluate to logical. If it is a vector of logical then only
   * the first element is returned.
   */
   bool RGetBool(string expression);
   
   /**
   * evaluate expression and return an integer. Expression can be any R code 
   * that will evaluate to an integer. If it is a floating point it will be
   * rounded, if it is a vector then only the first element will be returned.
   */
   int RGetInteger(string expression);
   
   /**
   * evaluate expression and return a double. Expression can be any R code 
   * that will evaluate to a floating point number, if it is a vector then
   * only the first element is returned.
   */
   double RGetDouble(string expression);
   
   /**
   * evaluate expression and return a vector of doubles. Expression can
   * be anything that evaluates to a vector of floating point numbers.
   * Return value is the number of elements that could be copied into the
   * array. It will never be bigger than size but might be smaller.
   * warnings are output on debuglevel 1 if the sizes don't match.
   */
   int RGetVector(string expression, double &vector[], int size);
   
   int RGetXTS(string variable, double rates[][6], int size);
   
   /**
   * do a print(expression) for debugging purposes. The outout will be 
   * sent to the debug monitor on debuglevel 0.
   */
   void RPrint(string expression);
#import

string initString(int len)
{
   string rval;
   int i;
   for(i=0;i<len;i++)
   {
      rval = rval + ".";
   }
   
   return(rval);
}

string bufferToString(int buffer[])
   {
   string text="";
   
   for (int i=0; i<ArraySize(buffer); i++)
      {
      int curr = buffer[i];
      text = text + CharToStr(curr & 0x000000FF)
         +CharToStr(curr >> 8 & 0x000000FF)
         +CharToStr(curr >> 16 & 0x000000FF)
         +CharToStr(curr >> 24 & 0x000000FF);
      }
   return (text);
   }     
   
//+------------------------------------------------------------------+
string StringReplace(string str, string str1, string str2)  {
//+------------------------------------------------------------------+
// Usage: replaces every occurrence of str1 with str2 in str
  string outstr = "";
  for (int i=0; i<StringLen(str); i++)   {
    if (StringSubstr(str,i,StringLen(str1)) == str1)  {
      outstr = outstr + str2;
      i += StringLen(str1) - 1;
    }
    else
      outstr = outstr + StringSubstr(str,i,1);
  }
  return(outstr);
}   
   
string GetEnvironmentVariable(string name)
{
   int buffer[1024];
   string rval;
   int result;
   ArrayInitialize(buffer,0);
   result = GetEnvironmentVariableA(name,buffer,ArraySize(buffer)*4);
   rval = bufferToString(buffer);
   
   //Print(name+"="+rval);
   
   return(rval);
}

string CmdExe(string lpCommandLine)
{
   int /*SECURITY_ATTRIBUTES*/ sa[ 3];
   int /*STARTUPINFO*/         si[17];
   int /*PROCESS_INFORMATION*/ pi[ 4];

   ArrayInitialize(sa,0);
   ArrayInitialize(si,0);
   ArrayInitialize(pi,0);

   //Set up structs;
   sa[0] = 12;
   sa[1] = 0; //TRUE
   sa[2] = 1;

   int hChildStdoutRd[1];
   int hChildStdoutWr[1]; 
   int hChildStdinRd[1];
   int hChildStdinWr[1]; 

   int result=1;
   //Print("hChildStdoutRd="+hChildStdoutRd[0]+",hChildStdoutWr="+hChildStdoutWr[0]+",hChildStdinRd="+hChildStdinRd[0]+",hChildStdinWr="+hChildStdinWr[0]);
   
   
   result = CreatePipe(hChildStdoutRd,hChildStdoutWr,sa,0);
   if(result == 0)
   {
      Print("Failed to create child process stdout pipe");
   }
   
   result = SetHandleInformation( hChildStdoutRd[0], 1, 0); //SetHandleInformation( hChildStdoutRd, HANDLE_FLAG_INHERIT, 0);
   if(result == 0)
   {
      Print("Failed to SetHandleInformation on hChildStdoutRd");
   }
   
   
   result = CreatePipe(hChildStdinRd,hChildStdinWr,sa,0);
   if(result == 0)
   {
      Print("Failed to create child process stdin pipe");
   }
   result = SetHandleInformation( hChildStdinWr[0], 1, 0); //SetHandleInformation( hChildStdinWr, HANDLE_FLAG_INHERIT, 0);
   if(result == 0)
   {
      Print("Failed to SetHandleInformation on hChildStdinWr");
   }

   //Print("hChildStdoutRd="+hChildStdoutRd[0]+",hChildStdoutWr="+hChildStdoutWr[0]+",hChildStdinRd="+hChildStdinRd[0]+",hChildStdinWr="+hChildStdinWr[0]);


   /*
      sa.cb = sizeof(STARTUPINFO); 
      sa.hStdError = hChildStdoutWr;
      sa.hStdOutput = hChildStdoutWr;
      sa.hStdInput = hChildStdinRd;
      sa.dwFlags |= STARTF_USESTDHANDLES;
   */

   si[0] = 68;
   si[16] = hChildStdoutWr[0];
   si[15] = hChildStdoutWr[0];
   si[14] = hChildStdinRd[0];
   si[11] = 0x00000100; //STARTF_USESTDHANDLES 
   

   result = CreateProcessA(0,            // module name
                               lpCommandLine,   // command line
                               0,              // process attributes
                               0,              // thread attributes
                               1,            // handle inheritance
                               0x08000200,      // creation flags CREATE_NO_WINDOW|CREATE_NEW_PROCESS_GROUP
                               0,            // environment block
                               0,            // starting directory
                               si,              // startup info
                               pi               // process info
                               );
   if(result == 0)
   {
      Print("Failed to create child process");
   }                            
    
   int buffer[1024]; 
   ArrayInitialize(buffer,0);
   int totalBytesRead = 0;
   
   int bufferSize = ArraySize(buffer) * 4;
   bool readFinished = false;
   string rval="";
   int bytesRead[1];
   
   int bSuccess = 0;
   
   // Close the write end of the pipe before reading from the 
	// read end of the pipe, to control child process execution.
	// The pipe is assumed to have enough buffer space to hold the
	// data the child process has already written to it.
   
   if (!CloseHandle(hChildStdoutWr[0])) 
		Print("Error in CloseHandler(hChildStdoutWr[0])"); 
   
   for (;;) 
	{ 
		bSuccess = ReadFile(hChildStdoutRd[0],buffer,bufferSize,bytesRead,0);

		if( ! bSuccess || bytesRead[0] == 0 )
		{ 
		    break; 
		}

      rval = rval + bufferToString(buffer); 
      totalBytesRead += bytesRead[0];      
        
	} 
   
   return (rval);
}

string EscapeQuotes(string value)
{
   return(StringReplace(value,"\"","\\\""));
}

string RProcCmd(string cmd)
{
   string fullCommand = "\"" + GetEnvironmentVariable("R_HOME") + "\\bin\\i386\\RScript.exe\"" + " -e \"" + EscapeQuotes(cmd) + "\"";
   string rval="";
   
   Print("MQL2R:     ",cmd);
   
   rval = CmdExe(fullCommand);

   return(rval);
}

string AddToPath(string path,string newEntry)
{
   if(StringFind(path,newEntry) == -1)
   {
      return(path + ";" + newEntry);
   }
   
   return(path);
}

/*
* start and initialize a new R session. Call this function in init() and store 
* the handle it returns. This will start an R session and all subsequent calls 
* to R functions will need this handle to identify the R session. This function
* will check the version of the dll against the version of this header file and
* if there is a mismatch it will report an error and refuse to initialize R.
*/
int RInit(string commandline, int debuglevel){
   int dll_version;
   int dll_major;
   int dll_minor;
   string error,message;
   string path;
   string r_home;
   
   path = GetEnvironmentVariable("PATH");
   r_home = GetEnvironmentVariable("R_HOME");
   
   string getMQL2RInstallPathCmd = "cat(tools:::file_path_as_absolute( system.file( \"libs\", package = \"MQL2R\" )))";
   
   string MQL2RLIBS;
   
   MQL2RLIBS = RProcCmd(getMQL2RInstallPathCmd);
   
   if(MQL2RLIBS == "")
   {
      Print("Could not locate R library MQL2R is it installed?");
      return(0);
   }
   else
   {
      MQL2RLIBS = StringReplace(MQL2RLIBS,"/","\\");
   
      Print("MQL2R library found at path: ",MQL2RLIBS);
   }
   
   path = AddToPath(path,MQL2RLIBS + "\\i386;");
   path = AddToPath(path,r_home + "\\bin\\i386");
   
   Print("Setting new PATH env var to: ",path);
   
   SetEnvironmentVariableA("PATH",r_home + "\bin\i386;" + path);
   
   dll_version = RGetDllVersion();
   dll_major = dll_version >> 16;
   dll_minor = dll_version & 0xffff;
   
   if (dll_version == MQL2R_VERSION_MAJOR << 16 + MQL2R_VERSION_MINOR){
         message = "Version match mql2R.dll: " + "  -  found dll version " + dll_major + "." + dll_minor;
            
      Print(message);      
      return(RInit_(commandline, debuglevel));
   }else{
      error = "Version mismatch MQL2R.dll: "
            + "expected version " + MQL2R_VERSION_MAJOR + "." + MQL2R_VERSION_MINOR
            + "  -  found dll version " + dll_major + "." + dll_minor;
      Print(error);
      return(0);
   }
   
   
}


/**
* shorthands for some of the above functions
*/

int hR;

void StartR(string path, int debug=1){
   hR = RInit(path, debug);
}

void StopR(){
   RDeinit(hR);
}

void Rx(string code){
   RExecute(code);
}

void Rs(string var, string s){
   RAssignString(var, s);
}

void Ri(string var, int i){
   RAssignInteger(var, i);
}

void Rd(string var, double d){
   RAssignDouble(var, d);
}

void Rv(string var, double v[]){
   RAssignVector(var, v, ArraySize(v));
}

void Rf(string name, string factor[]){
   RAssignStringVector(name, factor, ArraySize(factor));
   Rx(name + " <- as.factor(" + name + ")");
}

void Rm(string var, double matrix[], int rows, int cols){
   RAssignMatrix(var, matrix, rows, cols);
}

int Rgi(string var){
   return(RGetInteger(var));
}

double Rgd(string var){
   return(RGetDouble(var));
}

void Rgv(string var, double &v[]){
   RGetVector(var, v, ArraySize(v));
}

void Rp(string expression){
   RPrint(expression);
}


