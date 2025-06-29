#pragma once

#include <ostream>
#include <vector>
#include <cinder/Vector.h>
//#include "FakeCinder.h"

// See: https://github.com/arntanguy/gram_savitzky_golay/
// Also: http://phys.uri.edu/nigh/NumRec/bookfpdf/f14-8.pdf
class SavitzkyGolayFilter
{
public:
    struct Options
    {
        //! Window size is 2*m+1
        unsigned int m{ 5 };
        //! Time at which the filter is applied
        // For real-time, should be t=m
        int t{ 5 };
        //! Polynomial order
        unsigned n{ 3 };
        //! Derivation order (0 for no derivation)
        unsigned s{ 0 };
        //! Time step
        double dt{ 1.0f };

        Options() = default;

        ///
        //  @brief Construct a filter with the specified configuration
        //
        // @param m Window size is `2*m+1`
        // @param t Time at which the filter is applied
        // - `t=m` for real-time filtering.
        //   This uses only past information to determine the filter value and thus
        //   does not introduce delay. However, this comes at the cost of filtering
        //   accuracy as no future information is available.
        // - `t=0` for smoothing. Uses both past and future information to determine the optimal
        //   filtered value
        // @param n Polynamial order
        // @param s Derivation order
        // - `0`: No derivation
        // - `1`: First order derivative
        // - `2`: Second order derivative
        // @param dt Time step
        //
        Options( unsigned m_, int t_, unsigned n_, unsigned s_, float dt_ = 1.0f ) 
            : m( m_ ), t( t_ ), n( n_ ), s( s_ ), dt( dt_ )
        {
        }

        // @brief Time at which the filter is evaluated
        inline int dataPoint() const { return t; }
        // @brief Derivation order
        inline unsigned int derivationOrder() const { return s; }
        // @brief Polynomial order
        inline unsigned int order() const { return n; }
        // @brief Full size of the filter's window `2*m+1`
        inline unsigned window_size() const { return 2 * m + 1; }
        // @brief Time step
        double timeStep() const { return dt; }

        friend std::ostream &operator<<( std::ostream &os, const Options &conf );
    };

    SavitzkyGolayFilter( unsigned m, int t, unsigned n, unsigned s, float dt = 1.0f );
    SavitzkyGolayFilter( const Options &options );
    SavitzkyGolayFilter();
    
    void configure( const Options &conf );

    ///
    // @brief Apply Savitzky-Golay convolution to the data x should have size 2*m+1
    // As the function only applies a convolution, it runs in O(2m+1), and should
    // be rather fast.
    //
    // @param v Container with the data to be filtered.
    // Should have 2*m+1 elements
    // Type of elements needs to be compatible with multiplication by a scalar,
    // and addition with itself
    // Common types would be std::vector<float>, std::vector<Eigen::VectorXd>, boost::circular_buffer<Eigen::Vector3d>...
    //
    // @return Filtered value according to the precomputed filter weights.
    //
    /*template<typename ContainerT>
    typename ContainerT::value_type filter( const ContainerT &v ) const
    {
        assert( v.size() == weights_.size() && v.size() > 0 );
        using T = typename ContainerT::value_type;
        T res = weights_[0] * v[0];
        for( size_t i = 1; i < v.size(); ++i )
        {
            res += weights_[i] * v[i];
        }
        return res / dt_;
    }*/

    // Todo: move to child Filter3d class.
    ci::vec3 filter( const std::vector<ci::vec3> &v, int offset = 0 ) const;
    float filter( const std::vector<float> &v, int offset = 0 ) const;

    const std::vector<float> &getWeights() const;
    void setWeights( const std::vector<float> &weights );

    const Options &getOptions() const;

private:
    void init();
    static float gramPoly( const int i, const int m, const int k, const int s );
    static float genFact( const int a, const int b );
    static float weight( const int i, const int t, const int m, const int n, const int s );
    static std::vector<float> computeWeights( const int m, const int t, const int n, const int s );
    Options mOptions;
    std::vector<float> mWeights;
    float mDt{ 0.0f };

};

inline void SavitzkyGolayFilter::configure( const Options &options ) { mOptions = options; init(); }
inline const std::vector<float> &SavitzkyGolayFilter::getWeights() const { return mWeights; }
inline void SavitzkyGolayFilter::setWeights( const std::vector<float> &weights ) { mWeights = weights; }
inline const SavitzkyGolayFilter::Options &SavitzkyGolayFilter::getOptions() const { return mOptions; }
