#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) 
  :  _output(capacity), _capacity(capacity), _next {0}, _datagrams{}
{
}

void StreamReassembler::insert_datagram(list<datagram>& datagrams, datagram&& dg) {
  auto& [data, index, eof] = dg;
  if (_next > index) {
    data.erase(0, _next - index);
    index = _next;
  }

  if (datagrams.empty()) {
    datagrams.insert(datagrams.end(), dg);
    return;
  }

  auto before = datagrams.end();
  auto first = datagrams.end();
  auto last = datagrams.end();
  auto after = datagrams.end();
  for (auto iter = datagrams.begin(); iter != datagrams.end(); iter++) {
    const auto& [s, i, e] = *iter;
    if (i <= index && i + s.size() >= index + data.size())
      return;
    if (i + s.size() - 1 < index)
      before = iter;
    if (i < index && index <= i + s.size() - 1)
      first = iter;
    if (i <= index + data.size() - 1 && index + data.size() - 1 <= i + s.size() - 1)
      last = iter;
    if (after == datagrams.end() && index + data.size() - 1 < i)
      after = iter;
  }
  
  if (first != datagrams.end()) {
    auto& [s, i, e] = *first;
    s.resize(index - i);
  }
  if (last != datagrams.end()) {
    auto& [s, i, e] = *last;
    s.erase(0, index + data.size() - i);
    i = index + data.size();
  }

  if (first != datagrams.end() && last != datagrams.end()) {
    datagrams.erase(next(first, 1), last);
    datagrams.insert(last, dg);
  }
  else if (first == datagrams.end() && last != datagrams.end()) {
    datagrams.erase(before == datagrams.end() ? datagrams.begin() : next(before, 1), last);
    datagrams.insert(last, dg);
  }
  else if (first != datagrams.end() && last == datagrams.end()) {
    datagrams.erase(next(first, 1), after);
    datagrams.insert(after, dg);    
  }
  else {
    datagrams.erase(before == datagrams.end() ? datagrams.begin() : next(before, 1), after);
    datagrams.insert(after, dg);
  }
  
  size_t size = _output.buffer_size();
  for (auto d : _datagrams)
    size += get<0>(d).size();
  while (size > _capacity) {
    size_t more = size - _capacity;
    auto end = prev(_datagrams.end(), 1);
    if (get<0>(*end).size() > more) {
      get<0>(*end).resize(get<0>(*end).size() - more);
      get<2>(*end) = false;
      size -= more;
    }
    else {
      _datagrams.erase(end);
      size -= get<0>(*end).size();
    }
  }
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
  datagram new_dg {data, index, eof};
  insert_datagram(_datagrams, move(new_dg));

  for (auto iter = _datagrams.begin(); iter != _datagrams.end() && !_datagrams.empty() && get<1>(*iter) == _next; iter = _datagrams.begin()) {
    _next += _output.write(get<0>(*iter));
    if (get<2>(*iter))
      _output.end_input();
    _datagrams.erase(iter);
  }
}

size_t StreamReassembler::unassembled_bytes() const {
  size_t sum = 0;
  for (auto dg: _datagrams) {
    sum += get<0>(dg).size();
  }
  return sum;
}

bool StreamReassembler::empty() const { return _datagrams.empty(); }
