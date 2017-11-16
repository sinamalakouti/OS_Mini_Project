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
#include <windows.h>
#include <psapi.h>
#include <thread>


using namespace std;
using namespace std::chrono;

#define NUM_THREADS 5

void countWords (string line, int t_num  , int start , int end ) ;

int counter = 0;
int * wordNumbers;

static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;
static HANDLE self;

void init(){
    SYSTEM_INFO sysInfo;
    FILETIME ftime, fsys, fuser;

    GetSystemInfo(&sysInfo);
    numProcessors = sysInfo.dwNumberOfProcessors;

    GetSystemTimeAsFileTime(&ftime);
    memcpy(&lastCPU, &ftime, sizeof(FILETIME));

    self = GetCurrentProcess();
    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
    memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
}

double getCurrentValue(){
    FILETIME ftime, fsys, fuser;
    ULARGE_INTEGER now, sys, user;
    double percent;

    GetSystemTimeAsFileTime(&ftime);
    memcpy(&now, &ftime, sizeof(FILETIME));

    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    memcpy(&sys, &fsys, sizeof(FILETIME));
    memcpy(&user, &fuser, sizeof(FILETIME));
    percent = (sys.QuadPart - lastSysCPU.QuadPart) +
        (user.QuadPart - lastUserCPU.QuadPart);
    percent /= (now.QuadPart - lastCPU.QuadPart);
    percent /= numProcessors;
    lastCPU = now;
    lastUserCPU = user;
    lastSysCPU = sys;

    return percent * 100;
}

int main(int argc, const char * argv[]) {
	init();
    
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    
    string fileName;
    string word;
    wordNumbers = new int[NUM_THREADS];
    thread treadArray [ NUM_THREADS];
    
    std::ifstream file("tst.txt");
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
	
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
    
	cout << "memory is \t";
	cout << virtualMemUsedByMe << endl;
	
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
