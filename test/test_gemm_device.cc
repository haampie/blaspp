#include "test_device.hh"
#include "cblas.hh"
#include "lapack_tmp.hh"
#include "blas_flops.hh"
#include "print_matrix.hh"
#include "check_gemm.hh"


#include "gemm_device.hh"
#include "trsm_device.hh"
#include "trmm_device.hh"
#include "symm_device.hh"
#include "hemm_device.hh"
#include "syrk_device.hh"
#include "herk_device.hh"
#include "syr2k_device.hh"
#include "her2k_device.hh"
// -----------------------------------------------------------------------------
template< typename TA, typename TB, typename TC >
void test_gemm_device_work( Params& params, bool run )
{
    using namespace libtest;
    using namespace blas;
    using scalar_t = blas::scalar_type< TA, TB, TC >;
    using real_t = blas::real_type< scalar_t >;
    typedef long long lld;

    // get & mark input values
    blas::Layout layout = params.layout.value();
    blas::Op transA = params.transA.value();
    blas::Op transB = params.transB.value();
    scalar_t alpha  = params.alpha.value();
    scalar_t beta   = params.beta.value();
    int64_t m       = params.dim.m();
    int64_t n       = params.dim.n();
    int64_t k       = params.dim.k();
    int64_t align   = params.align.value();
    int64_t verbose = params.verbose.value();

    // mark non-standard output values
    params.ref_time.value();
    params.ref_gflops.value();

    if ( ! run)
        return;

    // setup
    int64_t Am = (transA == Op::NoTrans ? m : k);
    int64_t An = (transA == Op::NoTrans ? k : m);
    int64_t Bm = (transB == Op::NoTrans ? k : n);
    int64_t Bn = (transB == Op::NoTrans ? n : k);
    int64_t Cm = m;
    int64_t Cn = n;
    if (layout == Layout::RowMajor) {
        std::swap( Am, An );
        std::swap( Bm, Bn );
        std::swap( Cm, Cn );
    }
    int64_t lda = roundup( Am, align );
    int64_t ldb = roundup( Bm, align );
    int64_t ldc = roundup( Cm, align );
    size_t size_A = size_t(lda)*An;
    size_t size_B = size_t(ldb)*Bn;
    size_t size_C = size_t(ldc)*Cn;
    TA* A    = new TA[ size_A ];
    TB* B    = new TB[ size_B ];
    TC* C    = new TC[ size_C ];
    TC* Cref = new TC[ size_C ];

    // device specifics 
    blas::Queue queue(0,0);
    TA* dA; 
    TB* dB; 
    TC* dC;
 
    dA = blas::device_malloc<TA>(size_A);
    dB = blas::device_malloc<TB>(size_B);
    dC = blas::device_malloc<TC>(size_C);
    
    int64_t idist = 1;
    int iseed[4] = { 0, 0, 0, 1 };
    lapack_larnv( idist, iseed, size_A, A );
    lapack_larnv( idist, iseed, size_B, B );
    lapack_larnv( idist, iseed, size_C, C );
    lapack_lacpy( "g", Cm, Cn, C, ldc, Cref, ldc );

    blas::device_setmatrix(Am, An, A, lda, dA, lda, queue);
    blas::device_setmatrix(Bm, Bn, B, ldb, dB, ldb, queue);
    blas::device_setmatrix(Cm, Cn, C, ldc, dC, ldc, queue);
    queue.sync();

    // norms for error check
    real_t work[1];
    real_t Anorm = lapack_lange( "f", Am, An, A, lda, work );
    real_t Bnorm = lapack_lange( "f", Bm, Bn, B, ldb, work );
    real_t Cnorm = lapack_lange( "f", Cm, Cn, C, ldc, work );

    // test error exits
    assert_throw( blas::gemm( Layout(0), transA, transB,  m,  n,  k, alpha, dA, lda, dB, ldb, beta, dC, ldc, queue ), blas::Error );
    assert_throw( blas::gemm( layout,    Op(0),  transB,  m,  n,  k, alpha, dA, lda, dB, ldb, beta, dC, ldc, queue ), blas::Error );
    assert_throw( blas::gemm( layout,    transA, Op(0),   m,  n,  k, alpha, dA, lda, dB, ldb, beta, dC, ldc, queue ), blas::Error );
    assert_throw( blas::gemm( layout,    transA, transB, -1,  n,  k, alpha, dA, lda, dB, ldb, beta, dC, ldc, queue ), blas::Error );
    assert_throw( blas::gemm( layout,    transA, transB,  m, -1,  k, alpha, dA, lda, dB, ldb, beta, dC, ldc, queue ), blas::Error );
    assert_throw( blas::gemm( layout,    transA, transB,  m,  n, -1, alpha, dA, lda, dB, ldb, beta, dC, ldc, queue ), blas::Error );
    
    assert_throw( blas::gemm( Layout::ColMajor, Op::NoTrans,   Op::NoTrans, m, n, k, alpha, dA, m-1, dB, ldb, beta, dC, ldc, queue ), blas::Error );
    assert_throw( blas::gemm( Layout::ColMajor, Op::Trans,     Op::NoTrans, m, n, k, alpha, dA, k-1, dB, ldb, beta, dC, ldc, queue ), blas::Error );
    assert_throw( blas::gemm( Layout::ColMajor, Op::ConjTrans, Op::NoTrans, m, n, k, alpha, dA, k-1, dB, ldb, beta, dC, ldc, queue ), blas::Error );

    assert_throw( blas::gemm( Layout::RowMajor, Op::NoTrans,   Op::NoTrans, m, n, k, alpha, dA, k-1, dB, ldb, beta, dC, ldc, queue ), blas::Error );
    assert_throw( blas::gemm( Layout::RowMajor, Op::Trans,     Op::NoTrans, m, n, k, alpha, dA, m-1, dB, ldb, beta, dC, ldc, queue ), blas::Error );
    assert_throw( blas::gemm( Layout::RowMajor, Op::ConjTrans, Op::NoTrans, m, n, k, alpha, dA, m-1, dB, ldb, beta, dC, ldc, queue ), blas::Error );

    assert_throw( blas::gemm( Layout::ColMajor, Op::NoTrans, Op::NoTrans,   m, n, k, alpha, dA, lda, B, k-1, beta, dC, ldc, queue ), blas::Error );
    assert_throw( blas::gemm( Layout::ColMajor, Op::NoTrans, Op::Trans,     m, n, k, alpha, dA, lda, B, n-1, beta, dC, ldc, queue ), blas::Error );
    assert_throw( blas::gemm( Layout::ColMajor, Op::NoTrans, Op::ConjTrans, m, n, k, alpha, dA, lda, B, n-1, beta, dC, ldc, queue ), blas::Error );

    assert_throw( blas::gemm( Layout::RowMajor, Op::NoTrans, Op::NoTrans,   m, n, k, alpha, dA, lda, B, n-1, beta, dC, ldc, queue ), blas::Error );
    assert_throw( blas::gemm( Layout::RowMajor, Op::NoTrans, Op::Trans,     m, n, k, alpha, dA, lda, B, k-1, beta, dC, ldc, queue ), blas::Error );
    assert_throw( blas::gemm( Layout::RowMajor, Op::NoTrans, Op::ConjTrans, m, n, k, alpha, dA, lda, B, k-1, beta, dC, ldc, queue ), blas::Error );

    assert_throw( blas::gemm( Layout::ColMajor, transA, transB, m, n, k, alpha, dA, lda, dB, ldb, beta, dC, m-1, queue ), blas::Error );
    assert_throw( blas::gemm( Layout::RowMajor, transA, transB, m, n, k, alpha, dA, lda, dB, ldb, beta, dC, n-1, queue ), blas::Error );

    if (verbose >= 1) {
        printf( "\n"
                "A Am=%5lld, An=%5lld, lda=%5lld, size=%10lld, norm %.2e\n"
                "B Bm=%5lld, Bn=%5lld, ldb=%5lld, size=%10lld, norm %.2e\n"
                "C Cm=%5lld, Cn=%5lld, ldc=%5lld, size=%10lld, norm %.2e\n",
                (lld) Am, (lld) An, (lld) lda, (lld) size_A, Anorm,
                (lld) Bm, (lld) Bn, (lld) ldb, (lld) size_B, Bnorm,
                (lld) Cm, (lld) Cn, (lld) ldc, (lld) size_C, Cnorm );
    }
    if (verbose >= 2) {
        printf( "alpha = %.4e + %.4ei; beta = %.4e + %.4ei;\n",
                real(alpha), imag(alpha),
                real(beta),  imag(beta) );
        printf( "A = "    ); print_matrix( Am, An, A, lda );
        printf( "B = "    ); print_matrix( Bm, Bn, B, ldb );
        printf( "C = "    ); print_matrix( Cm, Cn, C, ldc );
    }

    // run test
    libtest::flush_cache( params.cache.value() );
    double time = get_wtime();
    blas::gemm( layout, transA, transB, m, n, k,
                alpha, dA, lda, dB, ldb, beta, dC, ldc, queue );
    queue.sync();
    time = get_wtime() - time;

    double gflop = Gflop < scalar_t >::gemm( m, n, k );
    params.time.value()   = time;
    params.gflops.value() = gflop / time;
    blas::device_getmatrix(Cm, Cn, dC, ldc, C, ldc, queue);
    queue.sync();

    if (verbose >= 2) {
        printf( "C2 = " ); print_matrix( Cm, Cn, C, ldc );
    }

    if (params.ref.value() == 'y' || params.check.value() == 'y') {
        // run reference
        libtest::flush_cache( params.cache.value() );
        time = get_wtime();
        cblas_gemm( cblas_layout_const(layout),
                    cblas_trans_const(transA),
                    cblas_trans_const(transB),
                    m, n, k, alpha, A, lda, B, ldb, beta, Cref, ldc );
        time = get_wtime() - time;

        params.ref_time.value()   = time;
        params.ref_gflops.value() = gflop / time;

        if (verbose >= 2) {
            printf( "Cref = " ); print_matrix( Cm, Cn, Cref, ldc );
        }

        // check error compared to reference
        real_t error;
        bool okay;
        check_gemm( Cm, Cn, k, alpha, beta, Anorm, Bnorm, Cnorm,
                    Cref, ldc, C, ldc, verbose, &error, &okay );
        params.error.value() = error;
        params.okay.value() = okay;
    }

    delete[] A;
    delete[] B;
    delete[] C;
    delete[] Cref;
    
    blas::device_free( dA );
    blas::device_free( dB );
    blas::device_free( dC );
}

// -----------------------------------------------------------------------------
void test_gemm_device( Params& params, bool run )
{
    switch (params.datatype.value()) {
        case libtest::DataType::Integer:
            //test_gemm_device_work< int64_t >( params, run );
            throw std::exception();
            break;

        case libtest::DataType::Single:
            test_gemm_device_work< float, float, float >( params, run );
            break;

        case libtest::DataType::Double:
            test_gemm_device_work< double, double, double >( params, run );
            break;

        case libtest::DataType::SingleComplex:
            test_gemm_device_work< std::complex<float>, std::complex<float>,
                            std::complex<float> >( params, run );
            break;

        case libtest::DataType::DoubleComplex:
            test_gemm_device_work< std::complex<double>, std::complex<double>,
                            std::complex<double> >( params, run );
            break;
    }
}
