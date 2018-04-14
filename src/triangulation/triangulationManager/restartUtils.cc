// ---------------------------------------------------------------------
//
// Copyright (c) 2017 The Regents of the University of Michigan and DFT-FE authors.
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
//
/** @file restartUtils.cc
 *
 *  @author Sambit Das
 */

#include <deal.II/distributed/solution_transfer.h>
#include <deal.II/numerics/solution_transfer.h>

namespace dftfe {

    // Move/rename a checkpoint file
    namespace internal
    {
      void moveFile(const std::string &old_name, const std::string &new_name)
      {

	int error = system (("mv " + old_name + " " + new_name).c_str());

	// If the above call failed, e.g. because there is no command-line
	// available, try with internal functions.
	if (error != 0)
	{
	    std::ifstream ifile(new_name);
	    if (static_cast<bool>(ifile))
	    {
		error = remove(new_name.c_str());
		AssertThrow (error == 0, ExcMessage(std::string ("Unable to remove file: "
		+ new_name
		+ ", although it seems to exist. "
		+ "The error code is "
		+ dealii::Utilities::to_string(error) + ".")));
	    }

	    error = rename(old_name.c_str(),new_name.c_str());
	    AssertThrow (error == 0, ExcMessage(std::string ("Unable to rename files: ")
	    +
	    old_name + " -> " + new_name
	    + ". The error code is "
	    + dealii::Utilities::to_string(error) + "."));
	}
      }

      void verifyCheckpointFileExists(const std::string filename)
      {
          std::ifstream in (filename);
          if (!in)
	  {
            AssertThrow (false,
               ExcMessage (std::string("DFT-FE Error: You are trying to restart a previous computation, "
               "but the restart file <")
                +
                filename
                +
                "> does not appear to exist!"));
	  }
      }
    }
    //
    //
    void triangulationManager::saveSupportTriangulations()
    {
       if (d_serialTriangulationUnmoved.n_global_active_cells()!=0 && this_mpi_process==0)
       {
         const std::string filename1="serialUnmovedTria.chk";
	 if (std::ifstream(filename1))
	     internal::moveFile(filename1, filename1+".old");

	 d_serialTriangulationUnmoved.save(filename1.c_str());
       }

       if (d_parallelTriangulationUnmovedPrevious.n_global_active_cells()!=0)
       {
         const std::string filename2="parallelUmmovedPrevTria.chk";
	 if (std::ifstream(filename2) && this_mpi_process==0)
	     internal::moveFile(filename2, filename2+".old");
	 MPI_Barrier(mpi_communicator);

         d_parallelTriangulationUnmovedPrevious.save(filename2.c_str());
       }

       if (d_serialTriangulationUnmovedPrevious.n_global_active_cells()!=0 && this_mpi_process==0)
       {
         const std::string filename3="serialUnmovedPrevTria.chk";
	 if (std::ifstream(filename3) && this_mpi_process==0)
	      internal::moveFile(filename3, filename3+".old");

         d_serialTriangulationUnmovedPrevious.save(filename3.c_str());
       }
    }

    //
    void triangulationManager::loadSupportTriangulations()
    {
       if (d_serialTriangulationUnmoved.n_global_active_cells()!=0)
       {
	   const std::string filename1="serialUnmovedTria.chk";
	   internal::verifyCheckpointFileExists(filename1);
	   try
	   {
	      d_serialTriangulationUnmoved.load(filename1.c_str(),false);
	   }
	   catch (...)
	   {
	     AssertThrow(false, ExcMessage("DFT-FE Error: Cannot open checkpoint file- serialUnmovedTria.chk or read the triangulation stored there."));
	   }
       }

       if (d_parallelTriangulationUnmovedPrevious.n_global_active_cells()!=0)
       {
         const std::string filename2="parallelUmmovedPrevTria.chk";
	 internal::verifyCheckpointFileExists(filename2);
         try
         {
	    d_parallelTriangulationUnmovedPrevious.load(filename2.c_str());
         }
         catch (...)
         {
	   AssertThrow(false, ExcMessage("DFT-FE Error: Cannot open checkpoint file- parallelUmmovedPrevTria.chk or read the triangulation stored there."));
         }
       }

       if (d_serialTriangulationUnmovedPrevious.n_global_active_cells()!=0)
       {
         const std::string filename3="serialUnmovedPrevTria.chk";
	 internal::verifyCheckpointFileExists(filename3);
         try
         {
	    d_serialTriangulationUnmovedPrevious.load(filename3.c_str(),false);
         }
         catch (...)
         {
	    AssertThrow(false, ExcMessage("DFT-FE Error: Cannot open checkpoint file- serialUnmovedPrevTria.chk or read the triangulation stored there."));
         }
       }
    }

