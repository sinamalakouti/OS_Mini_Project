//
//  main.cpp
//  MINIOS
//
//  Created by Sina on 11/8/17.
//  Copyright © 2017 Sina. All rights reserved.
//
#include<fstream>
#include<iostream>
#include <stdio.h>
#include <string.h>
#include <chrono>
#include <sys/mman.h>
#include <sys/times.h>
#include <sys/vtimes.h>

using namespace std;
using namespace std::chrono;

/* memory usage */
int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

/* cpu usage */
static clock_t lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;

void init(){
    FILE* file;
    struct tms timeSample;
    char line[128];

    lastCPU = times(&timeSample);
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;

    file = fopen("/proc/cpuinfo", "r");
    numProcessors = 0;
    while(fgets(line, 128, file) != NULL){
        if (strncmp(line, "processor", 9) == 0) numProcessors++;
    }
    fclose(file);
}

double getCurrentValue(){
    struct tms timeSample;
    clock_t now;
    double percent;

    now = times(&timeSample);
    if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
        timeSample.tms_utime < lastUserCPU){
        //Overflow detection. Just skip this value.
        percent = -1.0;
    }
    else{
        percent = (timeSample.tms_stime - lastSysCPU) +
            (timeSample.tms_utime - lastUserCPU);
        percent /= (now - lastCPU);
        percent /= numProcessors;
        percent *= 100;
    }
    lastCPU = now;
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;

    return percent;
}





int main(int argc, const char * argv[]) {

    init();

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    string fileName;
    string word;


    int count = 0;

    std::ifstream file("./tst");
    std::string temp;
    bool isSpace = false;

    if  ( !file )
        cout << "file cannot open !!" << endl;
    else {
        while(std::getline(file, temp)) {
            for (int i =0 ; i  < temp.length(); i ++) {


                while  ( ! (temp[i] == ' ' || temp[i] == '\t' )  ){
                    isSpace = false;

                    i++;

                }
                if ( isSpace == false && temp[i] != '\n'){
                    count ++;
                    isSpace = true;
                }

            }


        }

        cout<<"number of word is \t";
        cout<<count<<endl;
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>( t2 - t1 ).count();
    cout << "time is \t ";
    cout << duration << endl;
    cout << "memory is \t";
    cout << getValue() << " KB" << endl;
    cout << "cpu is \t";
    cout << getCurrentValue() << endl;

    return 0;
}
