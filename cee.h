/* CEE - C Ease-of-use Extensions */
/* TODO:
 *  - could sizeof(**ptr) be used to enforce void ** checks:
 *      - #define free_ptr(ptr) free_ptr((sizeof(**ptr), ptr))
 *      - would this fail with a literal void ** rather than normal int **
 *      - usage: int *x = malloc(sizeof(int)); free_ptr(&x); // x now == 0
 *
 */

#ifndef CEE_H

// for data in global scope
#define global   static
// for data in function scope
#define persist  static
// TODO: change to "local"
// for functions
#define internal static
// for array function parameters
#define atleast static

#define CEE_CAT1(a,b) a ## b
#define CEE_CAT2(a,b) CEE_CAT1(a,b)
#define CEE_CAT(a,b)  CEE_CAT2(a,b)

#define cee_n CEE_CAT(cee_internal_n_, __LINE__)
#define cee_a CEE_CAT(cee_internal_a_, __LINE__)
#define cee_b CEE_CAT(cee_internal_b_, __LINE__)
#define CEE_LINE(a) CEE_CAT(a, __LINE__)

#define CEE_FIRST(a, ...) a
#define CEE_FIRST_ONLY(...) CEE_FIRST(__VA_ARGS__, DUMMY)
#define CEE_REST(a, ...) __VA_ARGS__

#define CEE_SECOND(a, b, ...) b
#define CEE_HAS_MULTIPLE(...) CEE_SECOND((__VA_ARGS__), (DUMMY1), DUMMY2)

// compile-time assert:
/* #define ct_assert(e) struct { char compile_time_assert[(e) ? 1 : -1]; } */
#define ct_assert(e) typedef char CEE_CAT(compile_time_assertion, __LINE__)[(e) ? 1 : -1]

#define CEE_IF_0(...)
#define CEE_IF_1(...) __VA_ARGS__
#define IF(pred, ...) CEE_CAT(CEE_IF_, pred)(__VA_ARGS__)
#define CEE_IFE_0(t, f) f
#define CEE_IFE_1(t, f) t
#define IF_ELSE(pred, t, f) CEE_CAT(CEE_IFE_, pred)(t, f)

#if 1 // TYPES //

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// bytes //
typedef uint8_t  U1;
typedef uint16_t U2;
typedef uint32_t U4;
typedef uint64_t U8;

typedef  int8_t  I1;
typedef  int16_t I2;
typedef  int32_t I4;
typedef  int64_t I8;

typedef  int8_t  S1;
typedef  int16_t S2;
typedef  int32_t S4;
typedef  int64_t S8;

typedef  int8_t  B1;
typedef  int16_t B2;
typedef  int32_t B4;
typedef  int64_t B8;

typedef float    F4;
typedef double   F8;

// bits //
typedef uint8_t  U8b;
typedef uint16_t U16b;
typedef uint32_t U32b;
typedef uint64_t U64b;

typedef  int8_t  I8b;
typedef  int16_t I16b;
typedef  int32_t I32b;
typedef  int64_t I64b;

typedef  int8_t  S8b;
typedef  int16_t S16b;
typedef  int32_t S32b;
typedef  int64_t S64b;

typedef  int8_t  B8b;
typedef  int16_t B16b;
typedef  int32_t B32b;
typedef  int64_t B64b;

typedef float    F32b;
typedef double   F64b;

// named //
typedef struct { unsigned char val; } Byte;
typedef char      Char;
typedef bool     Bool;
typedef size_t    Size;
typedef uintptr_t UPtr;
typedef intptr_t  SPtr;
typedef void *    Addr;

#define ARR_LEN_BITS(bits) ((bits / CHAR_BIT) + (bits % CHAR_BIT > 0))

// TODO: aligned versions

// TYPE CREATION //
// NOTE: if you don't add the brackets these just act as normal: e.g. struct MyType vs struct (MyType)
/* #define struct(s) struct s; typedef struct s s; struct s */
/* #define union(s)  union  s; typedef union  s s; union  s */
/* #define enum(s)   enum   s; typedef enum   s s; enum   s */

