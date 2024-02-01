#!/bin/bash
# Nikhil script to setup and build DFT-FE.

set -e
set -o pipefail

if [ -s CMakeLists.txt ]; then
    echo "This script must be run from the build directory!"
    exit 1
fi

# Path to project source
SRC=`dirname $0` # location of source directory

########################################################################
#Provide paths below for external libraries, compiler options and flags,
# and optimization flag

#Paths for required external libraries
dealiiDir="/nlsasfs/home/numerical/nikhilk/spack/spack/opt/spack/linux-ubuntu20.04-zen2/gcc-11.3.0/dealii-9.4.2-vi7acvmlbkezfg52kmcql4deekgl2ry2"
alglibDir="/nlsasfs/home/numerical/nikhilk/spack/spack/opt/spack/linux-ubuntu20.04-zen2/gcc-11.3.0/alglib-3.20.0-deiq5fszrixaq3g43duxmeq4qiuhsixm"
libxcDir="/nlsasfs/home/numerical/nikhilk/spack/spack/opt/spack/linux-ubuntu20.04-zen2/gcc-11.3.0/libxc-6.1.0-kxtajf7qw2quyywao3eyxc4ksrpdstpx"
spglibDir="/nlsasfs/home/numerical/nikhilk/spack/spack/opt/spack/linux-ubuntu20.04-zen2/gcc-11.3.0/spglib-2.0.2-gti277mstj4vmms4xjf3epzf7cbi6g5t"
xmlIncludeDir="/nlsasfs/home/numerical/nikhilk/spack/spack/opt/spack/linux-ubuntu20.04-zen2/gcc-11.3.0/libxml2-2.10.3-4bjhnxbs7qcx7vvntcml7z3mordvxpi6/include/libxml2"
xmlLibDir="/nlsasfs/home/numerical/nikhilk/spack/spack/opt/spack/linux-ubuntu20.04-zen2/gcc-11.3.0/libxml2-2.10.3-4bjhnxbs7qcx7vvntcml7z3mordvxpi6/lib"
ELPA_PATH="/nlsasfs/home/numerical/nikhilk/spack/spack/opt/spack/linux-ubuntu20.04-zen2/gcc-11.3.0/elpa-2022.11.001-bpdcr523mqsnw2idy4g6yjllsyf522w5"
dftdpath="/nlsasfs/home/numerical/nikhilk/spack/dftd/install"
numdiffdir="/nlsasfs/home/numerical/nikhilk/spack/spack/opt/spack/linux-ubuntu20.04-zen2/gcc-11.3.0/numdiff-5.9.0-frza6cdkjn2atzvz5rbl3uufmjnzmj32"


#Paths for optional external libraries
# path for NCCL/RCCL libraries
DCCL_PATH="/nlsasfs/home/numerical/nikhilk/spack/spack/opt/spack/linux-ubuntu20.04-zen2/gcc-11.3.0/nccl-2.19.3-1-ethunrs7hudce5naltqm62e37uc2537p"
mdiPath=""

#Toggle GPU compilation
withGPU=ON
gpuLang="cuda"     # Use "cuda"/"hip"
gpuVendor="nvidia" # Use "nvidia/amd"
withGPUAwareMPI=ON #Please use this option with care
                   #Only use if the machine supports
                   #device aware MPI and is profiled
                   #to be fast

#Option to link to DCCL library (Only for GPU compilation)
withDCCL=ON
withMDI=OFF
# withTorch=OFF
# withCustomizedDealii=ON

#Compiler options and flags
cxx_compiler=mpic++  #sets DCMAKE_CXX_COMPILER
cxx_flags="-fPIC" #sets DCMAKE_CXX_FLAGS
cxx_flagsRelease="-O3 -march=native" #sets DCMAKE_CXX_FLAGS_RELEASE
device_flags="-arch=sm_80 -O3 -ccbin=$cxx_compiler" # set DCMAKE_CXX_CUDA/HIP_FLAGS
                           #(only applicable for withGPU=ON)
device_architectures="80" # set DCMAKE_CXX_CUDA/HIP_ARCHITECTURES
                           #(only applicable for withGPU=ON)


#Option to compile with default or higher order quadrature for storing pseudopotential data
#ON is recommended for MD simulations with hard pseudopotentials
withHigherQuadPSP=OFF

# build type: "Release" or "Debug"
build_type=Release

testing=OFF
minimal_compile=ON
###########################################################################
#Usually, no changes are needed below this line
#

#if [[ x"$build_type" == x"Release" ]]; then
#  c_flags="$c_flagsRelease"
#  cxx_flags="$c_flagsRelease"
#else
#fi
out=`echo "$build_type" | tr '[:upper:]' '[:lower:]'`

