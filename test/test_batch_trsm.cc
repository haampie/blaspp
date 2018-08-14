#include "test.hh"
#include "cblas.hh"
#include "lapack_tmp.hh"
#include "blas_flops.hh"
#include "print_matrix.hh"
#include "check_gemm.hh"

// -----------------------------------------------------------------------------
template< typename TA, typename TB >
void test_batch_trsm_work( Params& params, bool run )
{
    using namespace libtest;
    using namespace blas;
    typedef scalar_type<TA, TB> scalar_t;
    typedef real_type<scalar_t> real_t;
    typedef long long lld;

    // get & mark input values
    blas::Side side_    = params.side.value();
    blas::Uplo uplo_    = params.uplo.value();
    blas::Op trans_    = params.trans.value();
    blas::Diag diag_    = params.diag.value();
    scalar_t alpha_     = params.alpha.value();
    int64_t m_          = params.dim.m();
    int64_t n_          = params.dim.n();
    size_t  batch       = params.batch.value();
    int64_t align       = params.align.value();
    int64_t verbose     = params.verbose.value();

    // mark non-standard output values
    params.ref_time.value();
    params.ref_gflops.value();

    if (! run)
        return;

    // ----------
    // setup
    int64_t Am = (side_ == Side::Left ? m_ : n_);
    int64_t Bm = m_;
    int64_t Bn = n_;
    int64_t lda_ = roundup( Am, align );
    int64_t ldb_ = roundup( Bm, align );
    size_t size_A = size_t(lda_)*Am;
    size_t size_B = size_t(ldb_)*Bn;
    TA* A    = new TA[ batch * size_A ];
    TB* B    = new TB[ batch * size_B ];
    TB* Bref = new TB[ batch * size_B ];

    // pointer arrays
    std::vector<TA*>    Aarray( batch );
    std::vector<TB*>    Barray( batch );
    std::vector<TB*> Brefarray( batch );

    for(size_t i = 0; i < batch; i++){
         Aarray[i]   =  A   + i * size_A;
         Barray[i]   =  B   + i * size_B;
        Brefarray[i] = Bref + i * size_B;
    }

    // info
    std::vector<int64_t> info( batch );

    // wrap scalar arguments in std::vector
    std::vector<blas::Side> side(1, side_);
    std::vector<blas::Uplo> uplo(1, uplo_);
    std::vector<blas::Op>   trans(1, trans_);
    std::vector<blas::Diag> diag(1, diag_);
    std::vector<int64_t> m(1, m_);
    std::vector<int64_t> n(1, n_);
    std::vector<int64_t> vlda_(1, lda_);
    std::vector<int64_t> vldb_(1, ldb_);
    std::vector<scalar_t> alpha(1, alpha_);

    int64_t idist = 1;
    int iseed[4] = { 0, 0, 0, 1 };
    lapack_larnv( idist, iseed, batch * size_A, A );  // TODO: generate
    lapack_larnv( idist, iseed, batch * size_B, B );  // TODO
    lapack_lacpy( "g", Bm, batch * Bn, B, ldb_, Bref, ldb_ );

    // set unused data to nan
    if (uplo_ == Uplo::Lower) {
        for(size_t s = 0; s < batch; s++)
            for (int j = 0; j < Am; ++j)
                for (int i = 0; i < j; ++i)  // upper
                    Aarray[s][ i + j*lda_ ] = nan("");
    }
    else {
        for(size_t s = 0; s < batch; s++)
            for (int j = 0; j < Am; ++j)
                for (int i = j+1; i < Am; ++i)  // lower
                    Aarray[s][ i + j*lda_ ] = nan("");
    }

    // Factor A into L L^H or U U^H to get a well-conditioned triangular matrix.
    // If diag_ == Unit, the diagonal is replaced; this is still well-conditioned.
    // First, brute force positive definiteness.
    for(size_t s = 0; s < batch; s++){
        for (int i = 0; i < Am; ++i) {
            Aarray[s][ i + i*lda_ ] += Am;
        }
        blas_int blas_info = 0;
        lapack_potrf( uplo2str(uplo_), Am, Aarray[s], lda_, &blas_info );
        assert( blas_info == 0 );
    }

    // norms for error check
    real_t work[1];
    real_t* Anorm = new real_t[ batch ];
    real_t* Bnorm = new real_t[ batch ];

    for(size_t s = 0; s < batch; s++){
        Anorm[s] = lapack_lantr( "f", uplo2str(uplo_), diag2str(diag_), Am, Am, Aarray[s], lda_, work );
        Bnorm[s] = lapack_lange( "f", Bm, Bn, Barray[s], ldb_, work );
    }

    // decide error checking mode
    info.resize( 0 );

    // run test
    libtest::flush_cache( params.cache.value() );
    double time = get_wtime();
    blas::batch::trsm( side, uplo, trans, diag, m, n, alpha, Aarray, vlda_, Barray, vldb_, 
                       batch, info );
    time = get_wtime() - time;

    double gflop = batch * Gflop < scalar_t >::trsm( side_, m_, n_ );
    params.time.value()   = time;
    params.gflops.value() = gflop / time;

    if (params.check.value() == 'y') {
        // run reference
        libtest::flush_cache( params.cache.value() );
        time = get_wtime();
        for(size_t s = 0; s < batch; s++){
            cblas_trsm( CblasColMajor,
                        cblas_side_const(side_),
                        cblas_uplo_const(uplo_),
                        cblas_trans_const(trans_),
                        cblas_diag_const(diag_),
                        m_, n_, alpha_, Aarray[s], lda_, Brefarray[s], ldb_ );
        }
        time = get_wtime() - time;

        params.ref_time.value()   = time;
        params.ref_gflops.value() = gflop / time;

        // check error compared to reference
        // Am is reduction dimension
        // beta = 0, Cnorm = 0 (initial).
        real_t err, error = 0.0;
        bool ok, okay = true;
        for(size_t s = 0; s < batch; s++){
            check_gemm( Bm, Bn, Am, alpha_, scalar_t(0), Anorm[s], Bnorm[s], real_t(0),
                        Brefarray[s], ldb_, Barray[s], ldb_, verbose, &err, &ok );
            error = max(error, err);
            okay &= ok;
        }
        params.error.value() = error;
        params.okay.value() = okay;
    }

    delete[] A;
    delete[] B;
    delete[] Bref;
    delete[] Anorm;
    delete[] Bnorm;
}

// -----------------------------------------------------------------------------
void test_batch_trsm( Params& params, bool run )
{
    switch (params.datatype.value()) {
        case libtest::DataType::Integer:
            //test_batch_trsm_work< int64_t >( params, run );
            throw std::exception();
            break;

        case libtest::DataType::Single:
            test_batch_trsm_work< float, float >( params, run );
            break;

        case libtest::DataType::Double:
            test_batch_trsm_work< double, double >( params, run );
            break;

        case libtest::DataType::SingleComplex:
            test_batch_trsm_work< std::complex<float>, std::complex<float> >
                ( params, run );
            break;

        case libtest::DataType::DoubleComplex:
            test_batch_trsm_work< std::complex<double>, std::complex<double> >
                ( params, run );
            break;
    }
}
