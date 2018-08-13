#include "test.hh"
#include "cblas.hh"
#include "lapack_tmp.hh"
#include "blas_flops.hh"
#include "print_matrix.hh"
#include "check_gemm.hh"

// -----------------------------------------------------------------------------
template< typename TA, typename TB >
void test_trsm_work( Params& params, bool run )
{
    using namespace libtest;
    using namespace blas;
    typedef scalar_type<TA, TB> scalar_t;
    typedef real_type<scalar_t> real_t;
    typedef long long lld;

    // get & mark input values
    blas::Layout layout = params.layout.value();
    blas::Side side = params.side.value();
    blas::Uplo uplo = params.uplo.value();
    blas::Op trans  = params.trans.value();
    blas::Diag diag = params.diag.value();
    scalar_t alpha  = params.alpha.value();
    int64_t m       = params.dim.m();
    int64_t n       = params.dim.n();
    int64_t align   = params.align.value();
    int64_t verbose = params.verbose.value();

    // mark non-standard output values
    params.gflops.value();
    params.ref_time.value();
    params.ref_gflops.value();

    if (! run)
        return;

    // ----------
    // setup
    int64_t Am = (side == Side::Left ? m : n);
    int64_t Bm = m;
    int64_t Bn = n;
    if (layout == Layout::RowMajor)
        std::swap( Bm, Bn );
    int64_t lda = roundup( Am, align );
    int64_t ldb = roundup( Bm, align );
    size_t size_A = size_t(lda)*Am;
    size_t size_B = size_t(ldb)*Bn;
    TA* A    = new TA[ size_A ];
    TB* B    = new TB[ size_B ];
    TB* Bref = new TB[ size_B ];

    int64_t idist = 1;
    int iseed[4] = { 0, 0, 0, 1 };
    lapack_larnv( idist, iseed, size_A, A );  // TODO: generate
    lapack_larnv( idist, iseed, size_B, B );  // TODO
    lapack_lacpy( "g", Bm, Bn, B, ldb, Bref, ldb );

    // set unused data to nan
    if (uplo == Uplo::Lower) {
        for (int64_t j = 0; j < Am; ++j)
            for (int64_t i = 0; i < j; ++i)  // upper
                A[ i + j*lda ] = nan("");
    }
    else {
        for (int64_t j = 0; j < Am; ++j)
            for (int64_t i = j+1; i < Am; ++i)  // lower
                A[ i + j*lda ] = nan("");
    }

    // Factor A into L L^H or U U^H to get a well-conditioned triangular matrix.
    // If diag == Unit, the diagonal is replaced; this is still well-conditioned.
    // First, brute force positive definiteness.
    for (int64_t i = 0; i < Am; ++i) {
        A[ i + i*lda ] += Am;
    }
    blas_int info = 0;
    lapack_potrf( uplo2str(uplo), Am, A, lda, &info );
    assert( info == 0 );

    // norms for error check
    real_t work[1];
    real_t Anorm = lapack_lantr( "f", uplo2str(uplo), diag2str(diag),
                                 Am, Am, A, lda, work );
    real_t Bnorm = lapack_lange( "f", Bm, Bn, B, ldb, work );

    // if row-major, transpose A
    if (layout == Layout::RowMajor) {
        for (int64_t j = 0; j < Am; ++j) {
            for (int64_t i = 0; i < j; ++i) {
                std::swap( A[ i + j*lda ], A[ j + i*lda ] );
            }
        }
    }

    // test error exits
    assert_throw( blas::trsm( Layout(0), side,    uplo,    trans, diag,     m,  n, alpha, A, lda, B, ldb ), blas::Error );
    assert_throw( blas::trsm( layout,    Side(0), uplo,    trans, diag,     m,  n, alpha, A, lda, B, ldb ), blas::Error );
    assert_throw( blas::trsm( layout,    side,    Uplo(0), trans, diag,     m,  n, alpha, A, lda, B, ldb ), blas::Error );
    assert_throw( blas::trsm( layout,    side,    uplo,    Op(0), diag,     m,  n, alpha, A, lda, B, ldb ), blas::Error );
    assert_throw( blas::trsm( layout,    side,    uplo,    trans, Diag(0),  m,  n, alpha, A, lda, B, ldb ), blas::Error );
    assert_throw( blas::trsm( layout,    side,    uplo,    trans, diag,    -1,  n, alpha, A, lda, B, ldb ), blas::Error );
    assert_throw( blas::trsm( layout,    side,    uplo,    trans, diag,     m, -1, alpha, A, lda, B, ldb ), blas::Error );

    assert_throw( blas::trsm( layout, Side::Left,  uplo,   trans, diag,     m,  n, alpha, A, m-1, B, ldb ), blas::Error );
    assert_throw( blas::trsm( layout, Side::Right, uplo,   trans, diag,     m,  n, alpha, A, n-1, B, ldb ), blas::Error );

    assert_throw( blas::trsm( Layout::ColMajor, side, uplo, trans, diag,    m,  n, alpha, A, lda, B, m-1 ), blas::Error );
    assert_throw( blas::trsm( Layout::RowMajor, side, uplo, trans, diag,    m,  n, alpha, A, lda, B, n-1 ), blas::Error );

    if (verbose >= 1) {
        printf( "\n"
                "A Am=%5lld, Am=%5lld, lda=%5lld, size=%10lld, norm=%.2e\n"
                "B Bm=%5lld, Bn=%5lld, ldb=%5lld, size=%10lld, norm=%.2e\n",
                (lld) Am, (lld) Am, (lld) lda, (lld) size_A, Anorm,
                (lld) Bm, (lld) Bn, (lld) ldb, (lld) size_B, Bnorm );
    }
    if (verbose >= 2) {
        printf( "A = " ); print_matrix( Am, Am, A, lda );
        printf( "B = " ); print_matrix( Bm, Bn, B, ldb );
    }

    // run test
    libtest::flush_cache( params.cache.value() );
    double time = get_wtime();
    blas::trsm( layout, side, uplo, trans, diag, m, n, alpha, A, lda, B, ldb );
    time = get_wtime() - time;

    double gflop = Gflop < scalar_t >::trsm( side, m, n );
    params.time.value()   = time;
    params.gflops.value() = gflop / time;

    if (verbose >= 2) {
        printf( "X = " ); print_matrix( Bm, Bn, B, ldb );
    }

    if (params.check.value() == 'y') {
        // run reference
        libtest::flush_cache( params.cache.value() );
        time = get_wtime();
        cblas_trsm( cblas_layout_const(layout),
                    cblas_side_const(side),
                    cblas_uplo_const(uplo),
                    cblas_trans_const(trans),
                    cblas_diag_const(diag),
                    m, n, alpha, A, lda, Bref, ldb );
        time = get_wtime() - time;

        params.ref_time.value()   = time;
        params.ref_gflops.value() = gflop / time;

        if (verbose >= 2) {
            printf( "Xref = " ); print_matrix( Bm, Bn, Bref, ldb );
        }

        // check error compared to reference
        // Am is reduction dimension
        // beta = 0, Cnorm = 0 (initial).
        real_t error;
        bool okay;
        check_gemm( Bm, Bn, Am, alpha, scalar_t(0), Anorm, Bnorm, real_t(0),
                    Bref, ldb, B, ldb, verbose, &error, &okay );
        params.error.value() = error;
        params.okay.value() = okay;
    }

    delete[] A;
    delete[] B;
    delete[] Bref;
}

// -----------------------------------------------------------------------------
void test_trsm( Params& params, bool run )
{
    switch (params.datatype.value()) {
        case libtest::DataType::Integer:
            //test_trsm_work< int64_t >( params, run );
            throw std::exception();
            break;

        case libtest::DataType::Single:
            test_trsm_work< float, float >( params, run );
            break;

        case libtest::DataType::Double:
            test_trsm_work< double, double >( params, run );
            break;

        case libtest::DataType::SingleComplex:
            test_trsm_work< std::complex<float>, std::complex<float> >
                ( params, run );
            break;

        case libtest::DataType::DoubleComplex:
            test_trsm_work< std::complex<double>, std::complex<double> >
                ( params, run );
            break;
    }
}
