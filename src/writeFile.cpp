#include "writeFile.h"
#include <iostream>
#include <fstream>

writeFile::writeFile()
{
	//fileName = " ";
}

void writeFile::openFile()
{
	ofstream myFile;
	myFile.open("kctdeagulha.txt");
}

void writeFile::closeFile()
{
	myFile.close();
}