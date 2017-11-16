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
#include <unistd.h>
#include <windows.h>
#include <psapi.h>

#include <chrono>

using namespace std;
using namespace std::chrono;
//
#define NUM_PROCESS 5

LPCSTR countWords (string line, int t_num  , int start , int end ) ;

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

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

   // wordNumbers =     static_cast<int *>(mmap(NULL, sizeof(int) * (NUM_PROCESS + 1) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS , -1, 0));
    wordNumbers = new int [ NUM_PROCESS];
    std::ifstream file("tst.txt");

    std::string temp;
    int fileSize;
    file.seekg(0, std::ios::end);    // go to the end
    fileSize = (int) file.tellg();
    char * buffer = (char*) malloc (sizeof(char)*fileSize);
    file.seekg(0, std::ios::beg);
    file.read(buffer, fileSize);
    file.close();
    int pCounter = 0;
    int charRate = fileSize  / NUM_PROCESS;
    long  counter = 0;

    HANDLE childs [NUM_PROCESS] ;
    cout<<"HERE"<<endl;


    for ( int i =0  ; i < NUM_PROCESS ; i ++) {


        int start  = i * charRate;
        int end =  ( i  + 1 ) * charRate;



childs[i] = (HANDLE) CreateProcess(countWords(buffer , i, start , end),   // No module name (use command line)
                                              NULL,        // Command line
                                              NULL,           // Process handle not inheritable
                                              NULL,           // Thread handle not inheritable
                                              FALSE,          // Set handle inheritance to FALSE
                                              0,              // No creation flags
                                              NULL,           // Use parent's environment block
                                              NULL,           // Use parent's starting directory
                                              &si,            // Pointer to STARTUPINFO structure
                                              &pi);


        WaitForSingleObject(childs[i], INFINITE);
    }




        for  ( int j =0  ; j < NUM_PROCESS ; j++)
        {
            cout<< wordNumbers[j]<<endl ;
            counter += wordNumbers[j];

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
    




    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}


LPCSTR countWords (string line ,int t_num, int start , int end ) {
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
    //cout<<"sub routing=e"<<endl;
    //cout<<t_num<<endl;
    //cout<<c<<endl;
    wordNumbers[t_num] = c;

}
