/*
PRUEBAS
*/

#include "DDS_MPI.h"
#include <ace/Arg_Shifter.h>
#include <ace/Profile_Timer.h>
#include <assert.h>
#include <iostream>

bool GetArg(int &argc, ACE_TCHAR *argv[], ACE_TCHAR *arg, int &value) {
    bool foundArg = false;
    ACE_Arg_Shifter arg_shifter(argc, argv);
    while (arg_shifter.is_anything_left()) {
        const ACE_TCHAR *currentArg = 0;
        if ((currentArg = arg_shifter.get_the_parameter(arg)) != 0) {
            value = ACE_OS::atoi(currentArg);
            arg_shifter.consume_arg();
            foundArg = true;  // can't "return true" here as it will drop any arguments following the matched one
        }
        else
            arg_shifter.ignore_arg();
    }
    return foundArg;
}



int main(int argc, char * argv[]){

    int numIterations = 1, numIntegers = 1;
    GetArg(argc, argv, ACE_TEXT("-i"), numIterations);
    GetArg(argc, argv, ACE_TEXT("-n"), numIntegers);

    std::cout << "---------Prueba de rendimiento---------" << std::endl;
    std::cout << "Número de iteraciones: " << numIterations << std::endl;
    std::cout << "Número de enteros (integers): " << numIntegers << std::endl;
    std::cout << std::endl;
    DDS_MPI p;


    if(p.Initialize(argc, argv) != DDSMPI_SUCCESS){
        std::cerr << "ERROR" << std::endl;
        return -1;
    }

    int size = p.GetSize();
    int rank = p.GetRank();

    std::vector<int> buffer;
    buffer.resize(numIntegers);
    std::cout << "numIntegers: "<< numIntegers << "Size buffer: " << buffer.size()*sizeof(int) << std::endl;
    Status s;

    if(p.GetRank() == 0){
        ACE_Profile_Timer timer;
        timer.start();
        for (int iter = 0; iter < numIterations; iter++) {
            buffer[0] = 1;  // initially 1 for the master
            p.Send(&buffer[0], numIntegers*sizeof(int), 1, 99);
            p.Recv(&buffer[0], numIntegers*sizeof(int), size-1, 99, &s);
            if(buffer[0] != size){
                std::cout << "ERROR: Final count of " << buffer[0] << " does not match size of " << size << std::endl;
                return -1;
            }
        }
        timer.stop();
        ACE_Profile_Timer::ACE_Elapsed_Time et;
        timer.elapsed_time(et);
        std::cout << "Total elapsed time for " << numIterations << " iteration(s) and " << numIntegers << " integer(s): " << et.real_time << 
            "s, time per iteration: " << ((et.real_time / double (numIterations)) * 1000000) << "us" << std::endl;
    }else{
        for (int iter = 0; iter < numIterations; iter++) {
            p.Recv(&buffer[0], numIntegers*sizeof(int), rank-1, 99, &s);
            buffer[0]++;  // +1 for the worker
            p.Send(&buffer[0], numIntegers*sizeof(int), (rank+1)%size, 99);
        }
    }


    std::cout << "FIN PRUEBA" << std::endl;
    p.Finalize(); // ME DA UN FALLO
    return 0;
}