function cmake_configure() {
  if [ "$gpuLang" = "cuda" ]; then
    cmake -G Ninja -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_COMPILER=$cxx_compiler \
    -DCMAKE_CXX_FLAGS="$cxx_flags" \
    -DCMAKE_CXX_FLAGS_RELEASE="$cxx_flagsRelease" \
    -DCMAKE_BUILD_TYPE=$build_type -DDEAL_II_DIR=$dealiiDir \
    -DALGLIB_DIR=$alglibDir -DLIBXC_DIR=$libxcDir \
    -DSPGLIB_DIR=$spglibDir -DXML_LIB_DIR=$xmlLibDir \
    -DXML_INCLUDE_DIR=$xmlIncludeDir \
    -DWITH_MDI=$withMDI -DMDI_PATH=$mdiPath \
    -DWITH_DCCL=$withDCCL -DCMAKE_PREFIX_PATH="$ELPA_PATH;$DCCL_PATH" \
    -DWITH_COMPLEX=$withComplex -DWITH_GPU=$withGPU -DGPU_LANG=$gpuLang -DGPU_VENDOR=$gpuVendor -DWITH_GPU_AWARE_MPI=$withGPUAwareMPI -DCMAKE_CUDA_FLAGS="$device_flags" -DCMAKE_CUDA_ARCHITECTURES="$device_architectures" \
    -DWITH_TESTING=$testing -DMINIMAL_COMPILE=$minimal_compile \
    -DHIGHERQUAD_PSP=$withHigherQuadPSP $1
  elif [ "$gpuLang" = "hip" ]; then
    cmake -G Ninja -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_COMPILER=$cxx_compiler\
    -DCMAKE_CXX_FLAGS="$cxx_flags" \
    -DCMAKE_CXX_FLAGS_RELEASE="$cxx_flagsRelease" \
    -DCMAKE_BUILD_TYPE=$build_type -DDEAL_II_DIR=$dealiiDir \
    -DALGLIB_DIR=$alglibDir -DLIBXC_DIR=$libxcDir \
    -DSPGLIB_DIR=$spglibDir -DXML_LIB_DIR=$xmlLibDir \
    -DXML_INCLUDE_DIR=$xmlIncludeDir \
    -DWITH_MDI=$withMDI -DMDI_PATH=$mdiPath \
    -DWITH_DCCL=$withDCCL -DCMAKE_PREFIX_PATH="$ELPA_PATH;$DCCL_PATH" \
    -DWITH_COMPLEX=$withComplex -DWITH_GPU=$withGPU -DGPU_LANG=$gpuLang -DGPU_VENDOR=$gpuVendor -DWITH_GPU_AWARE_MPI=$withGPUAwareMPI -DCMAKE_HIP_FLAGS="$device_flags" -DCMAKE_HIP_ARCHITECTURES="$device_architectures" \
    -DWITH_TESTING=$testing -DMINIMAL_COMPILE=$minimal_compile \
    -DHIGHERQUAD_PSP=$withHigherQuadPSP $1
  else
    cmake -G Ninja -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_COMPILER=$cxx_compiler \
    -DCMAKE_CXX_FLAGS="$cxx_flags" \
    -DCMAKE_CXX_FLAGS_RELEASE="$cxx_flagsRelease" \
    -DCMAKE_BUILD_TYPE=$build_type -DDEAL_II_DIR=$dealiiDir \
    -DALGLIB_DIR=$alglibDir -DLIBXC_DIR=$libxcDir \
    -DSPGLIB_DIR=$spglibDir -DXML_LIB_DIR=$xmlLibDir \
    -DXML_INCLUDE_DIR=$xmlIncludeDir \
    -DWITH_MDI=$withMDI -DMDI_PATH=$mdiPath \
    -DWITH_DCCL=$withDCCL -DCMAKE_PREFIX_PATH="$ELPA_PATH;$DCCL_PATH" \
    -DWITH_COMPLEX=$withComplex \
    -DWITH_TESTING=$testing -DMINIMAL_COMPILE=$minimal_compile \
    -DHIGHERQUAD_PSP=$withHigherQuadPSP $1
  fi
}

RCol='\e[0m'
Blu='\e[0;34m';
if [ -d "$out" ]; then # build directory exists
    echo -e "${Blu}$out directory already present${RCol}"
else
    rm -rf "$out"
    echo -e "${Blu}Creating $out ${RCol}"
    mkdir -p "$out"
fi

cd $out

withComplex=OFF
echo -e "${Blu}Building Real executable in $build_type mode...${RCol}"
mkdir -p real && cd real
cmake_configure "$SRC" && ninja
cd ..

# withComplex=ON
# echo -e "${Blu}Building Complex executable in $build_type mode...${RCol}"
# mkdir -p complex && cd complex
# cmake_configure "$SRC" && ninja
# cd ..

echo -e "${Blu}Build complete.${RCol}"
