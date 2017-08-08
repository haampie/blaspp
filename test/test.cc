#include "test.hh"

#include "blas.hh"

//#include <mkl.h>

// -----------------------------------------------------------------------------
int main()
{
    // Level 1 BLAS
    test_asum();
    test_axpy();
    test_copy();
    test_dot();
    test_iamax();
    test_nrm2();
    test_rot();
    test_rotg();
    test_rotm();
    test_rotmg();
    test_scal();
    test_swap();

    // Level 2 BLAS
    test_gemv();
    test_symv();
    test_hemv();

    return 0;
}
