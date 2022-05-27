#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

// template <typename... Targs>
// void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : 
    bytes_(capacity+1, 0), capacity_(capacity) {}

size_t ByteStream::write(const string &data) {
    size_t bytes_written = 0;
    for(char byte : data) {
        if((end_i_+1) % bytes_.size() == start_i_) break;
        bytes_[end_i_++] = byte;
        end_i_ %= bytes_.size();
        ++bytes_written;
    }
    bytes_written_ += bytes_written;
    return bytes_written;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    // in case len is greater than buffer_size()
    size_t copy_len = min(buffer_size(), len);
    string ret = bytes_.substr(start_i_, copy_len);
    size_t left = copy_len - ret.size();
    ret += bytes_.substr(0, left);
    
    return ret;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    read(len);
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string ret = peek_output(len);
    size_t bytes_read = ret.size();
    start_i_ = (start_i_ + bytes_read) % bytes_.size();
    bytes_read_ += bytes_read;
    return ret;
}

void ByteStream::end_input() { is_end_ = true; }

bool ByteStream::input_ended() const { return is_end_; }

size_t ByteStream::buffer_size() const { 
    ssize_t sz = end_i_ - start_i_;
    if(sz < 0) {
        sz += bytes_.size();
    }
    return sz;
}

bool ByteStream::buffer_empty() const { return !buffer_size(); }

bool ByteStream::eof() const { return buffer_empty() && input_ended(); }

size_t ByteStream::bytes_written() const { return bytes_written_; }

size_t ByteStream::bytes_read() const { return bytes_read_; }

size_t ByteStream::remaining_capacity() const { return capacity_ - buffer_size(); }
