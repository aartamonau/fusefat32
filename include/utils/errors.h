/**
 * @file   errors.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Thu Oct  1 23:56:03 2009
 * 
 * @brief  Utility functions and macros for error handling.
 * 
 * 
 */

#ifndef _ERRORS_H_
#define _ERRORS_H

/** 
 * Macro checks that the return value of expression is non-negative.
 * If it's not then return from the function is done with -1 value.
 * 
 * @param expr expression to check
 * 
 * @return no return value
 */
#define CHECK_NN(expr) if ((expr) < 0) return -1

/** 
 * @link CHECK_NN @endlink which allows to specify your own value to return
 * on error.
 * 
 * @param expr 
 * @param ret 
 * 
 * @return 
 */
#define CHECK_NN_RET(expr, ret) if ((expr) < 0) return (ret)

#endif
