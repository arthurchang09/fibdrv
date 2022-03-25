#define LENGTH 22
/*
 * Struct that store big number
 */
typedef struct bn {
    unsigned int num[LENGTH];
} bn;


/*
 * Add two bn and put the result in target bn.
 */
void bn_add(bn a, bn b, bn *sum);

/*
 * Substract two bn, a - b, and put the result in target bn.
 */
void bn_dec(bn a, bn b, bn *diff);

/*
 * left shift serveral bits of  bn.
 */
void bn_lshift(bn a, int shift, bn *res);

#if 0 
/*
 * right shift several bits of bn.
 */
void bn_rshift(bn a, int shift, bn *res);
#endif

/*
 * print the bn in decimal form and return the whole string
 */
int bn_to_string(bn a, char str[]);

/*
 * Multiply two bn by long multipication and put the sesult in target bn.
 */
void bn_mul_2(bn a, bn b, bn *res);

#if 0
/*
 * Use iteration to calculate Fib Sequence
 */
void bn_norm_fib(unsigned int n, bn *res);
#endif

/*
 * Use Fast doubling to calculate Fib Sequence
 */
void bn_fib(unsigned int n, bn *res);