#ifndef BLAS_HH
#define BLAS_HH

#include "blas_wrappers.hh"

// =============================================================================
// Level 1 BLAS template implementations

#include "asum.hh"
#include "axpy.hh"
#include "copy.hh"
#include "dot.hh"
#include "iamax.hh"
#include "nrm2.hh"
#include "rot.hh"
#include "rotg.hh"
#include "rotm.hh"
#include "rotmg.hh"
#include "scal.hh"
#include "swap.hh"

// =============================================================================
// Level 2 BLAS template implementations

#include "gemv.hh"
#include "ger.hh"
#include "geru.hh"
#include "hemv.hh"
#include "her.hh"
#include "her2.hh"
#include "symv.hh"
#include "syr.hh"
#include "syr2.hh"
#include "trmv.hh"
#include "trsv.hh"

// =============================================================================
// Level 3 BLAS template implementations

#include "gemm.hh"
#include "hemm.hh"
#include "herk.hh"
#include "her2k.hh"
#include "symm.hh"
#include "syrk.hh"
#include "syr2k.hh"
#include "trmm.hh"
#include "trsm.hh"

#endif        //  #ifndef BLAS_HH
