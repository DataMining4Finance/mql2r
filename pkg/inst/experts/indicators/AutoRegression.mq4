#property copyright "© Bernd Kreuss"


#property indicator_chart_window

#property indicator_buffers 1
#property indicator_color1 Red
#property indicator_width1 2


#include <mql2R.mqh>

extern int order = 200;
extern int back = 500;
extern int ahead = 20;
extern int StartFrom = 5;


int R;
double buf_prediction[];

int init(){
   SetIndexBuffer(0, buf_prediction);
   SetIndexStyle(0, DRAW_LINE);
   SetIndexShift(0, ahead-StartFrom);
  
   R = RInit("", 2);
   Comment("history: " + back + " bars, method: OLS, order: " + order);
}

int deinit(){
  // RDeinit(R);
}

int start(){
   double hist[];
   double pred[];
   int i;

   if (RIsBusy()){
      // last RExecuteAsync() is still not finished, do nothing.
      Print("R Is Busy");
      return(0);
   }
   
   if (RGetInteger("as.integer(exists('model'))") == 1){
      // there exists a model (the variable is set). 
      // This means a previously started RExecuteAsync() has finished. 
      // we can now predict from this model and plot it.
      RAssignInteger("ahead", ahead);
      RExecute("pred <- predict(model, n.ahead=ahead)$pred");
      ArrayResize(pred, ahead);
      RGetVector("rev(pred)", pred, ahead);
      for (i=0; i<ahead; i++){
         buf_prediction[i] = pred[i];
      }
   }
   
   // make a (new) prediction
   // move some history over to R   
   ArrayResize(hist, back);
   for (i=0; i<back; i++){
      hist[i] = Close[i+StartFrom];
   }
   RAssignVector("hist", hist, ArraySize(hist));
   
   double rates[][6];
   ArrayCopyRates(rates);
   
   Print("Num assigned rates = " + Bars);
   
   RAssignXTS("histXts", rates, Bars);
   RExecute("hist <- rev(hist)");
   
   int r = RGetXTS("histXts", rates, Bars);
   
   //Print("Num returned rates = " + r);
   // crunch the numbers in the background and return from the start() function
   // RIsBusy() in the next ticks will tell us when it is finished.
   RAssignInteger("ord", order);
   RExecuteAsync("model <- ar(hist, aic=FALSE, order=ord, method='ols')");
   
   
   return(0);   
}