#=============================================
#   Plotting RBs and Frames in Time    
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

fileN <- "newSim.txt"
allText <- readLines(fileN)

# selecting flows per bearer type

index <-sort(c( grep("nb of RBs", allText), grep("--> flow ", allText), grep("effectiveSinr ", allText), 
           grep("Start SubFrame",allText),grep("effectiveSinr",allText)))

x = allText[index]

#x <- strsplit(as.matrix(allText[index]), ' ')


dt <- data.table(x)

dtSplit <- dt %>% separate(x, c(letters[1:10]),sep = " ",convert = TRUE, fill = "right")

rm(x,dt,allText)
#select all RBs and flows

###############################################################
#Detailed commands
#
# 
# piroca <- sapply(piroca, as.numeric)
# piroca[!is.na(piroca)] remove na
# 
##############################################################

#compact form
RBs <- sapply(ifelse(dtSplit$d=="RBs", dtSplit$e,NA),as.numeric)
flow <- sapply(ifelse(dtSplit$c=="flow",dtSplit$d,NA),as.numeric)

RBs <- RBs[!is.na(RBs)]
flow <- flow[!is.na(flow)]

rm(dtSplit)

dt <- data.table(flow,RBs)  # ORIGINALLY

#dt <- data.frame(flow,RBs)

dt[,index := seq(1,length(flow))] #MEGA CAGADA usando data table !!! NAO SE COMO MAS FUNCIONA
####### FILTERING ######

dtf<- dt %>% 
  select(index,flow,RBs) %>% 
  filter(flow >=20, flow<30)

# changes values in lines RB to coloumn

dtf <- reshape(dtf, idvar="index", timevar="flow",direction = "wide")
dtf[is.na(dtf)] <- 0 #remove NA

############ PLOT EXAMPLE

# plot(dtf$index,dtf$RBs.20, type = "p", col = 'red', pch = 19, 
#      xlim = c(3000,10000), ylim = c(1,6), 
#      xlab= "Time", ylab = "RBs")
# lines(dtf$index,dtf$RBs.20, col = 'red')


# ######## PLOTTING VARIOUS  ##########
# plot(dtf$index,dtf$RBs.20,xlim=c(5800,6000),ylim=(c(1,6)),pch=22,col='red',bg="red",xlab="Time (ms)", ylab="RBs used")
# points(dtf$index,dtf$RBs.21,pch=22,col="blue",bg="blue")
# points(dtf$index,dtf$RBs.22,pch=22,col="green",bg="green")
# points(dtf$index,dtf$RBs.23,pch=22,col="yellow",bg="yellow")
# points(dtf$index,dtf$RBs.24,pch=22,col="cyan",bg="cyan")