    //
    void
    triangulationManager::saveTriangulationsSolutionVectors
				 (const unsigned int feOrder,
				  const unsigned int nComponents,
				  const std::vector< const dealii::parallel::distributed::Vector<double> * > & solutionVectors,
	                          const MPI_Comm & interpoolComm)
    {

      const unsigned int poolId=dealii::Utilities::MPI::this_mpi_process(interpoolComm);
      const unsigned int minPoolId=dealii::Utilities::MPI::min(poolId,interpoolComm);

      if (poolId==minPoolId)
      {
         dealii::FESystem<3> FE(dealii::FE_Q<3>(dealii::QGaussLobatto<1>(feOrder+1)), nComponents); //linear shape function
         DoFHandler<3> dofHandler (d_parallelTriangulationUnmoved);
         dofHandler.distribute_dofs(FE);

         dealii::parallel::distributed::SolutionTransfer<3,typename dealii::parallel::distributed::Vector<double> > solTrans(dofHandler);
         //assumes solution vectors are ghosted
         solTrans.prepare_serialization(solutionVectors);

         const std::string filename="parallelUnmovedTriaSolData.chk";
	 if (std::ifstream(filename) && this_mpi_process==0)
	     internal::moveFile(filename, filename+".old");
	 MPI_Barrier(mpi_communicator);

         d_parallelTriangulationUnmoved.save(filename.c_str());

	 saveSupportTriangulations();
      }
    }

    //
    //
    void
    triangulationManager::loadTriangulationsSolutionVectors
				 (const unsigned int feOrder,
				  const unsigned int nComponents,
				  std::vector< dealii::parallel::distributed::Vector<double> * > & solutionVectors)
    {
      loadSupportTriangulations();
      const std::string filename="parallelUnmovedTriaSolData.chk";
      internal::verifyCheckpointFileExists(filename);
      try
      {
         d_parallelTriangulationMoved.load(filename.c_str());
	 d_parallelTriangulationUnmoved.load(filename.c_str());
      }
      catch (...)
      {
        AssertThrow(false, ExcMessage("DFT-FE Error: Cannot open checkpoint file- parallelUnmovedTriaSolData.chk or read the triangulation stored there."));
      }

      dealii::FESystem<3> FE(dealii::FE_Q<3>(dealii::QGaussLobatto<1>(feOrder+1)), nComponents); //linear shape function
      DoFHandler<3> dofHandler (d_parallelTriangulationMoved);
      dofHandler.distribute_dofs(FE);
      dealii::parallel::distributed::SolutionTransfer<3,typename dealii::parallel::distributed::Vector<double> > solTrans(dofHandler);

      for (unsigned int i=0; i< solutionVectors.size();++i)
            solutionVectors[i]->zero_out_ghosts();

      //assumes solution vectors are not ghosted
      solTrans.deserialize (solutionVectors);

      //dummy de-serialization for d_parallelTriangulationUnmoved to avoid assert fail in call to save
      dofHandler.initialize(d_parallelTriangulationUnmoved,FE);
      dofHandler.distribute_dofs(FE);
      dealii::parallel::distributed::SolutionTransfer<3,typename dealii::parallel::distributed::Vector<double> > solTransDummy(dofHandler);

      std::vector< dealii::parallel::distributed::Vector<double> * >
	                                    dummySolutionVectors(solutionVectors.size());
      for (unsigned int i=0; i<dummySolutionVectors.size();++i)
      {
          dummySolutionVectors[i]->reinit(*solutionVectors[0]);
	  dummySolutionVectors[i]->zero_out_ghosts();
      }

      solTransDummy.deserialize (dummySolutionVectors);
    }

    //
    //
    //
    void
    triangulationManager::saveTriangulationsCellQuadData
	      (const std::vector<const std::map<dealii::CellId, std::vector<double> > *> & cellQuadDataContainerIn,
	       const MPI_Comm & interpoolComm)
    {

      const unsigned int poolId=dealii::Utilities::MPI::this_mpi_process(interpoolComm);
      const unsigned int minPoolId=dealii::Utilities::MPI::min(poolId,interpoolComm);

      if (poolId==minPoolId)
      {
	 const unsigned int containerSize=cellQuadDataContainerIn.size();
         AssertThrow(containerSize!=0,ExcInternalError());

	 unsigned int totalQuadVectorSize=0;
	 for (unsigned int i=0; i<containerSize;++i)
	 {
           const unsigned int quadVectorSize=(*cellQuadDataContainerIn[i]).begin()->second.size();
	   Assert(quadVectorSize!=0,ExcInternalError());
	   totalQuadVectorSize+=quadVectorSize;
	 }

	 const unsigned int dataSizeInBytes=sizeof(double)*totalQuadVectorSize;
         const unsigned int offset = d_parallelTriangulationUnmoved.register_data_attach
	          (dataSizeInBytes,
		   [&](const typename dealii::parallel::distributed::Triangulation<3>::cell_iterator &cell,
		       const typename dealii::parallel::distributed::Triangulation<3>::CellStatus status,
		       void * data) -> void
		       {
			  if (cell->active() && cell->is_locally_owned())
			  {
			     Assert((*cellQuadDataContainerIn[0]).find(cell->id())!=(*cellQuadDataContainerIn[0]).end(),ExcInternalError());

			     double* dataStore = reinterpret_cast<double*>(data);

			     double tempArray[totalQuadVectorSize];
			     unsigned int count=0;
			     for (unsigned int i=0; i<containerSize;++i)
	                     {
			       const unsigned int quadVectorSize=
			           (*cellQuadDataContainerIn[i]).begin()->second.size();

                               for (unsigned int j=0; j<quadVectorSize;++j)
			       {
			           tempArray[count]=(*cellQuadDataContainerIn[i]).find(cell->id())->second[j];
				   count++;
                               }
			     }

		             std::memcpy(dataStore,
				         &tempArray[0],
					 dataSizeInBytes);
			  }
		          else
			  {
			     double* dataStore = reinterpret_cast<double*>(data);
			     double tempArray[totalQuadVectorSize];
			     std::memcpy(dataStore,
				         &tempArray[0],
					 dataSizeInBytes);
			  }
		       }
		   );

         const std::string filename="parallelUnmovedTriaSolData.chk";
	 if (std::ifstream(filename) && this_mpi_process==0)
	    internal::moveFile(filename, filename+".old");
	 MPI_Barrier(mpi_communicator);
         d_parallelTriangulationUnmoved.save(filename.c_str());

	 saveSupportTriangulations();
      }//poolId==minPoolId check
    }

