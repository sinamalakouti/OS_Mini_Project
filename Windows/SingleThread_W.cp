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
#include <windows.h>
#include <psapi.h>


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


using namespace std;
using namespace std::chrono;

int main(int argc, const char * argv[]) {
	init();

    
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    
    string fileName;
    string word;
    
    
    int count = 0;
    
    std::ifstream file("tst.txt");
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

			PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
    
	cout << "memory is \t";
	cout << virtualMemUsedByMe << endl;
	
	cout << "cpu is \t";
	cout << getCurrentValue() << endl;
   
	
    return 0;
}
