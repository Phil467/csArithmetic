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

#ifndef CSPARGS_H_INCLUDED
#define CSPARGS_H_INCLUDED

#include <iostream>
#include <stdlib.h>
#include <cstdarg>
#include <thread>
#include <mutex>

#define CSTHREAD_NORMAL_EXECUTION 0
#define CSTHREAD_BACKGROUND_EXECUTION 1


template<class T> T* _csAlloc(size_t n)
{
  T*t = (T*)malloc(n*sizeof(T));
  return t;
}

template<class T> T* _csAlloc(size_t n, T init)
{
  T*t = (T*)malloc(n*sizeof(T));
  for(size_t i=0; i<n; i++)
  {
    t[i] = init;
  }
  return t;
}

class CSPARALLEL_API csPARGS
{
public:

    typedef struct
    {
        size_t i;
        void*arg;
    }csVOID_ARG;

    typedef struct
    {
      size_t first;
      size_t last;
    }BOUNDS;

    csPARGS(size_t nArgs=0);
/**
 * @brief TODO: Initializes an Object.
 * @param _nbArgs TODO: Number of arguments.
 */
    void init(size_t _nbArgs);
/**
 * @brief TODO: Sets up the argument indexed by 'i'.
 * @param i TODO: Index of the argument.
 * @param arg TODO: Void pointer containing the argument value to assign.
 */
    void  setArg(size_t i, void* arg);
/**
 * @brief TODO: Sets up the number of arguments.
 * @param nbArgs TODO: Number of arguments.
 */
    void setArgNumber(size_t nbArgs);
/**
 * @brief TODO: Sets up the work block bounds.
 * @param bounds TODO: Strucure 'csPARGS::BOUNDS' that contains the bounds of the work block.
 */
    void setBounds(csPARGS::BOUNDS bounds);
/**
 * @brief TODO: Saves the current block index;
 * @param id TODO: Index of the block.
 */
    void setBlockId(size_t id);
/**
 * @brief TODO: Saves the number of work blocks created.
 * @param blockNumber TODO: nNumber of work blocks created.
 */
    void setBlocksNumber(size_t blockNumber);
/**
 * @brief TODO: Sets up the work size.
 * @param workSize TODO: Work size.
 */
    void setWorkSize(size_t workSize);
/**
 * @brief TODO: Sets up time delay (in nanoseconds) in the loop for a safe thread execution.
 * @param delay TODO: Time delay.
 */
    void setDelay(size_t delay);
/**
 * @brief TODO: Applies an hours delay in the thread.
 */
    void sleepHour();
/**
 * @brief TODO: Applies a minutes delay in the thread.
 */
    void sleepMin();
/**
 * @brief TODO: Applies a seconds delay in the thread.
 */
    void sleepSec();
/**
 * @brief TODO: Applies a milliseconds delay in the thread.
 */
    void sleepMilli();
/**
 * @brief TODO: Applies a microseconds delay in the thread.
 */
    void sleepMicro();
/**
 * @brief TODO: Applies a nanoseconds delay in the thread.
 */
    void sleepNano();
/**
/**
 * @brief TODO: Returns the argument indexed by 'i'.
 * @param i TODO: Index of the argument.
 * @return void* TODO: Void pointer containing the argument.
 */
    void* getArg(size_t i);
/**
 * @brief TODO: Returns the current block index;
 * @return size_t TODO: Current block index;.
 */
    size_t getBlockId();
/**
 * @brief TODO: Returns the number of work blocks created.
 * @return size_t TODO: Number of work blocks created.
 */
    size_t getBlocksNumber();
/**
 * @brief TODO: Returns the work block position;
 * @return csPARGS::BOUNDS TODO: Strucure 'csPARGS::BOUNDS' that contains the bounds of the work block.
 */
    csPARGS::BOUNDS getBounds();
/**
 * @brief TODO: Calculates the regular bounds of a work block following the 'workSize' parameter.
 * @param workSize TODO: Global size.
 * @param min TODO: Output lower bound.
 * @param max TODO: Output upper bound.
 */
    void getBounds(size_t workSize, size_t*min, size_t*max);

/**
 * @brief TODO: Returns the Number of registered arguments.
 * @return size_t TODO: Number of registered arguments.
 */
    size_t getArgNumber();
/**
 * @brief TODO: Returns a pointer to the argument indexed by 'idArg'.
 * @param idArg TODO: Index of the argument.
 * @return T* TODO: Void pointer to the argument.
 */
    template <class T> T* getArgPtr(size_t idArg)
    {
        return (T*)Args[idArg+2];
    };
/**
 * @brief TODO: Returns a pointer to the argument indexed by 'idArg'.
 * @param idArg TODO: Index of the argument.
 * @return T* TODO: Void pointer to the argument.
 */
    template <class T> T* getArgPtr(int idArg)
    {
        return (T*)Args[idArg+2];
    };
/**
 * @brief TODO: Returns the work size.
 * @return size_t TODO: Work size.
 */
    size_t getWorkSize();
/**
 * @brief TODO: Sets up arguments.
 * @param ... TODO: arguments.
 */
    void makeArgs(void*arg,...);
/**
 * @brief TODO: Sets up arguments.
 * @param args TODO: Table of void pointers to arguments addresses.
 * @param nbArgs TODO: Number of arguments.
 */
    void  makeArgs2(void** args, size_t nbArgs);
/**
 * @brief TODO: Sets up arguments.
 * @param args TODO: Table of void pointers to arguments addresses.
 * @param nbArgs TODO: Number of arguments.
 */
    template<size_t _nbArgs> void makeArgs2(void*args[_nbArgs]);
/**
 * @brief TODO: Activates mutex for safe thread coordination when accessing same ressources.
 */
    void lockGuard();
/**
 * @brief TODO: Release object.
 */
    void  clear();
/**
 * @brief TODO: Returns the boundaries of the work section where current thread is executed.
 * @return csPARGS::BOUNDS TODO: Boundaries of the work section.
 */
    operator csPARGS::BOUNDS();
/**
 * @brief TODO: Returns index of the current thread.
 * @return size_t TODO: Index of the current thread.
 */
    operator size_t();
/**
 * @brief TODO: Returns a void pointer to the argument indexed by 'i'.
 * @return void* TODO: Void pointer to the argument.
 */
    void* operator[](size_t i);
/**
 * @brief TODO: Assign a new void pointer to the argument indexed by the member 'i' of the 'va' object.
 * @return void* TODO: Index of the current thread.
 */
    void operator=(csVOID_ARG va);
/**
 * @brief TODO: Defines weather the thread will be executed normally or in background. Can be : CSTHREAD_NORMAL_EXECUTION or CSTHREAD_BACKGROUND_EXECUTION
 */
    bool EXEC_MODE = CSTHREAD_NORMAL_EXECUTION;


private:
    void** Args;
    size_t nbArgs;
    size_t blocksNumber;
    size_t blockId;
    BOUNDS bounds;
    size_t workSize;
    size_t delay;
};

#endif