// create a label as well as a case jump point
/* #define case(c) case c: case_## c */

// CASTING //

// TODO: option to use _Generic or similar mechanism to check both in & out type for cast.
//       (you want to have the in type to catch any times when you change the in typedef)
#define cast(type, data) ((type)(data))
#define ptr_cast(type, data) ((type)(uintptr_t)(data))
// "bitcast"?
#define recast(ti, to, value) (((union { ti in; char b[sizeof(value)]; to out; }){value}).out)

#endif // TYPES

#if 1 // VALUES
#if 0 // ID VALUES
// 7 char string as numeric value
// NOTE: d7 had implementation
#define STR_ID_(s, n) (((s "\0\0\0\0\0\0\0\0")[n]) << n)
#define STR_ID1(s) STR_ID_(s,0)|STR_ID_(s,1)|STR_ID_(s,2)|STR_ID_(s,3)|STR_ID_(s,4)|STR_ID_(s,5)|STR_ID_(s,6) & ~(0xFF << 7)

/* #define STR__ID(s, n) (s[n] ? s[n] : 0) << 7-n */
#define STR__ID(s, n) (((s "\0\0\0\0\0\0\0\0")[n]) << (7-n))
#define STR_ID2(s) \
    STR__ID(s,0)| \
STR__ID(s,1) |\
STR__ID(s,2) |\
STR__ID(s,3) |\
STR__ID(s,4) |\
STR__ID(s,5) |\
STR__ID(s,6) &\
~0xFF
typedef union CeeStringId {
    U8   val;
    char str[8];
} CeeStringId;
#endif // ID VALUES

#define TYPE_IS_SIGNED(t) ((t)-1 < 0)
#define VAR_IS_SIGNED(v) ((((v)^(v))-1) < 0)

// INVALID VALUES //

#define MIN_SIGNED(type) ((type)1 << (sizeof(type) * 8 - 1))
#define MAX_UNSIGNED(type) (~(type)0)

#define IS_MIN_SIGNED(v) ((v) == ((((v)^(v))+1) << (sizeof(v) * 8 - 1)) )

// for unsigned: all bits set to 1
// TODO: for signed: MIN_SIGNED
// makes valid range for signed integers symmetrical
// if only unsigned integers are needed, only loses top value
// rather than losing half of available as when `value < 0` -> invalid

#define INVALID(type) (TYPE_IS_SIGNED(type) ? MIN_SIGNED(type) : MAX_UNSIGNED(type))
// e.g. memset(new_array, INVALID(char), sizeof(new_array));
#define is_invalid(value) (VAR_IS_SIGNED(value) ? IS_MIN_SIGNED(value) : !~(value))
#define is_valid(value) (VAR_IS_SIGNED(value)   ? ! IS_MIN_SIGNED(value) : !!~(value))

// SIZES //
#ifndef KB
#define KB(x) (1024ULL *   (x))
#define MB(x) (1024ULL * KB(x))
#define GB(x) (1024ULL * MB(x))
#define TB(x) (1024ULL * GB(x))
#endif // KB
#endif // VALUES

#if 1 // SCOPES
// TODO: not sure about naming here... see how they tend to be used in practice
// TODO: could add an extra for loop just as a check for incorrect breaks inside
#define scoped(start, end) for(char cee_n = ((start), 0); !cee_n++; end)
#define before_after(when, ...) for(char const *when = "before", *cee_n = ((__VA_ARGS__), when); cee_n; when = "after", cee_n = 0, __VA_ARGS__)
// NOTE: MUST use a type
#define scope(init, end) for(init, *cee_n = 0; !cee_n; ++cee_n, (end))
#define scope_var(init)  for(init, *cee_n = 0; !cee_n; ++cee_n)
#endif // SCOPES


