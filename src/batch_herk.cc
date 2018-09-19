#include <limits>
#include <cstring>
#include "batch_common.hh"
#include "blas.hh"

// -----------------------------------------------------------------------------
/// @ingroup herk
void blas::batch::herk(
    blas::Layout                layout, 
    std::vector<blas::Uplo> const &uplo,
    std::vector<blas::Op>   const &trans,
    std::vector<int64_t>    const &n, 
    std::vector<int64_t>    const &k, 
    std::vector<float >     const &alpha,
    std::vector<float*>     const &Aarray, std::vector<int64_t> const &ldda,
    std::vector<float >     const &beta,
    std::vector<float*>     const &Carray, std::vector<int64_t> const &lddc,
    const size_t batch,                    std::vector<int64_t>       &info )
{
    blas_error_if( batch < 0 );
    blas_error_if( !(info.size() == 0 || info.size() == 1 || info.size() == batch) );
    if(info.size() > 0){
        // perform error checking
        blas::batch::herk_check<float, float>( 
                        layout, uplo, trans, 
                        n, k, 
                        alpha, Aarray, ldda, 
                        beta,  Carray, lddc, 
                        batch, info );
    }

    #pragma omp parallel for schedule(dynamic)
    for(size_t i = 0; i < batch; i++){ 
        Uplo uplo_   = blas::batch::extract<Uplo>(uplo, i);
        Op   trans_  = blas::batch::extract<Op>(trans, i);
        int64_t n_   = blas::batch::extract<int64_t>(n, i);
        int64_t k_   = blas::batch::extract<int64_t>(k, i);
        int64_t lda_ = blas::batch::extract<int64_t>(ldda, i);
        int64_t ldc_ = blas::batch::extract<int64_t>(lddc, i);
        float alpha_ = blas::batch::extract<float>(alpha, i);
        float beta_  = blas::batch::extract<float>(beta, i);
        float* dA_   = blas::batch::extract<float*>(Aarray, i);
        float* dC_   = blas::batch::extract<float*>(Carray, i);
        blas::herk(
            layout, uplo_, trans_, n_, k_, 
            alpha_, dA_, lda_ , 
            beta_,  dC_, ldc_ );
    }
}

// -----------------------------------------------------------------------------
/// @ingroup herk
void blas::batch::herk(
    blas::Layout                layout, 
    std::vector<blas::Uplo>  const &uplo,
    std::vector<blas::Op>    const &trans,
    std::vector<int64_t>     const &n, 
    std::vector<int64_t>     const &k, 
    std::vector<double >     const &alpha,
    std::vector<double*>     const &Aarray, std::vector<int64_t> const &ldda,
    std::vector<double >     const &beta,
    std::vector<double*>     const &Carray, std::vector<int64_t> const &lddc,
    const size_t batch,                     std::vector<int64_t>       &info )
{
    blas_error_if( batch < 0 );
    blas_error_if( !(info.size() == 0 || info.size() == 1 || info.size() == batch) );
    if(info.size() > 0){
        // perform error checking
        blas::batch::herk_check<double, double>( 
                        layout, uplo, trans, 
                        n, k, 
                        alpha, Aarray, ldda, 
                        beta,  Carray, lddc, 
                        batch, info );
    }

    #pragma omp parallel for schedule(dynamic)
    for(size_t i = 0; i < batch; i++){ 
        Uplo uplo_   = blas::batch::extract<Uplo>(uplo, i);
        Op   trans_  = blas::batch::extract<Op>(trans, i);
        int64_t n_   = blas::batch::extract<int64_t>(n, i);
        int64_t k_   = blas::batch::extract<int64_t>(k, i);
        int64_t lda_ = blas::batch::extract<int64_t>(ldda, i);
        int64_t ldc_ = blas::batch::extract<int64_t>(lddc, i);
        double alpha_ = blas::batch::extract<double>(alpha, i);
        double beta_  = blas::batch::extract<double>(beta, i);
        double* dA_   = blas::batch::extract<double*>(Aarray, i);
        double* dC_   = blas::batch::extract<double*>(Carray, i);
        blas::herk(
            layout, uplo_, trans_, n_, k_, 
            alpha_, dA_, lda_ , 
            beta_,  dC_, ldc_ );
    }
}