    //
    //
    void
    triangulationManager::loadTriangulationsCellQuadData
	       (std::vector<std::map<dealii::CellId, std::vector<double> > > & cellQuadDataContainerOut,
		const std::vector<unsigned int>  & cellDataSizeContainer)
    {
      loadSupportTriangulations();
      const std::string filename="parallelUnmovedTriaSolData.chk";
      internal::verifyCheckpointFileExists(filename);
      try
      {
         d_parallelTriangulationMoved.load(filename.c_str());
	 d_parallelTriangulationUnmoved.load(filename.c_str());
      }
      catch (...)
      {
        AssertThrow(false, ExcMessage("DFT-FE Error: Cannot open checkpoint file- parallelUnmovedTriaSolData.chk or read the triangulation stored there."));
      }

      AssertThrow(cellQuadDataContainerOut.size()!=0,ExcInternalError());
      AssertThrow(cellQuadDataContainerOut.size()==cellDataSizeContainer.size(),ExcInternalError());

      const unsigned int totalQuadVectorSize=std::accumulate(cellDataSizeContainer.begin(), cellDataSizeContainer.end(), 0);

      //FIXME: The underlying function calls to register_data_attach to notify_ready_to_unpack
      //will need to re-evaluated after the dealii github issue #6223 is fixed
      const  unsigned int offset1 = d_parallelTriangulationMoved.register_data_attach
	          (totalQuadVectorSize*sizeof(double),
		   [&](const typename dealii::parallel::distributed::Triangulation<3>::cell_iterator &cell,
		       const typename dealii::parallel::distributed::Triangulation<3>::CellStatus status,
		       void * data) -> void
		       {
		       }
		  );

      d_parallelTriangulationMoved.notify_ready_to_unpack
	      (offset1,[&](const typename dealii::parallel::distributed::Triangulation<3>::cell_iterator &cell,
		     const typename dealii::parallel::distributed::Triangulation<3>::CellStatus status,
		     const void * data) -> void
		   {
		      if (cell->active() && cell->is_locally_owned())
		      {
			 const double* dataStore = reinterpret_cast<const double*>(data);

			 double tempArray[totalQuadVectorSize];

			 std::memcpy(&tempArray[0],
				     dataStore,
				     totalQuadVectorSize*sizeof(double));

			 unsigned int count=0;
			 for (unsigned int i=0; i<cellQuadDataContainerOut.size();++i)
	                 {
			   Assert(cellDataSizeContainer[i]!=0,ExcInternalError());
                           cellQuadDataContainerOut[i][cell->id()]=std::vector<double>(cellDataSizeContainer[i]);
			   for (unsigned int j=0; j<cellDataSizeContainer[i];++j)
			   {
			       cellQuadDataContainerOut[i][cell->id()][j]=tempArray[count];
			       count++;
			   }
			 }//container loop
		      }
		   }
	       );

     //dummy de-serialization for d_parallelTriangulationUnmoved to avoid assert fail in call to save
     //FIXME: This also needs to be re-evaluated after the dealii github issue #6223 is fixed
     const  unsigned int offset2 = d_parallelTriangulationUnmoved.register_data_attach
	          (totalQuadVectorSize*sizeof(double),
		   [&](const typename dealii::parallel::distributed::Triangulation<3>::cell_iterator &cell,
		       const typename dealii::parallel::distributed::Triangulation<3>::CellStatus status,
		       void * data) -> void
		       {
		       }
		  );

      d_parallelTriangulationUnmoved.notify_ready_to_unpack
	      (offset2,[&](const typename dealii::parallel::distributed::Triangulation<3>::cell_iterator &cell,
		     const typename dealii::parallel::distributed::Triangulation<3>::CellStatus status,
		     const void * data) -> void
		   {
		   }
	       );

    }
}
