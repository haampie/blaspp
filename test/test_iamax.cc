#include <omp.h>

#include "test.hh"
#include "cblas.hh"
#include "lapack_tmp.hh"
#include "blas_flops.hh"
#include "print_matrix.hh"

#include "iamax.hh"

// -----------------------------------------------------------------------------
template< typename T >
void test_iamax_work( Params& params, bool run )
{
    using namespace blas;
    typedef typename traits< T >::real_t real_t;
    typedef long long lld;

    // get & mark input values
    int64_t n       = params.dim.n();
    int64_t incx    = params.incx.value();
    int64_t verbose = params.verbose.value();

    // mark non-standard output values
    params.gbytes.value();
    params.ref_time.value();
    params.ref_gflops.value();
    params.ref_gbytes.value();

    // adjust header to msec
    params.time.name( "BLAS++\ntime (ms)" );
    params.ref_time.name( "Ref.\ntime (ms)" );

    if ( ! run)
        return;

    // setup
    size_t size_x = (n - 1) * abs(incx) + 1;
    T* x = new T[ size_x ];

    int64_t idist = 1;
    int iseed[4] = { 0, 0, 0, 1 };
    lapack_larnv( idist, iseed, size_x, x );

    // test error exits
    assert_throw( blas::iamax( -1, x, incx ), blas::Error );
    assert_throw( blas::iamax(  n, x,    0 ), blas::Error );
    assert_throw( blas::iamax(  n, x,   -1 ), blas::Error );

    if (verbose >= 1) {
        printf( "\n"
                "x n=%5lld, inc=%5lld, size=%10lld\n",
                (lld) n, (lld) incx, (lld) size_x );
    }
    if (verbose >= 2) {
        printf( "x = " ); print_vector( n, x, incx );
    }

    // run test
    libtest::flush_cache( params.cache.value() );
    double time = omp_get_wtime();
    int64_t result = blas::iamax( n, x, incx );
    time = omp_get_wtime() - time;

    double gflop = Gflop < real_t >::iamax( n );
    double gbyte = Gbyte < real_t >::iamax( n );
    params.time.value()   = time * 1000;  // msec
    params.gflops.value() = gflop / time;
    params.gbytes.value() = gbyte / time;

    if (verbose >= 1) {
        printf( "result = %5lld\n", (lld) result );
    }

    if (params.check.value() == 'y') {
        // run reference
        libtest::flush_cache( params.cache.value() );
        time = omp_get_wtime();
        int64_t ref = cblas_iamax( n, x, incx );
        time = omp_get_wtime() - time;

        params.ref_time.value()   = time * 1000;  // msec
        params.ref_gflops.value() = gflop / time;
        params.ref_gbytes.value() = gbyte / time;

        if (verbose >= 1) {
            printf( "ref    = %5lld\n", (lld) ref );
        }

        // error = |ref - result|
        real_t error = std::abs( ref - result );
        params.error.value() = error;

        // iamax must be exact!
        params.okay.value() = (error == 0);
    }

    delete[] x;
}

// -----------------------------------------------------------------------------
void test_iamax( Params& params, bool run )
{
    switch (params.datatype.value()) {
        case libtest::DataType::Integer:
            //test_iamax_work< int64_t >( params, run );
            throw std::exception();
            break;

        case libtest::DataType::Single:
            test_iamax_work< float >( params, run );
            break;

        case libtest::DataType::Double:
            test_iamax_work< double >( params, run );
            break;

        case libtest::DataType::SingleComplex:
            test_iamax_work< std::complex<float> >( params, run );
            break;

        case libtest::DataType::DoubleComplex:
            test_iamax_work< std::complex<double> >( params, run );
            break;
    }
}
