#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

#include "bn.h"

#define clz(t) __builtin_clz(t)
#define unlikely(x) __builtin_expect(!!(x), 0)


void bn_add(bn a, bn b, bn *sum)
{
    unsigned int N[LENGTH];
    unsigned long long c = 0;
    for (int i = 0; i < LENGTH; ++i) {
        c += (uint64_t) a.num[i] + b.num[i];
        N[i] = c;
        c >>= 32;
    }

    for (int i = LENGTH - 1; i >= 0; --i) {
        sum->num[i] = N[i];
    }
}

void bn_dec(bn a, bn b, bn *diff)  // a - b
{
    unsigned int N[LENGTH];
    unsigned int br = 0;
    for (int i = 0; i < LENGTH; ++i) {
        if (a.num[i] < (b.num[i] + br)) {
            N[i] = UINT_MAX - b.num[i] + a.num[i] - br + 1;
            br = 1;
        } else {
            N[i] = a.num[i] - b.num[i] - br;
            br = 0;
        }
    }

    for (int i = LENGTH - 1; i >= 0; --i) {
        diff->num[i] = N[i];
    }
}

void bn_lshift(bn a, int shift, bn *res)
{
    int shift_32b_amount = shift >> 5;

    memcpy(res, &a, sizeof(bn));
    unsigned int tmp[shift_32b_amount + 1];
    unsigned int tmp2[shift_32b_amount + 1];
    unsigned int i;
    int shift_bit = shift & 0x0000001f;

    memset(tmp, 0, sizeof(int) * shift_32b_amount);

    if (shift_32b_amount) {
        for (int k = 0; k < shift_32b_amount; ++k) {
            tmp[k] = res->num[k];
            res->num[k] = 0;
        }

        for (i = shift_32b_amount; i <= LENGTH - shift_32b_amount;
             i += shift_32b_amount) {
            memcpy(tmp2, &res->num[i], sizeof(uint32_t) * shift_32b_amount);
            memcpy(&res->num[i], tmp, sizeof(uint32_t) * shift_32b_amount);
            memcpy(tmp, tmp2, sizeof(uint32_t) * shift_32b_amount);
        }
        if (shift_32b_amount >= (LENGTH >> 1) + 1) {
            memcpy(&res->num[shift_32b_amount], tmp,
                   sizeof(uint32_t) * (LENGTH - shift_32b_amount));
        }
    }
    if (shift_bit) {
        tmp[0] = (res->num[0] >> (32 - shift_bit));
        res->num[0] <<= shift_bit;
        for (i = 0; i < LENGTH - 1; ++i) {
            tmp2[0] = (res->num[i + 1] >> (32 - shift_bit));
            res->num[i + 1] = res->num[i + 1] << shift_bit | tmp[0];
            tmp[0] = tmp2[0];
        }
    }
}

#if 0 
void bn_rshift(bn a, int shift, bn *res)
{
    int shift_32b_amount = shift >> 5;
    unsigned int tmp[shift_32b_amount + 1];
    unsigned int tmp2[shift_32b_amount + 1];
    memcpy(res, &a, sizeof(bn));
    int shift_bit = shift & 0x0000001f;

    memset(tmp, 0, sizeof(int) * shift_32b_amount);

    if (shift_32b_amount) {
        for (int i = LENGTH - 1, j = shift_32b_amount - 1;
             i >= LENGTH - shift_32b_amount; --i, j--) {
            tmp[j] = res->num[i];
            res->num[i] = 0;
        }

        for (int i = LENGTH - shift_32b_amount; i >= shift_32b_amount - 1;
             i -= shift_32b_amount) {
            memcpy(tmp2, &res->num[i - shift_32b_amount],
                   sizeof(uint32_t) * shift_32b_amount);
            memcpy(&res->num[i - shift_32b_amount], tmp,
                   sizeof(uint32_t) * shift_32b_amount);
            memcpy(tmp, tmp2, sizeof(uint32_t) * shift_32b_amount);
        }
        if (shift_32b_amount >= (LENGTH >> 1) + 1) {
            memcpy(&res->num[0], &tmp[(shift_32b_amount << 1) - LENGTH],
                   sizeof(uint32_t) * (LENGTH - shift_32b_amount));
        }
    }
    if (shift_bit) {
        tmp[0] = (res->num[LENGTH - 1] << (32 - shift_bit));
        res->num[7] >>= shift_bit;
        for (int i = LENGTH - 1; i >= 1; --i) {
            tmp2[0] = (res->num[i - 1] << (32 - shift_bit));
            res->num[i - 1] = res->num[i - 1] >> shift_bit | tmp[0];
            tmp[0] = tmp2[0];
        }
    }
}
#endif

