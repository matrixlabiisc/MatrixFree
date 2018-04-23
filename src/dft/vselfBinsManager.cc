// ---------------------------------------------------------------------
//
// Copyright (c) 2017-2018 The Regents of the University of Michigan and DFT-FE authors.
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
// @author  Sambit Das, Phani Motamarri
//

#include <vselfBinsManager.h>
#include <dftParameters.h>


#include "solveVselfInBins.cc"
#include "createBinsSanityCheck.cc"
namespace dftfe
{

    namespace internal
    {
	void exchangeAtomToGlobalNodeIdMaps(const int totalNumberAtoms,
					    std::map<int,std::set<int> > & atomToGlobalNodeIdMap,
					    unsigned int numMeshPartitions,
					    const MPI_Comm & mpi_communicator)

	{
	  std::map<int,std::set<int> >::iterator iter;

	  for(int iGlobal = 0; iGlobal < totalNumberAtoms; ++iGlobal){

	    //
	    // for each charge, exchange its global list across all procs
	    //
	    iter = atomToGlobalNodeIdMap.find(iGlobal);

	    std::vector<int> localAtomToGlobalNodeIdList;

	    if(iter != atomToGlobalNodeIdMap.end()){
	      std::set<int>  & localGlobalNodeIdSet = iter->second;
	      std::copy(localGlobalNodeIdSet.begin(),
			localGlobalNodeIdSet.end(),
			std::back_inserter(localAtomToGlobalNodeIdList));

	    }

	    int numberGlobalNodeIdsOnLocalProc = localAtomToGlobalNodeIdList.size();

            std::vector<int> atomToGlobalNodeIdListSizes(numMeshPartitions);

	    MPI_Allgather(&numberGlobalNodeIdsOnLocalProc,
			  1,
			  MPI_INT,
			  &(atomToGlobalNodeIdListSizes[0]),
			  1,
			  MPI_INT,
			  mpi_communicator);

	    int newAtomToGlobalNodeIdListSize =
	      std::accumulate(&(atomToGlobalNodeIdListSizes[0]),
			      &(atomToGlobalNodeIdListSizes[numMeshPartitions]),
			      0);

	    std::vector<int> globalAtomToGlobalNodeIdList(newAtomToGlobalNodeIdListSize);

	    std::vector<int> mpiOffsets(numMeshPartitions);

	    mpiOffsets[0] = 0;

	    for(int i = 1; i < numMeshPartitions; ++i)
	      mpiOffsets[i] = atomToGlobalNodeIdListSizes[i-1]+ mpiOffsets[i-1];

	    MPI_Allgatherv(&(localAtomToGlobalNodeIdList[0]),
			   numberGlobalNodeIdsOnLocalProc,
			   MPI_INT,
			   &(globalAtomToGlobalNodeIdList[0]),
			   &(atomToGlobalNodeIdListSizes[0]),
			   &(mpiOffsets[0]),
			   MPI_INT,
			   mpi_communicator);

	    //
	    // over-write local interaction with items of globalInteractionList
	    //
	    for(int i = 0 ; i < globalAtomToGlobalNodeIdList.size(); ++i)
	      (atomToGlobalNodeIdMap[iGlobal]).insert(globalAtomToGlobalNodeIdList[i]);

	  }
	}
    }

    //constructor
    template<unsigned int FEOrder>
    vselfBinsManager<FEOrder>::vselfBinsManager(const MPI_Comm &mpi_comm):
      mpi_communicator (mpi_comm),
      n_mpi_processes (dealii::Utilities::MPI::n_mpi_processes(mpi_comm)),
      this_mpi_process (dealii::Utilities::MPI::this_mpi_process(mpi_comm)),
      pcout (std::cout, (dealii::Utilities::MPI::this_mpi_process(MPI_COMM_WORLD) == 0))
    {

    }

