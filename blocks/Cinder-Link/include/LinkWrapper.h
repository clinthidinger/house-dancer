#pragma once

#include <functional>
#include <memory>
struct LinkState;

class LinkWrapper
{
public:
    LinkWrapper();
    ~LinkWrapper() = default;
    LinkWrapper( LinkWrapper &other ) = delete;
    LinkWrapper &operator=( LinkWrapper &rhs ) = delete;
    LinkWrapper( LinkWrapper &&other ) = delete;
    LinkWrapper &operator=( LinkWrapper &&rhs ) = delete;

    bool getIsEnabled() const;
    void setIsEnabled( bool state );
    double getBeat() const;
    double getPhase() const;
    double getBeatAndPhase( double &phase ) const;
    size_t getNumPeers() const;
    double getTempo() const;
    void stop();
    void start();

private:
    using LinkStatePtr = std::unique_ptr<LinkState, std::function<void( LinkState * )>>;
    LinkStatePtr mLinkState;
};