/*
 * numbers.h
 *
 *  Created on: 2 Mar 2017
 *      Author: nico
 */

#ifndef SRC_UTILS_NUMBERS_H_
#define SRC_UTILS_NUMBERS_H_

/*
 * Converts a character representation of a integer number into a int.
 * A range check is performed to ensure the number does not fall outside
 * the allowed range [−2147483647, +2147483647].
 * Returns NULL of the string provided is out of range, otherwise a
 * pointer to the converted value is returned. The caller is responsible
 * for freeing the memory.
 */
int *strToInt(char * s);

/*
 * Converts a character representation of a floatingpoint number into a float.
 * A range check is performed to ensure the number does not fall outside
 * the allowed range [].
 * Returns NULL of the string provided is out of range, otherwise a
 * pointer to the converted value is returned. The caller is responsible
 * for freeing the memory.
 */
float *strToFloat(char * s);

#endif /* SRC_UTILS_NUMBERS_H_ */
