#pragma once

#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_CSPARALLEL_DLL
    #define CSPARALLEL_API __declspec(dllexport)
  #else
    #define CSPARALLEL_API __declspec(dllimport)
  #endif
#else
  #ifdef BUILDING_CSPARALLEL_DLL
    #define CSPARALLEL_API __attribute__ ((visibility ("default")))
  #else
    #define CSPARALLEL_API
  #endif
#endif

#ifndef CSPARALLEL_H_INCLUDED
#define CSPARALLEL_H_INCLUDED

#include <cstddef>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <string.h>
#include <memoryapi.h>
#include "csPARGS.h"
#include "csPERF_MEASUREMENT.h"

using namespace std;

typedef csPARGS::BOUNDS* BUFFER_SHAPE;

namespace csParallelTask
{

inline void registerArgs(void**&Args, size_t& nbArgs)
{

};

template<typename... _Args> void registerArgs(void**&Args, size_t& nbArgs, void* arg, _Args... args)
{
  nbArgs++;
  Args = (void**)realloc(Args, nbArgs*sizeof(void*));
  Args[nbArgs-1] = arg;
  registerArgs(Args, nbArgs, args...);
};

/**
 * @brief TODO: Sets up arguments of each work block for the function indexed by 'idf'.
 * @param idf TODO: Index of the function.
 * @param shape TODO: Table containing the bounds of each created block.
 * @param funcArgs TODO: Object of csPARGS class containing arguments of the function.
 */
void setArgs(size_t idf, BUFFER_SHAPE shape, csPARGS funcArgs);
/**
 * @brief TODO: Sets up arguments of each regular (same size) work block for the function indexed by 'idf'.
 * @param idf TODO: Index of the function.
 * @param workSize TODO: The size of the work, used to build regular blocks.
 * @param funcArgs TODO: Object of csPARGS class containing arguments of the function.
 */
void setArgsRegular(size_t idf, size_t workSize, csPARGS funcArgs);
/**
 * @brief TODO: reshape the work by assigning new one.
 * @param idf TODO: Index of the function.
 * @param shape TODO: Table containing the bounds of each created block.
 */
void setBufferShape(size_t idf, BUFFER_SHAPE shape);
/**
 * @brief TODO: reshape the work with regular (same size) blocks, following the new 'workSize' parameter.
 * @param idf TODO: Index of the function.
 * @param workSize TODO: Size of the work.
 */
void setBufferShapeRegular(size_t idf, size_t workSize);
/**
 * @brief TODO: Sets up time delay (in nanoseconds) for loops to make safe execution.
 * @param idf TODO: Index of the function.
 * @param delay TODO: Delay to assign to every thread.
 */
void setDelay(size_t idf, size_t delay);
/**
 * @brief TODO: Sets up time delay (in nanoseconds) for loops to make safe execution.
 * @param idf TODO: Index of the function.
 * @param delayList TODO: Lists of delay values for each thread.
 */
void setDelay(size_t idf, vector<size_t> delayList);
/**
 * @brief TODO: Defines weather the function will be executed normally or in background.
 * @param idf TODO: Index of the function.
 * @param execMode TODO: Execution mode. Can be : CSTHREAD_NORMAL_EXECUTION or CSTHREAD_BACKGROUND_EXECUTION.
 */
void setExecutionMode(size_t idf, bool execMode);
/**
 * @brief TODO: Defines weather each thread will be executed normally or in background.
 * @param idf TODO: Index of the function.
 * @param execMode TODO: List of execution mode for each thread. Each one can be : CSTHREAD_NORMAL_EXECUTION or CSTHREAD_BACKGROUND_EXECUTION.
 */
void setExecutionMode(size_t idf, vector<bool> execMode);
/**
 * @brief TODO: Make 'nBlocks' Threads of same size that work each one on a part of the work.
 * @param workSize TODO: The size of the work.
 * @param nBlocks TODO: Number of work blocks. Each block is managed by one thread. 'nBlocks' is modified to be equal to the number of computing units of the CPU, if it value is greater.
 * @return BUFFER_SHAPE  TODO: Table containing all the created blocks.
 */
BUFFER_SHAPE  makeRegularBufferShape(size_t workSize, size_t& nBlocks);
/**
 * @brief TODO: Register a new function.
 * @param nBlocks TODO: Number of work blocks, corresponding each one to a thread.
 * @return workSize TODO: Buffer Size.
 * @param shape TODO: Table containing the bounds of each block to be created.
 * @param charfName TODO: Name of the function to register.
 * @param blockFunc TODO: Pointer to the function to register.
 * @return size_t TODO: Index of the registered function.
 */
size_t registerFunction(size_t nBlocks, size_t workSize, BUFFER_SHAPE shape, char* fName, void(*blockFunc)(csPARGS), csPARGS funcArgs);
/**
 * @brief TODO: Extended registation of a new function. Allows to specify directly pointers to each argument of the function, rather than giving a 'csPARGS' class that contains arguments, as done by 'registerFunction'.
 * @param nBlocks TODO: Number of work blocks, corresponding each one to a thread.
 * @return workSize TODO: Buffer Size.
 * @param shape TODO: Table containing the bounds of each block to be created.
 * @param charfName TODO: Name of the function to register.
 * @param blockFunc TODO: Pointer to the function to register.
 * @return size_t TODO: Index of the registered function.
 */
size_t registerFunctionEx(size_t nBlocks, size_t workSize, BUFFER_SHAPE shape, char*fName, void(*blockFunc)(csPARGS), size_t nbArgs,...);
/**
 * @brief TODO: Extended registation of a new function. Allows to specify directly pointers to each argument of the function, rather than giving a 'csPARGS' class that contains arguments, as done by 'registerFunction'.
 * @param nBlocks TODO: Number of work blocks, corresponding each one to a thread.
 * @param workSize TODO: The size of the work, used to build regular blocks.
 * @param charfName TODO: Name of the function to register.
 * @param blockFunc TODO: Pointer to the function to register.
 * @return size_t TODO: Index of the registered function.
 */
size_t registerFunctionRegularEx(size_t nBlocks, size_t workSize, char*fName, void(*blockFunc)(csPARGS), size_t nbArgs,...);
/**
 * @brief TODO: Extended registation of a new function. Allows to specify directly pointers to each argument of the function, rather than giving a 'csPARGS' class that contains arguments, as done by 'registerFunction'.
 * @param nBlocks TODO: Number of work blocks, corresponding each one to a thread.
 * @param workSize TODO: The size of the work, used to build regular blocks.
 * @param charfName TODO: Name of the function to register.
 * @param blockFunc TODO: Pointer to the function to register.
 * @param arg TODO: Void Pointer to the first argument.
 * @param args TODO: Other arguments in variable number.
 */
template<typename... _Args> size_t registerFunctionRegularEx(size_t nBlocks, size_t workSize, char*fName, void(*Function)(csPARGS), void*arg, _Args... args)
{
  void **Args = 0;
  size_t nbArgs = 0;

  registerArgs(Args, nbArgs, arg, args...);

  BUFFER_SHAPE shape = makeRegularBufferShape(workSize, nBlocks);
  csPARGS funcArgs(nbArgs);
  funcArgs.makeArgs2(Args,nbArgs);

  size_t idf = registerFunction(nBlocks, workSize, shape, fName, Function, funcArgs);
  free(shape);
  return idf;
};
/**
 * @brief TODO: Returns min(nThread, getMaxThreadNumber()), where 'getMaxThreadNumber()' function gives the number of physical units of the CPU.
 * @param nThread TODO: Number of thread needed.
 * @return size_t TODO: min(nThread, getMaxThreadNumber()).
 */
size_t getRightThreadNumber(size_t nThread);
/**
 * @brief TODO: Gives the number of physical units of the CPU. Each one excecutes one thread.
 * @return size_t TODO: Number of physical units of the CPU.
 */
size_t getMaxThreadNumber();
/**
 * @brief TODO: Returns table that contains the 'csPARGS' arguments objects of all the blocks.
 * @param idf TODO: Index of the function.
 * @return vector<csPARGS> TODO: Table of the arguments objects of all blocks.
 */
vector<csPARGS> getArgs(size_t idf);
/**
 * @brief TODO: Returns the arguments object of the 'ida' block for the 'idf' function.
 * @param idf TODO: Index of the function.
 * @param ida TODO: Index of block.
 * @return csPARGS TODO: Arguments object of 'ida' block.
 */
csPARGS getArgs(size_t idf, size_t ida);
/**
 * @brief TODO: Returns the index of the function identified by it name 'funcName'.
 * @return size_t TODO: Index of the specified function.
 */
size_t getId(const char*funcName);
/**
 * @brief TODO: Returns the index of the function identified by it name address 'f'.
 * @param f TODO: Address of the function.
 * @return size_t TODO: Index of the specified function.
 */
size_t getId(void(*f)(csPARGS));
/**
 * @brief TODO: Returns the global work size for the function indexed by 'idf'.
 * @param idf TODO: Index of the function.
 * @return size_t TODO: Global work size.
 */
size_t getWorkSize(int idf);
/**
 * @brief TODO: Update the 'ida' argument of 'idf' function, with 'arg'.
 * @param idf TODO: Index of the function.
 * @param ida TODO: Index of the argument.
 * @param arg TODO: Argument to assign.
 */
void updateArg(size_t idf, size_t ida, void* &&arg);
/**
 * @brief TODO: Update the 'ida' list of arguments of 'idf' function, with 'arg' list.
 * @param idf TODO: Index of the function.
 * @param ida TODO: List of arguments Indexes.
 * @param arg TODO: Lists of arguments to assign.
 */
void updateArg(size_t idf, initializer_list<size_t> ida, initializer_list<void*> arg);
/**
* @brief TODO: Update the 'ida' list of arguments of 'idf' function, with 'arg' list.
 * @param idf TODO: Index of the function.
 * @param ida TODO: List of arguments Indexes.
 * @param arg TODO: Lists of arguments to assign.
 */
template<size_t N> void updateArg(size_t idf, const size_t (&ida)[N], void* const (&arg)[N]);
/**
 * @brief TODO: Computes every block in parallel.
 * @param id TODO: Index of the function to execute.
 */
void execute(int id);
/**
 * @brief TODO: Computes every block in parallel.
 * @param charfuncName TODO: Name of the function to execute.
 */
void execute(const char*funcName);
/**
 * @brief TODO: Computes every block in parallel for the function specified by pointer 'f'.
 * @param f TODO: Pointer to the function to execute.
 */
void execute(void(*f)(csPARGS));
/**
 * @brief TODO: Describe the purpose of execute.
 * @param threads TODO: Table Threads to execute.
 */
void execute(vector<thread> threads);


}

using csFunctionType = void(*)(int);



#endif