char *bn_to_string(bn a)
{
    char s[8 * sizeof(uint32_t) * LENGTH / 3 + 2];
    unsigned int n[LENGTH];
    int i;
    memset(s, '0', sizeof(s) - 1);
    memcpy(n, a.num, sizeof(uint32_t) * LENGTH);
    s[sizeof(s) - 1] = '\0';
    // printf("%ld\n", sizeof(a));
    for (i = 0; i < 8 * sizeof(uint32_t) * LENGTH; ++i) {
        int carry;
        carry = (n[LENGTH - 1] >= 0x80000000);
        for (int j = LENGTH - 1; j >= 0; --j) {
            n[j] = ((n[j] << 1) & 0xffffffff) +
                   ((j - 1) >= 0 ? (n[j - 1] >= 0x80000000) : 0);
        }

        for (int j = sizeof(s) - 2; j >= 0; j--) {
            s[j] += s[j] - '0' + carry;
            carry = (s[j] > '9');
            if (carry)
                s[j] -= 10;
        }
    }
    i = 0;
    while (i < sizeof(s) - 2 && s[i] == '0')
        i++;
    char *dec = (char *) kmalloc(sizeof(s) - i, GFP_KERNEL);
    memcpy(dec, &s[i], sizeof(s) - i);

    return dec;
}

void bn_mul_2(bn a, bn b, bn *res)
{
    unsigned int t[LENGTH];
    unsigned long long c1, c2;
    // memset(tmp, 0,sizeof(uint64_t) * LENGTH);
    memset(t, 0, sizeof(uint32_t) * LENGTH);
    for (int i = 0; i < LENGTH; ++i) {
        for (int j = 0; j < LENGTH; ++j) {
            if ((i + j) < LENGTH) {
                c1 = (uint64_t) a.num[i] * b.num[j];
                c2 = 0;
                for (int k = i + j; k < LENGTH; ++k) {
                    c2 += (uint64_t) t[k] + (c1 & 0xffffffff);
                    t[k] = c2 & 0xffffffff;
                    c2 >>= 32;
                    c1 >>= 32;
                    if (!c1 && !c2) {
                        break;
                    }
                }
            }
        }
    }
    for (int i = LENGTH - 1; i >= 0; --i) {
        res->num[i] = t[i];
    }
}

#if 0
void bn_norm_fib(unsigned int n, bn *res)
{
    if (unlikely(!n)) {
        memset(res, 0, sizeof(bn));
        return;
    }
    bn a, b;
    memset(&a, 0, sizeof(bn));
    memset(&b, 0, sizeof(bn));
    memset(res, 0, sizeof(bn));
    a.num[0] = 0;
    b.num[0] = 1;
    res->num[0] = 1;
    for (int i = 1; i < n; ++i) {
        bn_add(a, b, res);
        memcpy(&a, &b, sizeof(bn));
        memcpy(&b, res, sizeof(bn));
    }
}
#endif

void bn_fib(unsigned int n, bn *res)
{
    if (unlikely(!n)) {
        memset(res, 0, sizeof(bn));
        return;
    }

    bn a, b;
    memset(&a, 0, sizeof(bn));
    memset(&b, 0, sizeof(bn));
    a.num[0] = 0;
    b.num[0] = 1;

    for (unsigned int mask = 1 << (__builtin_clz(n)); mask; mask >>= 1) {
        bn t1;
        bn t2;
        memset(&t1, 0, sizeof(bn));
        memset(&t2, 0, sizeof(bn));

        bn_lshift(b, 1, &t1);
        bn_dec(t1, a, &t1);
        bn_mul_2(a, t1, &t1);

        bn_mul_2(a, a, &t2);
        bn_mul_2(b, b, &b);
        bn_add(t2, b, &t2);

        memcpy(&a, &t1, sizeof(bn));
        memcpy(&b, &t2, sizeof(bn));

        if (mask & n) {
            bn_add(a, b, &t1);
            memcpy(&a, &b, sizeof(bn));
            memcpy(&b, &t1, sizeof(bn));
        }
    }
    memcpy(res, &a, sizeof(bn));
}