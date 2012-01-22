
library(MQL2R)
library(quantmod)



init <- function(symbol,period)
{
  
}

deinit <- function()
{
   
}

start <- function(symbol,period,rates)
{
 # period <- 25
  
  rval <- EMA(rates,25)

  rval[25:length(rval),]
  
}