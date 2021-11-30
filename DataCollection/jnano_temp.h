/**
 * @file jnano_temp.h
 * @author cs
 * @brief This header file contains the functions for reading
 * Jetson Nano's values of thermal zones.
 */
#ifndef JNANO_TEMP_H_
#define JNANO_TEMP_H_

#define MAX_BUFF 128
#define SYSFS_TEMP_PATH "/sys/class/thermal"

/**
 * @brief Thermal zones index
 */
typedef enum jnano_tzones {
    A0 = 0, ///< on-chip thermal zone (mC)
    CPU, ///< on-chip thermal zone (mC)
    GPU, ///< on-chip thermal zone (mC)
    PLL, ///< on-chip thermal zone (mC)
    PMIC, ///< on-chip thermal zone (mC)
    TDIODE, ///< on-module thermal zone (mC)
    TBOARD, ///< on-module thermal zone (mC)
    FAN ///< on-chip thermal zone (mC)
} jnano_tzone;


/**
 * @brief Read on-chip and on-module temperatures.
 *
 * @param zone Indexed by ::jnano_tzone
 * @param *temperature Output's reference
 */
void jnano_get_temp(jnano_tzone zone,
		   unsigned int *temperature);

#endif // JNANO_TEMP_H_
