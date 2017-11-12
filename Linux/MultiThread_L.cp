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
#include <unistd.h>
#include <thread>
#include <sys/mman.h>
#include <sys/times.h>
#include <sys/vtimes.h>



using namespace std;
using namespace std::chrono;

#define NUM_THREADS 5

void countWords (string line, int t_num  , int start , int end ) ;

int counter = 0;
int * wordNumbers;

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
    wordNumbers = new int[NUM_THREADS];
    thread treadArray [ NUM_THREADS];

    std::ifstream file("./tst");
    std::string temp;
    int fileSize;
    file.seekg(0, std::ios::end);    // go to the end
    fileSize =  (int)file.tellg();
    char * buffer = (char*) malloc (sizeof(char)*fileSize);
    file.seekg(0, std::ios::beg);
    file.read(buffer, fileSize);
    file.close();

    int charRate = fileSize  / NUM_THREADS;


    for ( int i =0  ; i < NUM_THREADS ; i ++) {

        int start  = i * charRate;
        int end =  ( i  + 1 ) * charRate;
        if ( i == NUM_THREADS  - 1)
            treadArray[i] = thread(countWords, buffer, i , start , fileSize);
        else
            treadArray[i] = thread(countWords, buffer, i , start , end);
    }

    int counter = 0;
    for (int i = 0; i < NUM_THREADS; ++i) {
        treadArray[i].join();
        counter += wordNumbers[i];
    }




    cout << "number of words is \t";
    cout << counter << endl;


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


void countWords (string line ,int t_num, int start , int end ) {
    bool isSpace = false;
    int c =0;

    for (int i = start ; i  < end; i ++) {


        while  ( ! (line[i] == ' ' || line[i] == '\t' )  ){
            isSpace = false;

            i++;

        }
        if ( isSpace == false && line[i] != '\n'){
            c ++;
            isSpace = true;
        }

    }
    cout<<t_num<<endl;
    cout<<c<<endl;

    wordNumbers[t_num] = c;

}
