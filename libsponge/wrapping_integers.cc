#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.


using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint32_t v = isn.raw_value();
    v += n;
    return WrappingInt32(v);
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    auto get_diff = [](uint64_t nv, uint64_t cp) { return max(nv, cp) - min(nv, cp);};

    int64_t v = n.raw_value() - isn.raw_value();
    if(v < 0) v = -v;
    uint64_t candidates[] = {v | (((checkpoint >> 32)-1) << 32), v | (((checkpoint >> 32)) << 32), v | (((checkpoint >> 32)+1) << 32)};
    uint64_t diff = get_diff(candidates[0], checkpoint), ans = candidates[0];

    for(auto candidate : candidates) {
        if(auto ndiff = get_diff(candidate, checkpoint); diff > ndiff) {
            ans = candidate;
            diff = ndiff;
        }
    }
    return ans;
}
