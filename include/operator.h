//
// -------------------------------------------------------------------------------------
//
// Copyright (c) 2017-2022 The Regents of the University of Michigan and DFT-FE
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
// --------------------------------------------------------------------------------------
//
// @author Phani Motamarri
//
#ifndef operatorDFTClass_h
#define operatorDFTClass_h

#include "constraintMatrixInfo.h"
#include "headers.h"
#include "process_grid.h"
#include "scalapackWrapper.h"
#include "elpaScalaManager.h"

#include <vector>
#if defined(DFTFE_WITH_DEVICE)
#  include <constraintMatrixInfoDevice.h>
#  include <DeviceBlasWrapper.h> //FIX ME
#  include "deviceDirectCCLWrapper.h"
#endif

namespace dftfe
{
  /**
   * @brief Base class for building the DFT operator and the action of operator on a vector
   *
   * @author Phani Motamarri, Sambit Das
   */
  template <dftfe::utils::MemorySpace memorySpace>
  class operatorDFTClass
  {
    //
    // methods
    //
  public:
    /**
     * @brief Destructor.
     */
    virtual ~operatorDFTClass(){};

#if defined(DFTFE_WITH_DEVICE)
    using constraintInfoClass =
      typename std::conditional<memorySpace ==
                                  dftfe::utils::MemorySpace::DEVICE,
                                dftUtils::constraintMatrixInfoDevice,
                                dftUtils::constraintMatrixInfo>::type;
#else
    using constraintInfoClass = dftUtils::constraintMatrixInfo;
#endif

    virtual dftfe::linearAlgebra::MultiVector<dataTypes::number, memorySpace> &
    getScratchFEMultivector(const unsigned int numVectors,
                            const unsigned int index) = 0;

    virtual void
    createMFVector(
      dftfe::linearAlgebra::MultiVector<dataTypes::number, memorySpace> *
        &multiVector) = 0;

    virtual void
    reshapeMF(dataTypes::number *eigenVector,
              int                totalNumberWaveFunctions,
              bool               isXBlock,
              bool               CVtoBCV) = 0;

    virtual void
    reshapeMF(const dataTypes::number *eigenVector,
              int                      totalNumberWaveFunctions,
              bool                     isXBlock) = 0;

    virtual void
    swapMF() = 0;

    virtual void
    setValueMF(dataTypes::number val = 0) = 0;

    virtual dealii::AlignedVector<dealii::VectorizedArray<double>> &
    getBlockMF(bool isXBlock = true) = 0;

    virtual void
    print(dataTypes::number *Y, dataTypes::number *X, int size) = 0;

    virtual void
    print() = 0;

    virtual void
    norm(dataTypes::number *X, int size, dataTypes::number &nrm) = 0;

    virtual void
    computeAXMF(const double scalarHX,
                const double scalarY,
                const double scalarX) = 0;

    virtual void
    setVJxWMF() = 0;

    virtual void
    init(const std::vector<double> &kPointCoordinates,
         const std::vector<double> &kPointWeights) = 0;

    virtual void
    reinitkPointSpinIndex(const unsigned int kPointIndex,
                          const unsigned int spinIndex) = 0;

    virtual void
    reinitNumberWavefunctions(const unsigned int numWfc) = 0;

    virtual void
    HX(dftfe::linearAlgebra::MultiVector<dataTypes::number, memorySpace> &src,
       const double scalarHX,
       const double scalarY,
       const double scalarX,
       dftfe::linearAlgebra::MultiVector<dataTypes::number, memorySpace> &dst,
       const bool onlyHPrimePartForFirstOrderDensityMatResponse = false) = 0;

    virtual void
    HXCheby(
      dftfe::linearAlgebra::MultiVector<dataTypes::number, memorySpace> &src,
      const double scalarHX,
      const double scalarY,
      const double scalarX,
      dftfe::linearAlgebra::MultiVector<dataTypes::number, memorySpace> &dst,
      const bool onlyHPrimePartForFirstOrderDensityMatResponse = false,
      const bool skip1                                         = false,
      const bool skip2                                         = false,
      const bool skip3                                         = false) = 0;

    virtual dftUtils::constraintMatrixInfo *
    getOverloadedConstraintMatrixHost() const = 0;

    virtual constraintInfoClass *
    getOverloadedConstraintMatrix() const = 0;

    virtual const MPI_Comm &
    getMPICommunicatorDomain() = 0;

    virtual const dftfe::utils::MemoryStorage<double, memorySpace> &
    getInverseSqrtMassVector() = 0;

    virtual const dftfe::utils::MemoryStorage<double, memorySpace> &
    getSqrtMassVector() = 0;
  };
} // namespace dftfe
#endif
