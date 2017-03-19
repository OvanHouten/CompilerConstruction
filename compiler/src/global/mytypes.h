#ifndef _CIVCC_MYTYPES_H_
#define _CIVCC_MYTYPES_H_


/*
 * This enumeration defines two different Symbol Table Entry types
 */
typedef enum { STE_vardef, STE_fundef } ste_type;

/*
 * This enumeration defines all unary operations
 */
typedef enum { UO_not, UO_neg, UO_unknown } unop;

/*
 * This enumeration defines all data types
 */
typedef enum { TY_int, TY_bool, TY_float, TY_void, TY_unknown } type;

/*
 * This enumeration defines all arithmatic operations
 */
typedef enum { AO_add, AO_sub, AO_mul, AO_div, AO_mod, AO_unknown } arithop;

/*
 * This enumeration defines all relational operations
 */
typedef enum { RO_lt, RO_le, RO_gt, RO_ge, RO_eq, RO_ne, RO_unknown } relop;

/*
 * This enumeration defines all logical operations
 */
typedef enum { LO_and, LO_or, LO_unknown } logicop;


#endif  /* _CIVCC_MYTYPES_H_ */
