#ifndef WRITE_FILE_H_
#define WRITE_FILE_H_


#include <iostream>
#include <fstream>

class writeFile
{
public:
    writeFile(); // constructor
    void openFile(); // set hour, minute and second
    void closeFile(); // print time in universal-time format
private:
    char fileName; // 0 - 23 (24-hour clock format)
    
}; // end class Time

#endif