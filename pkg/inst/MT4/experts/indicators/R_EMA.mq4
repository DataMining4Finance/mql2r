#property copyright "© James Bates"


#property indicator_chart_window

#property indicator_buffers 1
#property indicator_color1 Red
#property indicator_width1 2


#include <mql2R.mqh>

int R;
double buf_indicator[];


int init(){
   SetIndexBuffer(0, buf_indicator);
   SetIndexStyle(0, DRAW_LINE);
  
   R = RInit("", 2);
   RExecute("library(quantmod)");
   RExecute("library(xts)");
   RExecute("library(TTR)");
   
   return(0);
}

int deinit(){ 
   return(0);
}



int start(){
   int rval=0;
   double hist[];
   double pred[];
   int i;

   double rates[][6];
   ArrayCopyRates(rates);
      
   RExecute("library(quantmod)");
   RExecute("library(xts)");
   RExecute("library(TTR)");      
   RAssignXTS_("rates", rates, Bars);
     
   Print("Num assigned rates = " + Bars);
   RExecute("indicator <- xts()");
   RExecute("indicator <- EMA(Cl(rates),300)");
   
   double indicator[];
   ArraySetAsSeries(indicator,true);

   int xtsRows = Bars,xtsCols = 1;

   Print("Fetching indicator values");

   datetime times[];
   ArrayResize(times,Bars); 
   ArrayResize(indicator,Bars);  

   rval = RGetVector("indicator",indicator,xtsRows);
   //rval = RGetXTS("indicator", indicator, times,xtsRows,xtsCols);

   for (i=0; i<xtsRows; i++){
      buf_indicator[i] = indicator[i + (Bars-xtsRows)];
      //Print(indicator[i + (Bars-xtsRows)]);
   }
   
   //RExecute("print rates");
   //RExecute("chartSeries(rates,type='candles')");
   //RExecute("addTA(indicator,col=4)");
   
   RExecute("rm(rates)");
   RExecute("rm(indicator)");
  
   return(0);   
}