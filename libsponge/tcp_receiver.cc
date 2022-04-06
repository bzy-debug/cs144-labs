#include "tcp_receiver.hh"
#include "wrapping_integers.hh"
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.


using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (seg.header().syn && !_isn) {
        _isn.emplace(seg.header().seqno);
    }
    if (!_isn)
        return;
    size_t checkpoint = _reassembler.get_next() == 0 ? 0 : _reassembler.get_next() - 1;
    if (seg.header().seqno == _isn.value() && !seg.header().syn)
        checkpoint = 1ul << 32;
    uint64_t index = unwrap(seg.header().seqno, _isn.value(), checkpoint);
    index = index == 0 ? 0 : index - 1;
    string data {seg.payload().str().begin(), seg.payload().str().end()};
    _reassembler.push_substring(data, index, seg.header().fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_isn)
        return nullopt;
    return wrap(_reassembler.get_next() + 1 + (_reassembler.stream_out().input_ended() ? 1 : 0), _isn.value());
}

size_t TCPReceiver::window_size() const {
    return _capacity - _reassembler.stream_out().buffer_size();
}
