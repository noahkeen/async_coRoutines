#pragma once

#include <vector>
#include <atomic>

/**
 * A thread safe FIFO ring buffer implementation
 * Uses atomics instead of mutexes cause faster
 *
 * Message based. Where message is Type T(e.g.: std::string_view).
 *
 * Currently good for single writer single reader
 * Coming soon: multiple writer
 *
 * If you need a (char/byte)buffer implementation where data received is written contiguously in memory
 * see the CircularRingBuffer
 */
template<typename T>
class FifoCircularMessageBuffer {
    static constexpr size_t DEFAULT_SIZE = 2048;
    bool write_in_front_of_read = true;

    uint64_t initial_capacity_;
    std::vector<T> data_;
    std::atomic<uint64_t> read_;
    std::atomic<uint64_t> write_;

public:

    explicit FifoCircularMessageBuffer(uint64_t initialCapacity = DEFAULT_SIZE) :
            initial_capacity_(initialCapacity),
            data_(initial_capacity_),
            read_(0),
            write_(0) {
    }

    /**
     * @param to_write Value to be copied into the next available push index
     * @return False if no space to push
     */
    bool push(const T& to_write) {
        uint64_t cur_write = write_.load();
        uint64_t cur_read = read_.load();
        if (write_in_front_of_read) {
            if (cur_write >= cur_read) {
                data_[cur_write] = to_write;
                uint64_t next_write = cur_write + 1;
                if (next_write == initial_capacity_) {
                    next_write = 0;
                    write_in_front_of_read = false;
                }
                write_.store(next_write);
                return true;
            } else {
                return false;
            }
        } else {
            if (cur_write < cur_read) {
                data_[cur_write] = to_write;
                uint64_t next_write = cur_write + 1;
                write_.store(next_write);
                return true;
            } else {
                return false;
            }
        }
    }

    /**
     * Reads the next available message. Consumes the message when read and advances the pop position.
     *
     * @param msg Reference will have the next available entry assigned when True is returned. Nothing if false
     * @return False if nothing to pop
     */
    bool pop(T &msg) {
        uint64_t cur_write = write_.load();
        uint64_t cur_read = read_.load();
        if (write_in_front_of_read) {
            if (cur_read < cur_write) {
                msg = data_[cur_read];
                uint64_t next_read = cur_read + 1;
                read_.store(next_read);
                return true;
            } else {
                return false;
            }
        } else {
            if (cur_read >= cur_write) {
                msg = data_[cur_read];
                uint64_t next_read = cur_read + 1;
                if (next_read == initial_capacity_) {
                    next_read = 0;
                    write_in_front_of_read = true;
                }
                read_.store(next_read);
                return true;
            } else {
                return false;
            }
        }
    }

};