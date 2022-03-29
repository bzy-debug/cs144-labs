#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

using namespace std;

ByteStream::ByteStream(const size_t capacity) 
  :_capacity {capacity}, bytes {}
{
}

size_t ByteStream::write(const string &data) {
    size_t len = min(data.size(), this->remaining_capacity());
    auto iter = data.begin();
    for (size_t i = 0; i < len; i++) {
      bytes.push_back(*iter);
      iter++;
    }
    written += len;
    return len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
  const size_t p_len = min(bytes.size(), len);
  string msg {bytes.begin(), bytes.begin() + p_len};
  return msg;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
  const size_t p_len = min(bytes.size(), len);
  bytes.erase(bytes.begin(), bytes.begin() + p_len);
  popped += p_len;
  return;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
  string msg {peek_output(len)};
  pop_output(len);
  return msg;
}

void ByteStream::end_input() {
  _eof = true;
}

bool ByteStream::input_ended() const {
  return _eof;
}

size_t ByteStream::buffer_size() const {
  return bytes.size();
}

bool ByteStream::buffer_empty() const {
  return bytes.empty();
}

bool ByteStream::eof() const {
  return bytes.empty() && _eof;
}

size_t ByteStream::bytes_written() const { return written; }

size_t ByteStream::bytes_read() const { return popped; }

size_t ByteStream::remaining_capacity() const {
  return _capacity - bytes.size();
}
