#=============================================
#   Parsing output files from Simulation   
#   (Flows in Time, per RB)
#
#   Marcio Teixeira                         
#   FT - Unicamp
#   29-Sep-2018
#---------------------------------------------

library(stringr)
library(dplyr)
library(tidyr)
setwd("~/LTESim/")
# import data

for(n in 1:5){

fileN <- paste("lixoso",n,".dat",sep = "")
allText <- readLines(fileN)

# selecting flows per bearer type

elem1 <- grep("assigned to the", allText)
elem2 <- grep("Start SubFrame", allText)

# selecting rows with info on flow selected and number of packets

#sFlow <- strsplit(as.character(allText[sort(c(elem1,elem2))]), ' ')
#sFlow1 <- strsplit(as.character(allText[sort(c(elem1))]), ' ')
#sFlow2 <- strsplit(as.character(allText[sort(c(elem2))]), ' ')
#x <- allText[sort(c(elem1,elem2))]

#rm(allText) # free some memory space
# transform in a table 
# TO DO: Include column names!!!!

index <- sort(c(elem1,elem2))
x = allText[index]
df <- data.frame(x)
dfSplit <- df %>% separate(x, c(letters[1:10]),sep = " ",convert = TRUE, fill = "right")

# clean up the mess
rm(df,allText,elem1,elem2,x)

#All RBs vs Frames
cara <- subset(dfSplit,!is.na(dfSplit$j),select = c(d,j))

#Just a selected RB

  cara2 <- subset(cara,(cara$d==n),select = c(d,j))
  
  #Plot
  plot(cara2$j, xlab = "Time (ms)", ylab = "Flow ID",main = c("RB",n), 
       type = "p", cex = 1.0,xlim = c(2000,2500),ylim=c(0,10) )
  lines(cara2$j)
  
}



# to do
# 
# 1. parse RB per frame (each 10ms). Just multiply by 100 and do a MOD
# 2. include more flows (e.g per mobile) and plot them together


