#pragma once

#define EIGEN_DEFINE_BIT_MASK_OPS(type)                                             \
    inline bool Any(type a)                     { return (int)a != 0; }             \
    inline bool None(type a)                    { return (int)a == 0; }             \
    inline type operator~(type a)               { return (type)~(int)a; }           \
    inline type operator|(type a, type b)       { return (type)((int)a | (int)b); } \
    inline type operator&(type a, type b)       { return (type)((int)a & (int)b); } \
    inline type operator^(type a, type b)       { return (type)((int)a ^ (int)b); } \
    inline type& operator|=(type& a, type b)    { return a = a | b; }               \
    inline type& operator&=(type& a, type b)    { return a = a & b; }               \
    inline type& operator^=(type& a, type b)    { return a = a ^ b; }
