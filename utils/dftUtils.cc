//
// The DFT-FE code is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the DFT-FE distribution.
//
// ---------------------------------------------------------------------
//

/** @file dftUtils.cc
 *  @brief Contains repeatedly used functions in the KSDFT calculations
 *
 *  @author Sambit Das, Denis Davydov
 */

#include <dftUtils.h>
#include <deal.II/base/mpi.h>

using namespace dealii;

namespace dftUtils
{
  double getPartialOccupancy(const double eigenValue,const double fermiEnergy,const double kb,const double T)
  {
    const double factor=(eigenValue-fermiEnergy)/(kb*T);
    return (factor >= 0)?std::exp(-factor)/(1.0 + std::exp(-factor)) : 1.0/(1.0 + std::exp(factor));
  }

  Pool::Pool(const MPI_Comm &mpi_communicator,
             const unsigned int npool)
  {
    const unsigned int n_mpi_processes = Utilities::MPI::n_mpi_processes(mpi_communicator);
    AssertThrow(n_mpi_processes % npool == 0,
                ExcMessage("Number of mpi processes must be a multiple of NUMBER OF POOLS"));
    const unsigned int poolSize= n_mpi_processes/npool;
    const unsigned int taskId = Utilities::MPI::this_mpi_process(mpi_communicator);

    // FIXME: any and all terminal output should be optional
    if (taskId == 0)
      {
        std::cout<<"Number of pools: "<<npool<<std::endl;
        std::cout<<"Pool size: "<<poolSize<<std::endl;
      }
    MPI_Barrier(mpi_communicator);

    const unsigned int color1 = taskId%poolSize ;
    MPI_Comm_split(mpi_communicator,
                   color1,
                   0,
                   &interpoolcomm);
    MPI_Barrier(mpi_communicator);

    const unsigned int color2 = taskId / poolSize ;
    MPI_Comm_split(mpi_communicator,
                   color2,
                   0,
                   &intrapoolcomm);

    // FIXME: why do we need a duplicate?
    MPI_Comm_dup(intrapoolcomm , &mpi_comm_replica);

    // FIXME: output should be optional
    for (unsigned int i=0; i<n_mpi_processes; ++i)
      {
        if (taskId==i)
          std::cout << " My global id is " << taskId << " , pool id is " << Utilities::MPI::this_mpi_process(interpoolcomm)  <<
                    " , intrapool id is " << Utilities::MPI::this_mpi_process(intrapoolcomm) << std::endl;
        MPI_Barrier(mpi_communicator);
      }
  }

  MPI_Comm &Pool::get_replica_comm()
  {
    return mpi_comm_replica;
  }

  MPI_Comm &Pool::get_interpool_comm()
  {
    return interpoolcomm;
  }

  MPI_Comm &Pool::get_intrapool_comm()
  {
    return intrapoolcomm;
  }

}