#ifndef _CIVCC_MYTYPES_H_
#define _CIVCC_MYTYPES_H_


/*
 * This enumeration defines the different Symbol Table Entry types
 */
typedef enum { STE_vardef, STE_varusage, STE_fundef } ste_type;

/*
 * This enumeration defines all unary operations
 */
typedef enum { UO_not, UO_neg, UO_unknown } unop;

/*
 * This enumeration defines all data types
 */
typedef enum { TY_int, TY_bool, TY_float, TY_void, TY_unknown } type;

/*
 * This enumeration defines all binary operations
 */
typedef enum { BO_add, BO_sub, BO_mul, BO_div, BO_mod, BO_lt, BO_le, BO_gt, BO_ge, BO_eq, BO_ne, BO_and, BO_or, BO_unknown } binop;

/*
 * This enumeration defines the compund inc and dec instructions
 */
typedef enum { CO_inc, CO_dec, CO_unknown } cop;

/*
 * Defined the three possible 'locations' for variables and functions.
 */
typedef enum { LOC_extern, LOC_global, LOC_local } location;

#endif  /* _CIVCC_MYTYPES_H_ */
