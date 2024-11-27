// buffer.h
#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uint8_t *data;     // The buffer's data array
    size_t size;       // Total capacity of the buffer
    size_t read;   // Position where the next read will occur
    size_t write;  // Position where the next write will occur
} buffer;

// Initialize the buffer with a given size and data array
void buffer_init(buffer *b, uint8_t *data, size_t size);

// Check if the buffer can write more data
bool buffer_can_write(const buffer *b);

// Check if the buffer has any data to read
bool buffer_can_read(const buffer *b);

// Write a byte to the buffer
void buffer_write(buffer *b, uint8_t byte);

// Read a byte from the buffer
uint8_t buffer_read(buffer *b);

// Reset the buffer by clearing the read and write positions
void buffer_reset(buffer *b);

// Get the current size of the available data in the buffer
size_t buffer_available_read(const buffer *b);

// Get the remaining space available in the buffer
size_t buffer_available_write(const buffer *b);

void buffer_read_adv(buffer *b, size_t bytes);

void buffer_write_adv(buffer *b, size_t bytes);

uint8_t *buffer_read_ptr(buffer *b, size_t *nbyte);

#endif
