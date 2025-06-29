#include "SavitzkyGolayFilter.h"
#include <cassert>

SavitzkyGolayFilter::SavitzkyGolayFilter( unsigned m, int t, unsigned n, unsigned s, float dt )
    : mOptions( m, t, n, s, dt )
{
    init();
}

SavitzkyGolayFilter::SavitzkyGolayFilter( const Options &options )
    : mOptions( options )
{
    init();
}

SavitzkyGolayFilter::SavitzkyGolayFilter()
{
    init();
}

float SavitzkyGolayFilter::gramPoly( const int i, const int m, const int k, const int s )
{
    if( k > 0 )
    {
        return ( 4.0f * k - 2.0f ) / ( k * ( 2.0f * m - k + 1.0f ) ) * ( i * gramPoly( i, m, k - 1, s ) + s * gramPoly( i, m, k - 1, s - 1 ) )
            - ( ( k - 1.0f ) * ( 2.0f * m + k ) ) / ( k * ( 2.0f * m - k + 1.0f ) ) * gramPoly( i, m, k - 2, s );
    }
    if( k == 0 && s == 0 )
    {
        return 1.0f;
    }

    return 0.0f;
}

float SavitzkyGolayFilter::genFact( const int a, const int b )
{
    float gf = 1.0f;

    for( int j = ( a - b ) + 1; j <= a; j++ )
    {
        gf *= j;
    }

    return gf;
}

float SavitzkyGolayFilter::weight( const int i, const int t, const int m, const int n, const int s )
{
    float w = 0.0f;
    for( int k = 0; k <= n; ++k )
    {
        w = w
            + ( 2 * k + 1 ) * ( genFact( 2 * m, k ) / genFact( 2 * m + k + 1, k + 1 ) ) * gramPoly( i, m, k, 0 )
            * gramPoly( t, m, k, s );
    }

    return w;
}

std::vector<float> SavitzkyGolayFilter::computeWeights( const int m, const int t, const int n, const int s )
{
    std::vector<float> weights( 2 * static_cast<size_t>( m ) + 1 );
    for( int i = 0; i < 2 * m + 1; ++i )
    {
        weights[static_cast<size_t>( i )] = weight( i - m, t, m, n, s );
    }

    return weights;
}

void SavitzkyGolayFilter::init()
{
    // Compute weights for the time window 2*m+1, for the t'th least-square
    // point of the s'th derivative
    mWeights = computeWeights( static_cast<int>( mOptions.m ),
                               mOptions.t,
                               static_cast<int>( mOptions.n ),
                               static_cast<int>( mOptions.s ) );
    mDt = static_cast<float>( std::pow( mOptions.timeStep(), mOptions.derivationOrder() ) );
}

ci::vec3 SavitzkyGolayFilter::filter( const std::vector<ci::vec3> &v, int offset ) const
{
    assert( v.size() >= mWeights.size() );// &&v.size() > 0 );
    ci::vec3 res( mWeights[0] * v[v.size() - mWeights.size() - offset].x,
                  mWeights[0] * v[v.size() - mWeights.size() - offset].y,
                  mWeights[0] * v[v.size() - mWeights.size() - offset].z );
    for( size_t i = 1; i < mWeights.size(); ++i )
    {
        res.x += mWeights[i] * v[v.size() - ( mWeights.size() - i ) - offset].x;
        res.y += mWeights[i] * v[v.size() - ( mWeights.size() - i ) - offset].y;
        res.z += mWeights[i] * v[v.size() - ( mWeights.size() - i ) - offset].z;
    }
    res.x /= mDt;
    res.y /= mDt;
    res.z /= mDt;

    return res;
}

float SavitzkyGolayFilter::filter( const std::vector<float> &v, int offset ) const
{
    assert( v.size() >= mWeights.size() );// &&v.size() > 0 );
    float res = ( mWeights[0] * v[v.size() - mWeights.size() - offset] );
    for( size_t i = 1; i < mWeights.size(); ++i )
    {
        res += mWeights[i] * v[v.size() - ( mWeights.size() - i ) - offset];
    }
    res /= mDt;

    return res;
}

std::ostream &operator<<( std::ostream &os, const SavitzkyGolayFilter::Options &options )
{
    os << "m                       : " << options.m << std::endl
        << "Window Size (2*m+1)     : " << 2 * options.m + 1 << std::endl
        << "n (Order)               :" << options.n << std::endl
        << "s (Differentiate)       : " << options.s << std::endl
        << "t: Filter point ([-m,m]): " << options.t << std::endl;
    return os;
}
