// ---------------------------------------------------------------------
//
// Copyright (c) 2017-2022  The Regents of the University of Michigan and DFT-FE
// authors.
//
// This file is part of the DFT-FE code.
//
// The DFT-FE code is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the DFT-FE distribution.
//
// ---------------------------------------------------------------------

#if defined(DFTFE_WITH_DEVICE)
#  ifndef cudaHelpers_h
#    define cudaHelpers_h

#    include <cuda_runtime.h>
#    include "dftfeDataTypesDealii.h"
#    include "memorySpaceDealii.h"
#    include "headers.h"
#    include <cooperative_groups.h>
#    include <cooperative_groups/reduce.h>

namespace dftfe
{
#    define CUDACHECK(cmd)                              \
      do                                                \
        {                                               \
          cudaError_t e = cmd;                          \
          if (e != cudaSuccess)                         \
            {                                           \
              printf("Failed: Cuda error %s:%d '%s'\n", \
                     __FILE__,                          \
                     __LINE__,                          \
                     cudaGetErrorString(e));            \
              exit(EXIT_FAILURE);                       \
            }                                           \
        }                                               \
      while (0)

#    define cublasCheck(expr)                                                            \
      {                                                                                  \
        cublasStatus_t __cublas_error = expr;                                            \
        if ((__cublas_error) != CUBLAS_STATUS_SUCCESS)                                   \
          {                                                                              \
            printf(                                                                      \
              "cuBLAS error on or before line number %d in file: %s. Error code: %d.\n", \
              __LINE__,                                                                  \
              __FILE__,                                                                  \
              __cublas_error);                                                           \
          }                                                                              \
      }

#    define WARPSIZE 32
#    define MAXBLOCKSIZE 1024

  namespace cudaConstants
  {
    static const int blockSize = 256;
  }

  namespace cudaUtils
  {
    void
    setupGPU();

    template <typename NumberTypeComplex, typename NumberTypeReal>
    void
    copyComplexArrToRealArrsGPU(const dataTypes::local_size_type size,
                                const NumberTypeComplex *        complexArr,
                                NumberTypeReal *                 realArr,
                                NumberTypeReal *                 imagArr);


    template <typename NumberTypeComplex, typename NumberTypeReal>
    void
    copyRealArrsToComplexArrGPU(const dataTypes::local_size_type size,
                                const NumberTypeReal *           realArr,
                                const NumberTypeReal *           imagArr,
                                NumberTypeComplex *              complexArr);

    template <typename NumberType>
    void
    copyCUDAVecToCUDAVec(const NumberType *               cudaVecSrc,
                         NumberType *                     cudaVecDst,
                         const dataTypes::local_size_type size);

    template <typename NumberType>
    void
    copyHostVecToCUDAVec(const NumberType *               hostVec,
                         NumberType *                     cudaVector,
                         const dataTypes::local_size_type size);

    template <typename NumberType>
    void
    copyCUDAVecToHostVec(const NumberType *               cudaVector,
                         NumberType *                     hostVec,
                         const dataTypes::local_size_type size);


    void
    add(double *        y,
        const double *  x,
        const double    alpha,
        const int       size,
        cublasHandle_t &cublasHandle);

    double
    l2_norm(const double *  x,
            const int       size,
            const MPI_Comm &mpi_communicator,
            cublasHandle_t &cublasHandle);

    double
    dot(const double *  x,
        const double *  y,
        const int       size,
        const MPI_Comm &mpi_communicator,
        cublasHandle_t &cublasHandle);

    template <typename NumberType>
    void
    set(NumberType *x, const NumberType &alpha, const int size);


    template <typename NumberType, typename MemorySpace>
    class Vector
    {
    public:
      Vector();

      Vector(const dataTypes::local_size_type size, const NumberType s);

      ~Vector();

      void
      resize(const dataTypes::local_size_type size);

      void
      resize(const dataTypes::local_size_type size, const NumberType s);

      void
      set(const NumberType s);

      NumberType *
      begin();

      const NumberType *
      begin() const;

      dataTypes::local_size_type
      size() const;

      void
      clear();

    private:
      NumberType *               d_data;
      dataTypes::local_size_type d_size;
    };


    template <typename NumberType>
    NumberType
    makeNumberFromReal(const double s);

    template <>
    inline double
    makeNumberFromReal(const double s)
    {
      return s;
    }

    template <>
    inline cuDoubleComplex
    makeNumberFromReal(const double s)
    {
      return make_cuDoubleComplex(s, 0.0);
    }

    template <>
    inline float
    makeNumberFromReal(const double s)
    {
      return s;
    }

    template <>
    inline cuFloatComplex
    makeNumberFromReal(const double s)
    {
      return make_cuFloatComplex((float)s, 0.0);
    }

    inline double
    makeRealFromNumber(const double s)
    {
      return s;
    }

    inline float
    makeRealFromNumber(const float s)
    {
      return s;
    }

    inline double
    makeRealFromNumber(const cuDoubleComplex number)
    {
      return number.x;
    }

    inline float
    makeRealFromNumber(const cuFloatComplex number)
    {
      return number.x;
    }

  } // namespace cudaUtils

} // namespace dftfe

#  endif
#endif
