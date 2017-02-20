#ifndef _CIVCC_MYTYPES_H_
#define _CIVCC_MYTYPES_H_


/*
 * This enumeration defines all unary operations
 */
typedef enum { UO_not, UO_neg, UO_unknown } unop;

/*
 * This enumeration defines all data types
 */
typedef enum { TY_int, TY_bool, TY_float, TY_unknown } type;

/*
 * This enumeration defines all binary operations
 */
typedef enum { BO_add, BO_sub, BO_mul, BO_div, BO_mod, 
               BO_lt, BO_le, BO_gt, BO_ge, BO_eq, BO_ne, 
               BO_and, BO_or, BO_unknown } binop;


#endif  /* _CIVCC_MYTYPES_H_ */