#if 1 // ANNOTATION
#define NOTE(x,m) x
#define yes(m) 1
#define no(m)  0
#define nonull
// ALT for nonull
// #if DEBUG
// #define not_null(p) (assert(p && "argument is null"),p)
// #endif
//
// int my_func(nonull int *nums, int count) {...}
// #define my_func(a,b) my_func(not_null(a), b)


// to be redefined at the point of use as _:  _(a, V2_add, _(b, V2_sub, c))
// #define _(a,fn,b) INFIX_PRE(V2_,a,fn,b) => _(a, add, _(b, sub, c))
//                           vs normal use => V2_add(a, V2_sub(b, c))
#define INFIX(a, fn, b)                                   fn((a), (b))
#define INFIX_PRE(pre, a, fn, b)                   pre ## fn((a), (b))
#define INFIX_POST(post, a, fn, b)                fn ## post((a), (b))
#define INFIX_PREPOST(pre, post, a, fn, b) pre ## fn ## post((a), (b))

//#define ASSUME_  typedef char assumption_failed[-1];
#define ASSUME_CHECK_0 typedef char assumption_failed[-1];
#define ASSUME_CHECK_1
#define assume(a) CEE_CAT(ASSUME_CHECK_, ASSUME_## a)
#endif // ANNOTATION


#if 1 // LOOPS
#define array_len(...) (sizeof(__VA_ARGS__)/sizeof(*(__VA_ARGS__)))
#define countof(...) (sizeof(__VA_ARGS__)/sizeof(*(__VA_ARGS__)))
/* #ifndef  offsetof */
/* # define offsetof(st, m) (&((st *)0)->m) */
/* #endif */

#define member_of(t, m) (((t *)0)->m)
#define container_of(t, m, p) ((t *)((char*)(p) - offsetof(t, m)))

#ifndef  alignof
# define alignof(t) ((UPtr)&(member_of(struct{char c; t m;}, m)))
#endif
#define ARRAY__N(...)    (__VA_ARGS__), countof(__VA_ARGS__)
#define N__ARRAY(...)    countof(__VA_ARGS__), (__VA_ARGS__)
#define ARRAY__SIZE(...) (__VA_ARGS__), sizeof(__VA_ARGS__)
#define SIZE__ARRAY(...) sizeof(__VA_ARGS__), (__VA_ARGS__)
#define STR_LIT__SIZE(a) (a), sizeof(a)
#define SIZE__STR_LIT(a) sizeof(a), (a)
#define STR_LIT__LEN(a)  (a), (sizeof(a)-1)
#define LEN__STR_LIT(a)  (sizeof(a)-1), (a)

// TODO: typeof variants
// TODO: can define struct with all components of loop...
#define each
// NOTE: because this uses the auto-coercing void *, this will only work in C, not C++, this is required because it's unknown what level of pointer t is
#define all(t, a, ...) (t a = __VA_ARGS__, *cee_n = (void*)((char*)a + sizeof(__VA_ARGS__)); (void*)a < (void*)cee_n; ++a)
#define all_lit(t, a, ...) all(t*, a, (t[])__VA_ARGS__)
// TODO: use (unsigned) -1 to allow unsigned loops to zero with same format
//#define range(t, i, start, end) (t i = (start), cee_a = ((end) < (start) ? (t)-1 : +1), cee_n = (t)(end)-((end) < (start) ? (t)1 : 0); i != cee_n; i += cee_a)
#define range(  t, i, start, end) (t i = (start); i != (end); i += 1)
#define range_const(t, i, start, end) (t i = (start), cee_n = (end); i != cee_n; i += 1)
#define range_n(t, i, start, n, end) (t i = (start), n = (end); i != n; i += 1)
#define upto(t, i, end) (t i = 0; i < end; ++i)
#define repeat(n) for (Size cee_n = (n); cee_n-- > 0;)

#define until(e) while(! (e))
#endif // LOOPS

// JUMPS //
#define case_label(x) case x: case_## x
// case_label(3): dostuff(); break;
// case_label(4): dootherstuff(); goto case_3;
#define fallthrough

