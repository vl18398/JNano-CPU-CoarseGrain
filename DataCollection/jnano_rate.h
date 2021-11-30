/**
 * @file jnano_rate.h
 * @author cs
 * @brief This header file contains the functions for setting and reading
 * Jetson Nano's GPU and EMC frequncies.
 */
#ifndef NANO_RATE_H
#define NANO_RATE_H

/**
 * @brief Units with adjustable operating frequency
 */
typedef enum jnano_units {
    EMC_RATE = 0, ///< external memory controller (EMC)
    GPU_RATE, ///< graphics processing unit (GPU)
    CPU0_RATE, ///< first core of central processing unit (CPU)
    CPU1_RATE, ///< second core of CPU
    CPU2_RATE, ///< third core of CPU
    CPU3_RATE, ///< fourth core of CPU
} jnano_unit;

/**
 * @brief Read operating frequency
 * @param unit See ::jnano_unit
 * @param *rate Output's reference
 */
void jnano_get_rate(const jnano_unit unit, unsigned long *rate);

/**
 * @brief Set operating frequency
 * @param unit See ::jnano_unit
 * @param rate Operating frequency 
 */
void jnano_set_rate(const jnano_unit unit, const unsigned long rate);

#endif
