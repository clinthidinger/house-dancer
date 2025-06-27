#include "LinkWrapper.h"
#include <AudioPlatform_Dummy.hpp>

struct LinkState
{
    std::atomic<bool> running { true };
    ableton::Link link{ 120.0 };
    ableton::linkaudio::AudioPlatform audioPlatform{ link };

    //LinkState()
    //    : running( true )
    //    , link( 120.0 )
    //    , audioPlatform( link )
    //{
    //}
};

LinkWrapper::LinkWrapper()
{
    //mLinkState = std::make_unique<LinkState>();
    mLinkState = LinkStatePtr( new LinkState(), [] ( LinkState *ls) { delete ls; } );
}

bool LinkWrapper::getIsEnabled() const
{
    return mLinkState->link.isEnabled();
}

void LinkWrapper::setIsEnabled( bool state )
{
    mLinkState->link.enable( state );
}

double LinkWrapper::getBeat() const
{
    const auto time = mLinkState->link.clock().micros();
    auto sessionState = mLinkState->link.captureAppSessionState();
    auto quantum = mLinkState->audioPlatform.mEngine.quantum();
    return sessionState.beatAtTime( time, quantum );
}

double LinkWrapper::getPhase() const
{
    const auto time = mLinkState->link.clock().micros();
    auto sessionState = mLinkState->link.captureAppSessionState();
    auto quantum = mLinkState->audioPlatform.mEngine.quantum();
    return sessionState.phaseAtTime( time, quantum );
}

double LinkWrapper::getBeatAndPhase( double &phase ) const
{
    const auto time = mLinkState->link.clock().micros();
    auto sessionState = mLinkState->link.captureAppSessionState();
    auto quantum = mLinkState->audioPlatform.mEngine.quantum();
    phase = sessionState.phaseAtTime( time, quantum );;
    return sessionState.beatAtTime( time, quantum );
}

size_t LinkWrapper::getNumPeers() const
{
    return mLinkState->link.numPeers();
}

double LinkWrapper::getTempo() const
{
    return mLinkState->link.captureAppSessionState().tempo();
}

void LinkWrapper::stop()
{
    if( mLinkState->audioPlatform.mEngine.isPlaying() )
    {
        mLinkState->audioPlatform.mEngine.stopPlaying();
    }
}

void LinkWrapper::start()
{
    //const auto tempo = state.link.captureAppSessionState().tempo();
    mLinkState->audioPlatform.mEngine.startPlaying();

}
