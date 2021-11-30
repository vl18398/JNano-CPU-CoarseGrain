/**
 * @file jnano_var.h
 * @author cs
 * @brief This header file contains Jetson Nano data and parameters.
 */
#ifndef JNANO_VAR_H
#define JNANO_VAR_H

/**
 * @brief Available frequencies of Jetson Nano's GPU
 */
static const unsigned long jnano_gpu_freqs[] = {
    76800000
    ,153600000
    ,230400000
    ,307200000
    ,384000000
    ,460800000
    ,537600000
    ,614400000
    ,691200000
    ,768000000
    ,844800000
    ,921600000
    ,998400000
}; // [Hz]

/**
 * @brief Available frequencies of Jetson Nano's EMC
 */
static const unsigned long jnano_emc_freqs[] = {
    /*40800000, 68000000, 102000000,*/ // problematic
    204000000,
    408000000,
    665600000,
    800000000,
    1065600000,
    1331200000,
    1600000000
}; // [Hz]

#endif
