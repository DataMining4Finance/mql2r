#property copyright "© James Bates"


#property indicator_chart_window

#property indicator_buffers 1
#property indicator_color1 Red
#property indicator_width1 2


#include <mql2R.mqh>

extern int order = 200;
extern int back = 500;
extern int ahead = 0;
extern int StartFrom = 0;
extern string RIndicatorScript = "R\EMA.R";


int R;
double buf_indicator[];

bool scriptLoaded = false;
int scriptWriteTime[2] = {0,0};

bool IsScriptLoadRequired()
{
   if(!scriptLoaded)
      return(true);
      
   int newScriptWriteTime[2];
   
   GetFileWriteTime(RIndicatorScript,newScriptWriteTime);
   if(CompareFileTime(scriptWriteTime,newScriptWriteTime) == -1)
   {
      Print("R script update detected re-loading: ",RIndicatorScript);
      scriptLoaded = LoadRScript(RIndicatorScript);
      ArrayCopy(scriptWriteTime,newScriptWriteTime);
   }    

}

int init(){
   SetIndexBuffer(0, buf_indicator);
   SetIndexStyle(0, DRAW_LINE);
   SetIndexShift(0, ahead-StartFrom);
  
   R = RInit("", 2);
   Comment("history: " + back + " bars, method: OLS, order: " + order);
   Comment("RScript: " + RIndicatorScript);
   
   if(IsScriptLoadRequired())
      scriptLoaded = LoadRScript(RIndicatorScript);
   
   if(!scriptLoaded)
      return(1);
      
   RExecute("library(xts)");
   RExecute("init("+Symbol()+","+Period()+")");
   
   return(0);
}

int deinit(){
  // RDeinit(R);
   if(scriptLoaded)  
      RExecute("deinit()");
   
   return(0);
}



int start(){
   int rval=0;
   double hist[];
   double pred[];
   int i;


   if(IsScriptLoadRequired())
      scriptLoaded = LoadRScript(RIndicatorScript);
  
   if(!scriptLoaded)
      return(1);

     
      
   double rates[][6];
   ArrayCopyRates(rates);
      
   string ratesName = "Rates_"+Symbol()+"_"+Period();
   string indicatorsName = "Indicators_"+Symbol()+"_"+Period();
   
   Print("Assigning rates " + ratesName + " = " + Bars);
   
   RAssignXTS_(ratesName, rates, Bars);
     
   Print("Num assigned rates = " + Bars);
   
   RExecute(indicatorsName + " <- xts()");
   
   string startCommand = indicatorsName + " <- start("+Symbol()+","+Period()+","+ratesName+")";
   
   Print("Calling " + startCommand);
   
   RExecute(startCommand);
   
   Print("OK");
   
   double indicators[];
   ArraySetAsSeries(indicators,true);
   /*
   if(!RExecute("class("+indicatorsName+")"))
      return(-1);
   */
   int xtsRows = Bars,xtsCols = 1;

   Print("Fetching indicator values");

   datetime times[];
   ArrayResize(times,Bars); 
   ArrayResize(indicators,Bars);  

   rval = RGetVector(indicatorsName,indicators,xtsRows);
   //rval = RGetXTS(indicatorsName, indicators, times,xtsRows,xtsCols);

   for (i=0; i<xtsRows; i++){
      buf_indicator[i] = indicators[i + (Bars-xtsRows)];
   }
   
   Print(xtsRows+" Indicator values retrieved");

   RExecute("rm("+ratesName+")");
   RExecute("rm("+indicatorsName+")");
  
   return(0);   
}