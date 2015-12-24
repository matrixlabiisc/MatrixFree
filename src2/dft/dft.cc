//Include header files
#include "../../include2/headers.h"
#include "../../include2/dft.h"
#include "../../utils/fileReaders.cc"
#include "../poisson/poisson.cc"
#include "../eigen/eigen.cc"
#include "mesh.cc"
#include "init.cc"
#include "psiInitialGuess.cc"
#include "energy.cc"
#include "charge.cc"
#include "density.cc"
#include "locatenodes.cc"
#include "mixingschemes.cc"
#include "chebyshev.cc"
 
//dft constructor
dftClass::dftClass():
  triangulation (MPI_COMM_WORLD),
  FE (QGaussLobatto<1>(FEOrder+1)),
  dofHandler (triangulation),
  mpi_communicator (MPI_COMM_WORLD),
  n_mpi_processes (Utilities::MPI::n_mpi_processes(mpi_communicator)),
  this_mpi_process (Utilities::MPI::this_mpi_process(mpi_communicator)),
  poisson(this),
  eigen(this),
  pcout (std::cout, (Utilities::MPI::this_mpi_process(MPI_COMM_WORLD) == 0)),
  computing_timer (pcout, TimerOutput::summary, TimerOutput::wall_times),
  bLow(0.0),
  a0(lowerEndWantedSpectrum)
{
  //set size of eigenvalues and eigenvectors data structures
  eigenValues.resize(numEigenValues);
  for (unsigned int i=0; i<numEigenValues; ++i){
    eigenVectors.push_back(new vectorType);
    PSI.push_back(new vectorType);
    tempPSI.push_back(new vectorType);
    tempPSI2.push_back(new vectorType);
    tempPSI3.push_back(new vectorType);
  } 
}

//dft run
void dftClass::run (){
  pcout << "number of MPI processes: "
	<< Utilities::MPI::n_mpi_processes(mpi_communicator)
	<< std::endl;
  //read coordinates file

  //fill wavefunctions info (Z, n, l, m, pointID) vector and coordinate points vector
  
  
  //generate mesh
  //if meshFile provided, pass to mesh()
  mesh();

  //initialize
  init();
 
  //solve
  computing_timer.enter_section("dft solve"); 
  //phiExt with nuclear charge
  poisson.solve(poisson.phiExt);
  
  /*
  DataOut<3> data_out;
  data_out.attach_dof_handler (dofHandler);
  data_out.add_data_vector (poisson.phiExt, "solution");
  data_out.build_patches ();
  std::ofstream output ("poisson.vtu");
  data_out.write_vtu (output);
  */

  //Begin SCF iteration
  unsigned int scfIter=0;
  double norm=1.0;
  while ((norm>1.0e-13) && (scfIter<numSCFIterations)){
    if(this_mpi_process==0) printf("\n\nBegin SCF Iteration:%u\n", scfIter+1);
    //Mixing scheme
    if (scfIter>0){
      if (scfIter==1) norm=mixing_simple();
      else norm=mixing_anderson();
      if(this_mpi_process==0) printf("Mixing Scheme: iter:%u, norm:%12.6e\n", scfIter+1, norm);
    }
    //phiTot with rhoIn
    poisson.solve(poisson.phiTotRhoIn, rhoInValues);
    //eigen solve
    eigen.computeVEff(rhoInValues, poisson.phiTotRhoIn); 
    chebyshevSolver();
    //fermi energy
    compute_fermienergy();
    //rhoOut
    compute_rhoOut();
    //phiTot with rhoOut
    poisson.solve(poisson.phiTotRhoOut, rhoOutValues);
    //energy
    compute_energy();
    pcout<<"SCF iteration: " << scfIter+1 << " complete\n";
    scfIter++;
  }
  computing_timer.exit_section("dft solve"); 
}