    template<unsigned int FEOrder>
    void vselfBinsManager<FEOrder>::createAtomBins(std::vector<dealii::ConstraintMatrix * > & constraintsVector,
		                           const dealii::DoFHandler<3> &  dofHandler,
			                   const dealii::ConstraintMatrix & constraintMatrix,
			                   const std::map<dealii::types::global_dof_index, double> & atoms,
			                   std::vector<std::vector<double> > & atomLocations,
			                   const std::vector<std::vector<double> > & imagePositions,
			                   const std::vector<int> & imageIds,
			                   const std::vector<double> & imageCharges,
			                   const double radiusAtomBall)

    {
      d_bins.clear();
      d_imageIdsInBins.clear();
      d_boundaryFlag.clear();
      d_vselfBinField.clear();
      d_closestAtomBin.clear();
      d_vselfBinConstraintMatrices.clear();

      d_atomLocations=atomLocations;
      d_imagePositions=imagePositions;
      d_imageCharges=imageCharges;

      const unsigned int numberImageCharges = imageIds.size();
      const unsigned int numberGlobalAtoms = atomLocations.size();
      const unsigned int totalNumberAtoms = numberGlobalAtoms + numberImageCharges;

      const unsigned int vertices_per_cell=dealii::GeometryInfo<3>::vertices_per_cell;
      const unsigned int dofs_per_cell = dofHandler.get_fe().dofs_per_cell;

      std::map<dealii::types::global_dof_index, dealii::Point<3> > supportPoints;
      dealii::DoFTools::map_dofs_to_support_points(dealii::MappingQ1<3,3>(), dofHandler, supportPoints);

      dealii::IndexSet   locally_relevant_dofs;
      dealii::DoFTools::extract_locally_relevant_dofs(dofHandler, locally_relevant_dofs);

      //create a constraint matrix with only hanging node constraints
      dealii::ConstraintMatrix onlyHangingNodeConstraints;
      onlyHangingNodeConstraints.reinit(locally_relevant_dofs);
      dealii::DoFTools::make_hanging_node_constraints(dofHandler, onlyHangingNodeConstraints);
      onlyHangingNodeConstraints.close();

      std::map<int,std::set<int> > atomToGlobalNodeIdMap;

      for(unsigned int iAtom = 0; iAtom < totalNumberAtoms; ++iAtom)
	{
	  std::set<int> tempNodalSet;
	  dealii::Point<3> atomCoor;

	  if(iAtom < numberGlobalAtoms)
	    {
	      atomCoor[0] = atomLocations[iAtom][2];
	      atomCoor[1] = atomLocations[iAtom][3];
	      atomCoor[2] = atomLocations[iAtom][4];
	    }
	  else
	    {
	      //
	      //Fill with ImageAtom Coors
	      //
	      atomCoor[0] = imagePositions[iAtom-numberGlobalAtoms][0];
	      atomCoor[1] = imagePositions[iAtom-numberGlobalAtoms][1];
	      atomCoor[2] = imagePositions[iAtom-numberGlobalAtoms][2];
	    }

	  // std::cout<<"Atom Coor: "<<atomCoor[0]<<" "<<atomCoor[1]<<" "<<atomCoor[2]<<std::endl;

	  dealii::DoFHandler<3>::active_cell_iterator cell = dofHandler.begin_active(),endc = dofHandler.end();
          std::vector<dealii::types::global_dof_index> cell_dof_indices(dofs_per_cell);

	  for(; cell!= endc; ++cell)
	      if(cell->is_locally_owned())
		{
		  int cutOffFlag = 0;
		  cell->get_dof_indices(cell_dof_indices);

		  for(unsigned int iNode = 0; iNode < dofs_per_cell; ++iNode)
		    {

		      dealii::Point<3> feNodeGlobalCoord = supportPoints[cell_dof_indices[iNode]];
		      double distance = atomCoor.distance(feNodeGlobalCoord);

		      if(distance < radiusAtomBall)
			{
			  cutOffFlag = 1;
			  break;
			}

		    }//element node loop

		  if(cutOffFlag == 1)
		    {
		      for(unsigned int iNode = 0; iNode < vertices_per_cell; ++iNode)
			{
			  unsigned int nodeID=cell->vertex_dof_index(iNode,0);
			  tempNodalSet.insert(nodeID);
			}

		    }

		}//cell locally owned if loop

	  atomToGlobalNodeIdMap[iAtom] = tempNodalSet;

	}//atom loop

      //
      //exchange atomToGlobalNodeIdMap across all processors
      //
      internal::exchangeAtomToGlobalNodeIdMaps(totalNumberAtoms,
				               atomToGlobalNodeIdMap,
				               n_mpi_processes,
				               mpi_communicator);


      //
      //erase keys which have empty values
      //
      for(int iAtom = numberGlobalAtoms; iAtom < totalNumberAtoms; ++iAtom)
	{
	  if(atomToGlobalNodeIdMap[iAtom].empty() == true)
	    {
	      atomToGlobalNodeIdMap.erase(iAtom);
	    }
	}


      //
      //create interaction maps by finding the intersection of global NodeIds of each atom
      //
      std::map<int,std::set<int> > interactionMap;

      for(int iAtom = 0; iAtom < totalNumberAtoms; ++iAtom)
	{
	  //
	  //Add iAtom to the interactionMap corresponding to the key iAtom
	  //
	  if(iAtom < numberGlobalAtoms)
	    interactionMap[iAtom].insert(iAtom);

	  //std::cout<<"IAtom: "<<iAtom<<std::endl;

	  for(int jAtom = iAtom - 1; jAtom > -1; jAtom--)
	    {
	      //std::cout<<"JAtom: "<<jAtom<<std::endl;

	      //
	      //compute intersection between the atomGlobalNodeIdMap of iAtom and jAtom
	      //
	      std::vector<int> nodesIntersection;

	      std::set_intersection(atomToGlobalNodeIdMap[iAtom].begin(),
				    atomToGlobalNodeIdMap[iAtom].end(),
				    atomToGlobalNodeIdMap[jAtom].begin(),
				    atomToGlobalNodeIdMap[jAtom].end(),
				    std::back_inserter(nodesIntersection));

	      // std::cout<<"Size of NodeIntersection: "<<nodesIntersection.size()<<std::endl;

	      if(nodesIntersection.size() > 0)
		{
		  if(iAtom < numberGlobalAtoms && jAtom < numberGlobalAtoms)
		    {
		      //
		      //if both iAtom and jAtom are actual atoms in unit-cell/domain,then iAtom and jAtom are interacting atoms
		      //
		      interactionMap[iAtom].insert(jAtom);
		      interactionMap[jAtom].insert(iAtom);
		    }
		  else if(iAtom < numberGlobalAtoms && jAtom >= numberGlobalAtoms)
		    {
		      //
		      //if iAtom is actual atom in unit-cell and jAtom is imageAtom, find the actual atom for which jAtom is
		      //the image then create the interaction map between that atom and iAtom
		      //
		      int masterAtomId = imageIds[jAtom - numberGlobalAtoms];
		      if(masterAtomId == iAtom)
			{
			  std::cout<<"Atom and its own image is interacting decrease radius"<<std::endl;
			  exit(-1);
			}
		      interactionMap[iAtom].insert(masterAtomId);
		      interactionMap[masterAtomId].insert(iAtom);
		    }
		  else if(iAtom >= numberGlobalAtoms && jAtom < numberGlobalAtoms)
		    {
		      //
		      //if jAtom is actual atom in unit-cell and iAtom is imageAtom, find the actual atom for which iAtom is
		      //the image and then create interaction map between that atom and jAtom
		      //
		      int masterAtomId = imageIds[iAtom - numberGlobalAtoms];
		      if(masterAtomId == jAtom)
			{
			  std::cout<<"Atom and its own image is interacting decrease radius"<<std::endl;
			  exit(-1);
			}
		      interactionMap[masterAtomId].insert(jAtom);
		      interactionMap[jAtom].insert(masterAtomId);

		    }
		  else if(iAtom >= numberGlobalAtoms && jAtom >= numberGlobalAtoms)
		    {
		      //
		      //if both iAtom and jAtom are image atoms in unit-cell iAtom and jAtom are interacting atoms
		      //find the actual atoms for which iAtom and jAtoms are images and create interacting maps between them
		      int masteriAtomId = imageIds[iAtom - numberGlobalAtoms];
		      int masterjAtomId = imageIds[jAtom - numberGlobalAtoms];
		      if(masteriAtomId == masterjAtomId)
			{
			  std::cout<<"Two Image Atoms corresponding to same parent Atoms are interacting decrease radius"<<std::endl;
			  exit(-1);
			}
		      interactionMap[masteriAtomId].insert(masterjAtomId);
		      interactionMap[masterjAtomId].insert(masteriAtomId);
		    }

		}

	    }//end of jAtom loop

	}//end of iAtom loop

      std::map<int,std::set<int> >::iterator iter;

      //
      // start by adding atom 0 to bin 0
      //
      (d_bins[0]).insert(0);
      int binCount = 0;
      // iterate from atom 1 onwards
      for(int i = 1; i < numberGlobalAtoms; ++i){

	const std::set<int> & interactingAtoms = interactionMap[i];
	//
	//treat spl case when no atom intersects with another. e.g. simple cubic
	//
	if(interactingAtoms.size() == 0){
	  (d_bins[binCount]).insert(i);
	  continue;
	}

	bool isBinFound;
	// iterate over each existing bin and see if atom i fits into the bin
	for(iter = d_bins.begin();iter!= d_bins.end();++iter){

	  // pick out atoms in this bin
	  std::set<int>& atomsInThisBin = iter->second;
	  int index = std::distance(d_bins.begin(),iter);

	  isBinFound = true;

	  // to belong to this bin, this atom must not overlap with any other
	  // atom already present in this bin
	  for(std::set<int>::iterator iter2 = interactingAtoms.begin(); iter2!= interactingAtoms.end();++iter2){

	    int atom = *iter2;

	    if(atomsInThisBin.find(atom) != atomsInThisBin.end()){
	      isBinFound = false;
	      break;
	    }
	  }

	  if(isBinFound == true){
	    (d_bins[index]).insert(i);
	    break;
	  }
	}
	// if all current bins have been iterated over w/o a match then
	// create a new bin for this atom
	if(isBinFound == false){
	  binCount++;
	  (d_bins[binCount]).insert(i);
	}
      }


      const int numberBins = binCount + 1;
      if (dftParameters::verbosity==2)
	pcout<<"number bins: "<<numberBins<<std::endl;

      d_imageIdsInBins.resize(numberBins);
      d_boundaryFlag.resize(numberBins);
      d_vselfBinField.resize(numberBins);
      d_closestAtomBin.resize(numberBins);
      d_vselfBinConstraintMatrices.resize(numberBins);
      //
      //set constraint matrices for each bin
      //
      for(int iBin = 0; iBin < numberBins; ++iBin)
	{
	  std::set<int> & atomsInBinSet = d_bins[iBin];
	  std::vector<int> atomsInCurrentBin(atomsInBinSet.begin(),atomsInBinSet.end());
	  std::vector<dealii::Point<3> > atomPositionsInCurrentBin;

	  int numberGlobalAtomsInBin = atomsInCurrentBin.size();

	  std::vector<int> &imageIdsOfAtomsInCurrentBin = d_imageIdsInBins[iBin];
	  std::vector<std::vector<double> > imagePositionsOfAtomsInCurrentBin;

	  if (dftParameters::verbosity==2)
	   pcout<<"bin "<<iBin<< ": number of global atoms: "<<numberGlobalAtomsInBin<<std::endl;

	  for(int index = 0; index < numberGlobalAtomsInBin; ++index)
	    {
	      int globalChargeIdInCurrentBin = atomsInCurrentBin[index];

	      //std:cout<<"Index: "<<index<<"Global Charge Id: "<<globalChargeIdInCurrentBin<<std::endl;

	      dealii::Point<3> atomPosition(atomLocations[globalChargeIdInCurrentBin][2],atomLocations[globalChargeIdInCurrentBin][3],atomLocations[globalChargeIdInCurrentBin][4]);
	      atomPositionsInCurrentBin.push_back(atomPosition);

	      for(int iImageAtom = 0; iImageAtom < numberImageCharges; ++iImageAtom)
		{
		  if(imageIds[iImageAtom] == globalChargeIdInCurrentBin)
		    {
		      imageIdsOfAtomsInCurrentBin.push_back(iImageAtom);
		      std::vector<double> imageChargeCoor = imagePositions[iImageAtom];
		      imagePositionsOfAtomsInCurrentBin.push_back(imageChargeCoor);
		    }
		}

	    }

	  int numberImageAtomsInBin = imageIdsOfAtomsInCurrentBin.size();

	  //
	  //create constraint matrix for current bin
	  //
	  d_vselfBinConstraintMatrices[iBin].reinit(locally_relevant_dofs);


	  unsigned int inNodes=0, outNodes=0;
	  std::map<dealii::types::global_dof_index,dealii::Point<3> >::iterator iterMap;
	  for(iterMap = supportPoints.begin(); iterMap != supportPoints.end(); ++iterMap)
	    {
	      if(locally_relevant_dofs.is_element(iterMap->first))
		{
		  if(!onlyHangingNodeConstraints.is_constrained(iterMap->first))
		    {
		      int overlapFlag = 0;
		      dealii::Point<3> nodalCoor = iterMap->second;
		      std::vector<double> distanceFromNode;

		      for(unsigned int iAtom = 0; iAtom < numberGlobalAtomsInBin+numberImageAtomsInBin; ++iAtom)
			{
                          dealii::Point<3> atomCoor;
			  if(iAtom < numberGlobalAtomsInBin)
			    {
			      atomCoor = atomPositionsInCurrentBin[iAtom];
			    }
			  else
			    {
			      atomCoor[0] = imagePositionsOfAtomsInCurrentBin[iAtom - numberGlobalAtomsInBin][0];
			      atomCoor[1] = imagePositionsOfAtomsInCurrentBin[iAtom - numberGlobalAtomsInBin][1];
			      atomCoor[2] = imagePositionsOfAtomsInCurrentBin[iAtom - numberGlobalAtomsInBin][2];
			    }


			  double distance = nodalCoor.distance(atomCoor);

			  if(distance < radiusAtomBall)
			    overlapFlag += 1;

			  if(overlapFlag > 1)
			    {
			      std::cout<< "One of your Bins has a problem. It has interacting atoms" << std::endl;
			      exit(-1);
			    }

			  distanceFromNode.push_back(distance);

			}//atom loop

		      std::vector<double>::iterator minDistanceIter = std::min_element(distanceFromNode.begin(),
										       distanceFromNode.end());

		      std::iterator_traits<std::vector<double>::iterator>::difference_type minDistanceAtomId = std::distance(distanceFromNode.begin(),
															     minDistanceIter);


		      double minDistance = *minDistanceIter;

		      int chargeId;

		      if(minDistanceAtomId < numberGlobalAtomsInBin)
			chargeId = atomsInCurrentBin[minDistanceAtomId];
		      else
			chargeId = imageIdsOfAtomsInCurrentBin[minDistanceAtomId-numberGlobalAtomsInBin]+numberGlobalAtoms;

		      d_closestAtomBin[iBin][iterMap->first] = chargeId;

		      //FIXME: These two can be moved to the outermost bin loop
		      std::map<dealii::types::global_dof_index, int> & boundaryNodeMap = d_boundaryFlag[iBin];
		      std::map<dealii::types::global_dof_index, double> & vSelfBinNodeMap = d_vselfBinField[iBin];

		      if(minDistance < radiusAtomBall)
			{
			  boundaryNodeMap[iterMap->first] = chargeId;
			  inNodes++;

			  double atomCharge;

			  if(minDistanceAtomId < numberGlobalAtomsInBin)
			    {
			      if(dftParameters::isPseudopotential)
				atomCharge = atomLocations[chargeId][1];
			      else
				atomCharge = atomLocations[chargeId][0];
			    }
			  else
			    atomCharge = imageCharges[imageIdsOfAtomsInCurrentBin[minDistanceAtomId-numberGlobalAtomsInBin]];

			  if(minDistance <= 1e-05)
			    {
			      vSelfBinNodeMap[iterMap->first] = 0.0;
			    }
			  else
			    {
			      vSelfBinNodeMap[iterMap->first] = -atomCharge/minDistance;
			    }

			}
		      else
			{
			  double atomCharge;

			  if(minDistanceAtomId < numberGlobalAtomsInBin)
			    {
			      if(dftParameters::isPseudopotential)
				atomCharge = atomLocations[chargeId][1];
			      else
				atomCharge = atomLocations[chargeId][0];
			    }
			  else
			    atomCharge = imageCharges[imageIdsOfAtomsInCurrentBin[minDistanceAtomId-numberGlobalAtomsInBin]];

			  const double potentialValue = -atomCharge/minDistance;

			  boundaryNodeMap[iterMap->first] = -1;
			  vSelfBinNodeMap[iterMap->first] = potentialValue;
			  outNodes++;

			}//else loop

		    }//non-hanging node check

		}//locally relevant dofs

	    }//nodal loop

	   //First apply correct dirichlet boundary conditions on elements with atleast one solved node
	  dealii::DoFHandler<3>::active_cell_iterator cell = dofHandler.begin_active(),endc = dofHandler.end();
	   for(; cell!= endc; ++cell)
	    {
	      if(cell->is_locally_owned() || cell->is_ghost())
	      {

		  std::vector<dealii::types::global_dof_index> cell_dof_indices(dofs_per_cell);
		  cell->get_dof_indices(cell_dof_indices);

		  int closestChargeIdSolvedNode=-1;
		  bool isDirichletNodePresent=false;
		  bool isSolvedNodePresent=false;
		  int numSolvedNodes=0;
		  int closestChargeIdSolvedSum=0;
		  for(unsigned int iNode = 0; iNode < dofs_per_cell; ++iNode)
		    {
		      const int globalNodeId=cell_dof_indices[iNode];
		      if(!onlyHangingNodeConstraints.is_constrained(globalNodeId))
		      {
			const int boundaryId=d_boundaryFlag[iBin][globalNodeId];
			if(boundaryId == -1)
			{
			    isDirichletNodePresent=true;
			}
			else
			{
			    isSolvedNodePresent=true;
			    closestChargeIdSolvedNode=boundaryId;
			    numSolvedNodes++;
			    closestChargeIdSolvedSum+=boundaryId;
			}
		      }

		    }//element node loop



		  if(isDirichletNodePresent && isSolvedNodePresent)
		    {
		       double closestAtomChargeSolved;
		       dealii::Point<3> closestAtomLocationSolved;
		       Assert(numSolvedNodes*closestChargeIdSolvedNode==closestChargeIdSolvedSum,dealii::ExcMessage("BUG"));
		       if(closestChargeIdSolvedNode < numberGlobalAtoms)
		       {
			   closestAtomLocationSolved[0]=atomLocations[closestChargeIdSolvedNode][2];
			   closestAtomLocationSolved[1]=atomLocations[closestChargeIdSolvedNode][3];
			   closestAtomLocationSolved[2]=atomLocations[closestChargeIdSolvedNode][4];
			   if(dftParameters::isPseudopotential)
			      closestAtomChargeSolved = atomLocations[closestChargeIdSolvedNode][1];
			   else
			      closestAtomChargeSolved = atomLocations[closestChargeIdSolvedNode][0];
		       }
		       else
		       {
			   const int imageId=closestChargeIdSolvedNode-numberGlobalAtoms;
			   closestAtomChargeSolved = imageCharges[imageId];
			   closestAtomLocationSolved[0]=imagePositions[imageId][0];
			   closestAtomLocationSolved[1]=imagePositions[imageId][1];
			   closestAtomLocationSolved[2]=imagePositions[imageId][2];
		       }
		      for(unsigned int iNode = 0; iNode < dofs_per_cell; ++iNode)
			{

			  const int globalNodeId=cell_dof_indices[iNode];
			  const int boundaryId=d_boundaryFlag[iBin][globalNodeId];
			  if(!onlyHangingNodeConstraints.is_constrained(globalNodeId) && !d_vselfBinConstraintMatrices[iBin].is_constrained(globalNodeId) && boundaryId==-1)
			  {
			    d_vselfBinConstraintMatrices[iBin].add_line(globalNodeId);
			    const double distance=supportPoints[globalNodeId].distance(closestAtomLocationSolved);
			    const double newPotentialValue =-closestAtomChargeSolved/distance;
			    d_vselfBinConstraintMatrices[iBin].set_inhomogeneity(globalNodeId,newPotentialValue);
			    d_vselfBinField[iBin][globalNodeId] = newPotentialValue;
			    d_closestAtomBin[iBin][globalNodeId] = closestChargeIdSolvedNode;
			    //outNodes2++;
			  }//check non hanging node and vself consraints not already set
			}//element node loop

		    }//check if element has atleast one dirichlet node and atleast one solved node
	      }//cell locally owned or ghost
	    }// cell loop


	   //Next apply correct dirichlet boundary conditions on elements with all dirichlet nodes
	   cell = dofHandler.begin_active();
	   for(; cell!= endc; ++cell) {
	      if(cell->is_locally_owned() || cell->is_ghost())
	      {

		  std::vector<dealii::types::global_dof_index> cell_dof_indices(dofs_per_cell);
		  cell->get_dof_indices(cell_dof_indices);

		  bool isDirichletNodePresent=false;
		  bool isSolvedNodePresent=false;
		  for(unsigned int iNode = 0; iNode < dofs_per_cell; ++iNode)
		    {
		      const int globalNodeId=cell_dof_indices[iNode];
		      if(!onlyHangingNodeConstraints.is_constrained(globalNodeId))
		      {
			const int boundaryId=d_boundaryFlag[iBin][globalNodeId];
			if(boundaryId == -1)
			{
			    isDirichletNodePresent=true;
			}
			else
			{
			    isSolvedNodePresent=true;
			}
		      }

		    }//element node loop

		  if(isDirichletNodePresent && !isSolvedNodePresent)
		    {
		      for(unsigned int iNode = 0; iNode < dofs_per_cell; ++iNode)
			{

			  const unsigned int globalNodeId=cell_dof_indices[iNode];
			  const int boundaryId=d_boundaryFlag[iBin][globalNodeId];
			  if(!onlyHangingNodeConstraints.is_constrained(globalNodeId) && !d_vselfBinConstraintMatrices[iBin].is_constrained(globalNodeId) && boundaryId==-1)
			  {
			    d_vselfBinConstraintMatrices[iBin].add_line(globalNodeId);
			    d_vselfBinConstraintMatrices[iBin].set_inhomogeneity(globalNodeId,d_vselfBinField[iBin][globalNodeId]);
			    //outNodes2++;
			  }//check non hanging node and vself consraints not already set
			}//element node loop

		    }//check if element has atleast one dirichlet node and atleast one solved node
	      }//cell locally owned or ghost
	    } //cell loop

	    d_vselfBinConstraintMatrices[iBin].merge(constraintMatrix,dealii::ConstraintMatrix::MergeConflictBehavior::left_object_wins);
	    d_vselfBinConstraintMatrices[iBin].close(); //pcout<<" ibin: "<<iBin <<" size of constraints: "<< constraintsForVselfInBin->n_constraints()<<std::endl;
	    constraintsVector.push_back(&(d_vselfBinConstraintMatrices[iBin]));

	}//bin loop

        createAtomBinsSanityCheck(dofHandler,onlyHangingNodeConstraints);
        locateAtomsInBins(dofHandler);

      return;

    }//

