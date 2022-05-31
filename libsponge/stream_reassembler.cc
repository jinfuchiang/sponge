#include "stream_reassembler.hh"

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) :
    output_(capacity), capacity_(capacity), 
    unassembled_(capacity, 0), ranges_({{0, 0}}) {}

pair<size_t, size_t> StreamReassembler::unassembled_range() const {
    return {output_.bytes_written(), output_.bytes_read() + capacity_};
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    size_t sz = data.size();
    size_t start_i = 0, end_i = sz; // move data[start_i:end_i-1] to unassembled_
    size_t start_index = index, end_index = index + sz; // 
    
    if(eof) {
        last_index_ = end_index;
    }

    auto [start_index_, end_index_] = unassembled_range();

    if(start_index < start_index_) {
        start_i += start_index_ - start_index;
        start_index = start_index_;
    }

    if(end_index > end_index_) {
        end_i -= end_index - end_index_;
        end_index = end_index_;
    }


    assert(end_index - start_index == end_i - start_i);

    sz = end_index - start_index;
    // move data[start_i:end_i] to unassembled[i:]
    for(int i = (start_i_ + start_index - start_index_) % unassembled_.size(); 
        start_i < end_i;
        (++i) %= unassembled_.size(), ++start_i) {
        unassembled_[i]= data[start_i];
    }
    
    auto [left, right] = add_range({start_index, end_index});
    if(left <= start_index && right > start_index_) {
        size_t stream_sz = right - start_index_;

        string stream = unassembled_.substr(start_i_, stream_sz);        
        if(ssize_t left_len = stream_sz - stream.size(); left_len > 0) {
            stream += unassembled_.substr(0, left_len);
        }
        start_i_ = (start_i_ + stream_sz) % unassembled_.size();
        
        output_.write(stream);
    }

    if(last_index_ != -1 && right == static_cast<size_t>(last_index_)) output_.end_input();
}

pair<size_t, size_t> StreamReassembler::add_range(const pair<size_t, size_t>& range) {
    auto [left, right] = range;
    auto range_pos = ranges_.lower_bound(range);
    assert(left <= range_pos->first);
    auto erase_pos_end = range_pos, erase_pos_begin = range_pos;
    
    // forward merge
    if(range_pos != ranges_.begin()) {
        auto pos = prev(range_pos);
        auto [pos_left, pos_right] = *pos;
        if(right <= pos_right) return *ranges_.begin(); // cover
        if(pos_right >= left) {
            // left < right, pos_left < pos_right
            // right > pos_right, pos_right >= left
            // left_pos < left <= pos_right < right
            // overlap
            left = pos_left;
            erase_pos_begin = pos;
        } 
    }
    
    // backward merge
    for(auto pos = range_pos; ; ++pos) {
        if(pos == ranges_.end()) {
            erase_pos_end = pos;
            break;
        }
        auto [pos_left, pos_right] = *pos;
        if(right < pos_left) {
            erase_pos_end = pos; break;
        } 
        // right >= pos_left, overlap exsits.
        right = max(pos_right, right);
    }
    while(erase_pos_begin != erase_pos_end) {
        erase_pos_begin = ranges_.erase(erase_pos_begin);
    }
    ranges_.insert({left, right});
    {   // for debug
        auto [prev_left, prev_right] = *ranges_.begin();
        for(auto it = ++ranges_.begin(); it != ranges_.end(); ++it) {
            auto [cur_left, cur_right] = *it;
            if(!(prev_left < cur_left && prev_right < cur_right)) assert(false);
        }
    }
    return *ranges_.begin();
}

size_t StreamReassembler::unassembled_bytes() const { 
    auto [start_index_, end_index_] = unassembled_range();
    size_t cnt = 0;
    for(auto it = ranges_.lower_bound({start_index_, 0}); it != ranges_.end(); ++it) {
        auto [left, right] = *it;
        if(end_index_ <= left) break;
        if(left <= start_index_ && start_index_ <= right && right <= end_index_) cnt += right - start_index_;
        else if(start_index_ <= left && right <= end_index_){
            cnt += right - left;
        }
        else if(left <= end_index_ && end_index_ <= right) cnt += end_index_ - left;
    }
    return cnt;
}

bool StreamReassembler::empty() const { 
    auto [start_index_, end_index_] = unassembled_range();
    return start_index_ + capacity_ == end_index_;
}
