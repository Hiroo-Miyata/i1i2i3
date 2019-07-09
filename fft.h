#ifndef FFT_H
#define FFT_H
#pragma once

typedef short sample_t;

ssize_t read_n(int, ssize_t, void *);
ssize_t write_n(int, ssize_t, void *);
void sample_to_complex(sample_t *, complex double *, long);
void complex_to_sample(complex double *, sample_t *, long);
void fft_r(complex double *, complex double *, long, complex double);
void fft(complex double *, complex double *, long);
void ifft(complex double *, complex double *, long);
int pow2check(long);
void print_complex(FILE *, complex double *, long);
int all(int, char **);

#endif