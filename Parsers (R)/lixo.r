#=============================================
#   Plotting RBs and SINR in Time    
#
#   Marcio Teixeira                         
#   FT - Unicamp
#   04-Oct-2018
#---------------------------------------------

library(stringr)
library(dplyr)
library(tidyr)
library(data.table)
setwd("~/LTESim/")

#fileN <- paste("lixoso",n,".dat",sep = "")

fileN <- "RBsSinr.dat"
df <- read.delim(file = fileN,header= TRUE, sep = "\t")
dtf <- reshape(df, idvar="Time", timevar="RB",direction = "wide")


############ PLOT EXAMPLE

plot(dtf$Time,dtf$Sinr.0, type = "p", col = 'blue', pch = 19, cex=0.6,
   xlim = c(100.1,100.2), 
    #ylim = c(1,6), 
      xlab= "Time", ylab = "SINR", main = "SINR at RB0")
# lines(dtf$index,dtf$RBs.20, col = 'red')