    template<unsigned int FEOrder>
    void vselfBinsManager<FEOrder>::locateAtomsInBins(const dealii::DoFHandler<3> & dofHandler)
    {
      d_atomsInBin.clear();

      dealii::IndexSet  locally_owned_dofs=dofHandler.locally_owned_dofs();

      const unsigned int numberBins = d_boundaryFlag.size();
      d_atomsInBin.resize(numberBins);


      for(int iBin = 0; iBin < numberBins; ++iBin)
	{
	  unsigned int vertices_per_cell=dealii::GeometryInfo<3>::vertices_per_cell;
	  dealii::DoFHandler<3>::active_cell_iterator cell = dofHandler.begin_active(),endc = dofHandler.end();

	  std::set<int> & atomsInBinSet = d_bins[iBin];
	  std::vector<int> atomsInCurrentBin(atomsInBinSet.begin(),atomsInBinSet.end());
	  unsigned int numberGlobalAtomsInBin = atomsInCurrentBin.size();
	  std::set<unsigned int> atomsTolocate;
	  for (unsigned int i = 0; i < numberGlobalAtomsInBin; i++) atomsTolocate.insert(i);

	  for (; cell!=endc; ++cell) {
	    if (cell->is_locally_owned()){
	      for (unsigned int i=0; i<vertices_per_cell; ++i){
		const dealii::types::global_dof_index nodeID=cell->vertex_dof_index(i,0);
		dealii::Point<3> feNodeGlobalCoord = cell->vertex(i);
		//
		//loop over all atoms to locate the corresponding nodes
		//
		for (std::set<unsigned int>::iterator it=atomsTolocate.begin(); it!=atomsTolocate.end(); ++it)
		  {
		    const int chargeId = atomsInCurrentBin[*it];
		    dealii::Point<3> atomCoord(d_atomLocations[chargeId][2],d_atomLocations[chargeId][3],d_atomLocations[chargeId][4]);
		    if(feNodeGlobalCoord.distance(atomCoord) < 1.0e-5){
		      if(dftParameters::isPseudopotential)
		      {
			if (dftParameters::verbosity==2)
			  std::cout << "atom core in bin " << iBin<<" with valence charge "<<d_atomLocations[chargeId][1] << " located with node id " << nodeID << " in processor " << this_mpi_process;
		      }
		      else
		      {
			if (dftParameters::verbosity==2)
			  std::cout << "atom core in bin " << iBin<<" with charge "<<d_atomLocations[chargeId][0] << " located with node id " << nodeID << " in processor " << this_mpi_process;
		      }
		      if (locally_owned_dofs.is_element(nodeID)){
			if(dftParameters::isPseudopotential)
			  d_atomsInBin[iBin].insert(std::pair<dealii::types::global_dof_index,double>(nodeID,d_atomLocations[chargeId][1]));
			else
			  d_atomsInBin[iBin].insert(std::pair<dealii::types::global_dof_index,double>(nodeID,d_atomLocations[chargeId][0]));
			if (dftParameters::verbosity==2)
			   std::cout << " and added \n";
		      }
		      else
		      {
			if (dftParameters::verbosity==2)
			   std::cout << " but skipped \n";
		      }
		      atomsTolocate.erase(*it);
		      break;
		    }//tolerance check if loop
		  }//atomsTolocate loop
	      }//vertices_per_cell loop
	    }//locally owned cell if loop
	  }//cell loop
	  MPI_Barrier(mpi_communicator);
	}//iBin loop
    }

