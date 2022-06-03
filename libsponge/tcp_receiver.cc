#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

// Set the Initial Sequence Number if necessary. The sequence number of the first arriving
// segment that has the SYN flag set is the initial sequence number. You’ll want
// to keep track of that in order to keep converting between 32-bit wrapped seqnos/acknos
// and their absolute equivalents. (Note that the SYN flag is just one flag in the header.
// The same segment could also carry data and could even have the FIN flag set.)

// Push any data, or end-of-stream marker, to the StreamReassembler. If the
// FIN flag is set in a TCPSegment’s header, that means that the last byte of the payload
// is the last byte of the entire stream. Remember that the StreamReassembler expects
// stream indexes starting at zero; you will have to unwrap the seqnos to produce these.

void TCPReceiver::segment_received(const TCPSegment &seg) {
    auto seg_header = seg.header();
    if(seg_header.syn) {
        if(!isn.has_value()) 
            isn = seg_header.seqno;
        state = SYN_RECV;
    }
    
    if(seg_header.fin && state == SYN_RECV) {
        // stream_out().end_input();
        state = FIN_RECV;
    }
    if(state != LISTEN) {
        // wrapping_seqno->abolute_seqno
        uint64_t absolute_seqno = unwrap(seg_header.seqno, isn.value(), stream_out().bytes_written());
        if(absolute_seqno == 0 && !seg_header.syn) {
            // invalid segment
            return;
        }
        // aboslute_seqno->stream_index
        uint64_t stream_index = absolute_seqno ? absolute_seqno-1 : 0; // in case SYN segment with data
       
        reassembler_.push_substring(seg.payload().copy(), stream_index, seg_header.fin);
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if(state == LISTEN) return {};
    // stream_index->absolute_index
    uint64_t absolute_seqno = stream_out().bytes_written() + 1;
    // absolute_index->wrapping_index
    auto wrapping_seqno = wrap(absolute_seqno, isn.value());
    if(reassembler_.unassembled_bytes() == 0 && state == FIN_RECV) wrapping_seqno = wrapping_seqno + 1;
    return wrapping_seqno;
}

size_t TCPReceiver::window_size() const { 
    return reassembler_.window_size();
}
