/**
 * @file util.h
 * @author Daniel Schloms <e11701253@student.tuwien.ac.at>
 * @date 08.11.18
 *  
 * @brief Contains a function used by mydiff
 *
 *
 */
 
#ifndef UTIL_H__ 
#define UTIL_H__
 
/**
 * @brief This function compares 2 lines passed on by the main function.
 * @details This function takes 2 lines, compares them char by char and counts the
 * amount of mismatches. If mode_i is > 0 (option -i is used) then each
 * letter is changed to it's lowercase variant.
 */

void compareLine(char line1[], char line2[], int lineCount, int mode_i, char *output, int buffer);
 
#endif /* UTIL_H__ */