// FUNCTIONS //
// XXX: meh...
#define multiple_return(types, fn) struct fn##_result types fn
/*
 * multiple_return( {int a; int b},
 * my_function_name)() {
 * 		struct my_function_name_result result = {0};
 * 		return result;
 * }
 *
 * struct my_function_name_result result = my_function_name();
 */

#define VAR_ARGS(t, ...) N__ARRAY((t[]){ __VA_ARGS__ })

#if 0  // VAR_ARGS USAGE
typedef struct { int x; float f; } MyData;

void my_fn(Size n, MyData *data)
{   for (Size i = 0; i < n; ++i) { ... }   }

#define my_fn(...) my_fn(VAR_ARGS(MyData, __VA_ARGS__))

MyData d = {0};
my_fn({42, 3.14f}, d, {68, 6.28f});
#endif // VAR_ARGS USAGE

#define NAMED_ARGS_PTR(t, ...) (&(t){ __VA_ARGS__ })
#define NAMED_ARGS(t, ...)  ((t){ __VA_ARGS__ })


#if 0  // NAMED_ARGS USAGE
typedef struct { int data; bool add_new_lines; int repeat_n; } MyOpts;

void my_fn(int a, MyOpts opts)
{
    printf("%d: ", a);
    while (opts->repeat_n --> 0)
    {
        print("%d%s", data,
              opts->add_new_lines ? "\n" : "");
    }
}

#define my_fn(a, ...) my_fn((a), NAMED_ARGS(MyOpts, __VA_ARGS__))

my_fn(42, 12, .repeat_n=6);
#endif // NAMED_ARGS USAGE


#if 1  // BINARY
#define BITS_0000 0x0
#define BITS_0001 0x1
#define BITS_0010 0x2
#define BITS_0011 0x3
#define BITS_0100 0x4
#define BITS_0101 0x5
#define BITS_0110 0x6
#define BITS_0111 0x7
#define BITS_1000 0x8
#define BITS_1001 0x9
#define BITS_1010 0xa
#define BITS_1011 0xb
#define BITS_1100 0xc
#define BITS_1101 0xd
#define BITS_1110 0xe
#define BITS_1111 0xf

#define BITS_4(a)   BITS_##a
#define BITS_8(a,b)               ((BITS_##a <<  4) | BITS_##b)
#define BITS_16(a,b,c,d)          ((BITS_##a << 12) | (BITS_##b <<  8) | (BITS_##c << 4) | BITS_##d)
#define BITS_32(a,b,c,d, e,f,g,h) ((BITS_##a << 28) | (BITS_##b << 24) | (BITS_##c << 20) | (BITS_##d << 16) |\
                                   (BITS_##e << 12) | (BITS_##f <<  8) | (BITS_##g <<  4) | (BITS_##h))
#define BITS_64(a,b,c,d, e,f,g,h, i,j,k,l, m,n,o,p) (((long long)BITS_32(a,b,c,d, e,f,g,h) << 32) | BITS_32(i,j,k,l, m,n,o,p))

#if 0  // USAGE
enum { My_Bits = BITS_8(0010, 1100) };
#endif // USAGE
#endif // BINARY


// USEFUL CONSTANTS FOR QUASI-RANDOM LAYOUT
static const double Golden_Ratio     = 1.61803398874989484820458683436563;
static const double Phi1             = 1.61803398874989484820458683436563;
static const double Plastic_Constant = 1.32471795724474602596090885447809;
static const double Phi2             = 1.32471795724474602596090885447809;
static const double Phi3             = 1.220744084605759475361685349108831;

#if 0  // USAGE
// for use with `n` points in `d` dimensions:

g = Phi_d

a1 = 1.0/g
a2 = 1.0/(g*g)
a3 = 1.0/(g*g*g)
...

unused;
x[n] = modf(0.5+a1*n, &unused) // fractional part, i.e. between 0 & 1
y[n] = modf(0.5+a2*n, &unused)
z[n] = modf(0.5+a3*n, &unused)
...
#endif // USAGE

#define CEE_H
#endif//CEE_H
