#=============================================
#   Parsing output files from Simulation    
#
#   Marcio Teixeira                         
#   FT - Unicamp
#   28-Sep-2018
#---------------------------------------------

library(stringr)
setwd("~/LTESim/")
# import data
allText <- as.list(readLines("singleCellDebugSched.txt"))

# selecting flows per bearer type

elem1 <- grep("assigned to the", allText)
elem2 <- grep("Start SubFrame", allText)
# selecting rows with info on flow selected and number of packets

#selecFlow <- strsplit(as.character(allText[sort(c(elem1,elem2))]), ' ')
sFlow1 <- strsplit(as.character(allText[sort(c(elem1))]), ' ')
sFlow2 <- strsplit(as.character(allText[sort(c(elem2))]), ' ')
#x <- allText[sort(c(elem1,elem2))]

rm(allText) # free some memory space
# transform in a table 
# TO DO: Include column names!!!!

f <- data.frame(matrix(unlist(sFlow1), nrow=length(sFlow1), byrow=T),stringsAsFactors=FALSE)

g <- data.frame(matrix(unlist(sFlow2), nrow=length(sFlow2), byrow=T),stringsAsFactors=FALSE)
# Now we subset only the rows with TX 

f <- f[f$X1=="TX",]

# Write table to file

write.csv2 (f$X14, "teste.csv")


# to do
# 
# 1. parse RB per frame (each 10ms). Just multipky by 100 and do a MOD
# 2. include more flows (e.g per mobile) and plot them together