// -----------------------------------------------------------------------------
/// @ingroup herk
void blas::batch::herk(
    blas::Layout                layout, 
    std::vector<blas::Uplo>  const &uplo,
    std::vector<blas::Op>    const &trans,
    std::vector<int64_t>     const &n, 
    std::vector<int64_t>     const &k, 
    std::vector<float>       const &alpha,
    std::vector<std::complex<float>*>     const &Aarray, std::vector<int64_t> const &ldda,
    std::vector<float >      const &beta,
    std::vector<std::complex<float>*>     const &Carray, std::vector<int64_t> const &lddc,
    const size_t batch,                                  std::vector<int64_t>       &info )
{
    blas_error_if( batch < 0 );
    blas_error_if( !(info.size() == 0 || info.size() == 1 || info.size() == batch) );
    if(info.size() > 0){
        // perform error checking
        blas::batch::herk_check<std::complex<float>, float>( 
                        layout, uplo, trans, 
                        n, k, 
                        alpha, Aarray, ldda, 
                        beta,  Carray, lddc, 
                        batch, info );
    }

    #pragma omp parallel for schedule(dynamic)
    for(size_t i = 0; i < batch; i++){ 
        Uplo uplo_   = blas::batch::extract<Uplo>(uplo, i);
        Op   trans_  = blas::batch::extract<Op>(trans, i);
        int64_t n_   = blas::batch::extract<int64_t>(n, i);
        int64_t k_   = blas::batch::extract<int64_t>(k, i);
        int64_t lda_ = blas::batch::extract<int64_t>(ldda, i);
        int64_t ldc_ = blas::batch::extract<int64_t>(lddc, i);
        float alpha_ = blas::batch::extract<float>(alpha, i);
        float beta_  = blas::batch::extract<float>(beta, i);
        std::complex<float>* dA_   = blas::batch::extract<std::complex<float>*>(Aarray, i);
        std::complex<float>* dC_   = blas::batch::extract<std::complex<float>*>(Carray, i);
        blas::herk(
            layout, uplo_, trans_, n_, k_, 
            alpha_, dA_, lda_ , 
            beta_,  dC_, ldc_ );
    }
}

// -----------------------------------------------------------------------------
/// @ingroup herk
void blas::batch::herk(
    blas::Layout                layout, 
    std::vector<blas::Uplo>  const &uplo,
    std::vector<blas::Op>    const &trans,
    std::vector<int64_t>     const &n, 
    std::vector<int64_t>     const &k, 
    std::vector<double>      const &alpha,
    std::vector<std::complex<double>*>     const &Aarray, std::vector<int64_t> const &ldda,
    std::vector<double >     const &beta,
    std::vector<std::complex<double>*>     const &Carray, std::vector<int64_t> const &lddc,
    const size_t batch,                                   std::vector<int64_t>       &info )
{
    blas_error_if( batch < 0 );
    blas_error_if( !(info.size() == 0 || info.size() == 1 || info.size() == batch) );
    if(info.size() > 0){
        // perform error checking
        blas::batch::herk_check<std::complex<double>, double>( 
                        layout, uplo, trans, 
                        n, k, 
                        alpha, Aarray, ldda, 
                        beta,  Carray, lddc, 
                        batch, info );
    }

    #pragma omp parallel for schedule(dynamic)
    for(size_t i = 0; i < batch; i++){ 
        Uplo uplo_   = blas::batch::extract<Uplo>(uplo, i);
        Op   trans_  = blas::batch::extract<Op>(trans, i);
        int64_t n_   = blas::batch::extract<int64_t>(n, i);
        int64_t k_   = blas::batch::extract<int64_t>(k, i);
        int64_t lda_ = blas::batch::extract<int64_t>(ldda, i);
        int64_t ldc_ = blas::batch::extract<int64_t>(lddc, i);
        double alpha_ = blas::batch::extract<double>(alpha, i);
        double beta_  = blas::batch::extract<double>(beta, i);
        std::complex<double>* dA_   = blas::batch::extract<std::complex<double>*>(Aarray, i);
        std::complex<double>* dC_   = blas::batch::extract<std::complex<double>*>(Carray, i);
        blas::herk(
            layout, uplo_, trans_, n_, k_, 
            alpha_, dA_, lda_ , 
            beta_,  dC_, ldc_ );
    }
}
