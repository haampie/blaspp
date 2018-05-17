#ifndef DEVICE_BATCH_SYRK_HH
#define DEVICE_BATCH_SYRK_HH

#include "device.hh"
#include <limits>
#include <cstring>

namespace blas {

namespace batch{
// -----------------------------------------------------------------------------
/// @ingroup syrk
inline
void syrk(
    std::vector<blas::Uplo> const &uplo,
    std::vector<blas::Op>   const &trans,
    std::vector<int64_t>    const &n, 
    std::vector<int64_t>    const &k, 
    std::vector<float >     const &alpha,
    std::vector<float*>     const &Aarray, std::vector<int64_t> const &ldda,
    std::vector<float >     const &beta,
    std::vector<float*>     const &Carray, std::vector<int64_t> const &lddc,
    const size_t batch,                    std::vector<int64_t>       &info, 
    blas::Queue &queue )
{
    blas_error_if( batch < 0 );
    blas_error_if( !(info.size() == 0 || info.size() == 1 || info.size() == batch) );
    if(info.size() > 0){
        // perform error checking
        blas::batch::syrk_check<float>( 
                        uplo, trans, 
                        n, k, 
                        alpha, Aarray, ldda, 
                        beta,  Carray, lddc, 
                        batch, info );
    }

    throw std::exception();  // not yet available
}

// -----------------------------------------------------------------------------
/// @ingroup syrk
inline
void syrk(
    std::vector<blas::Uplo>  const &uplo,
    std::vector<blas::Op>    const &trans,
    std::vector<int64_t>     const &n, 
    std::vector<int64_t>     const &k, 
    std::vector<double >     const &alpha,
    std::vector<double*>     const &Aarray, std::vector<int64_t> const &ldda,
    std::vector<double >     const &beta,
    std::vector<double*>     const &Carray, std::vector<int64_t> const &lddc,
    const size_t batch,                     std::vector<int64_t>       &info, 
    blas::Queue &queue )
{
    blas_error_if( batch < 0 );
    blas_error_if( !(info.size() == 0 || info.size() == 1 || info.size() == batch) );
    if(info.size() > 0){
        // perform error checking
        blas::batch::syrk_check<double>( 
                        uplo, trans, 
                        n, k, 
                        alpha, Aarray, ldda, 
                        beta,  Carray, lddc, 
                        batch, info );
    }

    throw std::exception();  // not yet available
}

// -----------------------------------------------------------------------------
/// @ingroup syrk
inline
void syrk(
    std::vector<blas::Uplo>  const &uplo,
    std::vector<blas::Op>    const &trans,
    std::vector<int64_t>     const &n, 
    std::vector<int64_t>     const &k, 
    std::vector<std::complex<float> > const &alpha,
    std::vector<std::complex<float>*> const &Aarray, std::vector<int64_t> const &ldda,
    std::vector<std::complex<float> > const &beta,
    std::vector<std::complex<float>*> const &Carray, std::vector<int64_t> const &lddc,
    const size_t batch, std::vector<int64_t>       &info, 
    blas::Queue &queue )
{
    blas_error_if( batch < 0 );
    blas_error_if( !(info.size() == 0 || info.size() == 1 || info.size() == batch) );
    if(info.size() > 0){
        // perform error checking
        blas::batch::syrk_check<std::complex<float>>( 
                        uplo, trans, 
                        n, k, 
                        alpha, Aarray, ldda, 
                        beta,  Carray, lddc, 
                        batch, info );
    }

    throw std::exception();  // not yet available
}

// -----------------------------------------------------------------------------
/// @ingroup syrk
inline
void syrk(
    std::vector<blas::Uplo>  const &uplo,
    std::vector<blas::Op>    const &trans,
    std::vector<int64_t>     const &n, 
    std::vector<int64_t>     const &k, 
    std::vector<std::complex<double> > const &alpha,
    std::vector<std::complex<double>*> const &Aarray, std::vector<int64_t> const &ldda,
    std::vector<std::complex<double> > const &beta,
    std::vector<std::complex<double>*> const &Carray, std::vector<int64_t> const &lddc,
    const size_t batch, std::vector<int64_t>       &info, 
    blas::Queue &queue )
{
    blas_error_if( batch < 0 );
    blas_error_if( !(info.size() == 0 || info.size() == 1 || info.size() == batch) );
    if(info.size() > 0){
        // perform error checking
        blas::batch::syrk_check<std::complex<double>>( 
                        uplo, trans, 
                        n, k, 
                        alpha, Aarray, ldda, 
                        beta,  Carray, lddc, 
                        batch, info );
    }

    throw std::exception();  // not yet available
}

}        //  namespace batch
}        //  namespace blas

#endif        //  #ifndef DEVICE_BATCH_SYRK_HH