    template<unsigned int FEOrder>
    const std::map<int,std::set<int> > & vselfBinsManager<FEOrder>::getAtomIdsBins() const {return d_bins;}

    template<unsigned int FEOrder>
    const std::vector<std::vector<int> > & vselfBinsManager<FEOrder>::getImageIdsBins() const {return d_imageIdsInBins;}

    template<unsigned int FEOrder>
    const std::vector<std::map<dealii::types::global_dof_index, int> > & vselfBinsManager<FEOrder>::getBoundaryFlagsBins() const {return d_boundaryFlag;}

    template<unsigned int FEOrder>
    const std::vector<std::map<dealii::types::global_dof_index, int> > & vselfBinsManager<FEOrder>::getClosestAtomIdsBins() const {return d_closestAtomBin;}

    template<unsigned int FEOrder>
    const std::vector<vectorType> & vselfBinsManager<FEOrder>::getVselfFieldBins() const {return d_vselfFieldBins;}

    template class vselfBinsManager<1>;
    template class vselfBinsManager<2>;
    template class vselfBinsManager<3>;
    template class vselfBinsManager<4>;
    template class vselfBinsManager<5>;
    template class vselfBinsManager<6>;
    template class vselfBinsManager<7>;
    template class vselfBinsManager<8>;
    template class vselfBinsManager<9>;
    template class vselfBinsManager<10>;
    template class vselfBinsManager<11>;
    template class vselfBinsManager<12>;
}
