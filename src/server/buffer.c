#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "buffer.h"

// Initialize the buffer with the provided data array and size
void buffer_init(buffer *b, uint8_t *data, size_t size) {
    b->data = data;
    b->size = size;
    b->read = 0;
    b->write = 0;
}

//  Getters for read and write
uint8_t *buffer_read_ptr(buffer *b, size_t *nbyte) {
    // If there is data available to read, return the pointer to the data
    if (buffer_can_read(b)) {
        *nbyte = buffer_available_read(b);
        return &b->data[b->read];
    } else {
        *nbyte = 0;
        return NULL;  // Return NULL if there's no data to read
    }
}
// Check if the buffer can write more data
bool buffer_can_write(const buffer *b) {
    return b->write < b->size;
}

// Check if the buffer has any data to read
bool buffer_can_read(const buffer *b) {
    return b->read < b->write;
}

// Write a byte to the buffer
void buffer_write(buffer *b, uint8_t byte) {
    if (buffer_can_write(b)) {
        b->data[b->write] = byte;
        b->write++;
    }
}

// Read a byte from the buffer
uint8_t buffer_read(buffer *b) {
    uint8_t byte = 0;
    if (buffer_can_read(b)) {
        byte = b->data[b->read];
        b->read++;
    }
    return byte;
}

// Reset the buffer, clearing the read and write positions
void buffer_reset(buffer *b) {
    b->read = 0;
    b->write = 0;
}

// Get the current size of the available data in the buffer
size_t buffer_available_read(const buffer *b) {
    return b->write - b->read;
}

// Get the remaining space available in the buffer
size_t buffer_available_write(const buffer *b) {
    return b->size - b->write;
}

// Advance the read pointer by a specific number of bytes
void buffer_read_adv(buffer *b, size_t bytes) {
    assert(b->read + bytes <= b->write); // Ensure we're not reading beyond the write position
    b->read += bytes;

    // Optional: if the buffer is empty after advancing, reset the read/write pointers
    if (b->read == b->write) {
        buffer_reset(b);
    }
}

void buffer_write_adv(buffer *b, size_t bytes) {
    assert(b->write + bytes <= b->size); // Ensure we're not writing beyond the buffer's capacity
    b->write += bytes;
}