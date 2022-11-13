/* License at end of file */
/* TODO:
 * - add compile-time option for bounds-checking with custom behaviour on fail
 * - refcounting
 * - thread-safe/atomic versions
 * - alignment for SIMD
 * - minimize repeated arguments where possible
 *     - move to functions?
 * - consider adding a temp/register at a[-1]
 * - add flags to top capacity bits
 */

/*
| ---- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| Test | Name                      | Description                                                                                                              |
| ---- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
|      |                           |                                                                                                                          |
| ---- | Adding elements           | ------------------------------------------------------------------------------------------------------------------------ |
| ---- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| PASS | push(a,v)                 | append value to the end of the array, growing (or creating) the array if needed                                          |
| PASS | add(a,n)                  | add space for `n` more items on the array, growing (or creating) if needed. The items are considered used (in len)       |
| PASS | insert(a, i, v)           | inserts value into the array at existing index i, moving elements from position i upwards along 1 to make space          |
|      |                           |                                                                                                                          |
| TODO | changecap(a,n)            | change space to allow for `n` items on the array, growing (or creating) if needed                                        |
| TODO | expand(a,n)               | add space to allow for `n` items on the array, growing (or creating) if needed                                           |
|      |                           |                                                                                                                          |
| PASS | pusharr(a,b)              | appends the values from b to the end of a                                                                                |
| PASS | pushptr(a,ptr,n)          | appends n elements from ptr to the end of a                                                                              |
| TODO | pushstr(a,str)            | appends chars from str until a '\0' character is hit
| TODO | pushstrlit(a,str)         | pushes a string literal
| TODO | pushstrn(a,str,n)         | appends n elements from ptr to the end of a                                                                              |
| PASS | pusharray(a,arr)          | append array to the end of a for fixed-size arrays (where the number of elements can be determined at compile-time)      |
|      |                           |                                                                                                                          |
| TODO | insertarr(a, i, b)        | inserts value into the array at existing index i, moving elements from position i upwards along 1 to make space          |
| TODO | insertptr(a, i, v, n)     | inserts value into the array at existing index i, moving elements from position i upwards along 1 to make space          |
| TODO | insertarray(a, i, arr)    | inserts value into the array at existing index i, moving elements from position i upwards along 1 to make space          |
|      |                           |                                                                                                                          |
| ---- | Removing elements         | ------------------------------------------------------------------------------------------------------------------------ |
| ---- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| NONE | remove(a, i, n)           | removes the element at index i, shifting the higher elements down into its place TODO: this should have n                |
| NONE | pull(a,i)                 | remove 1 and return the value. This fails if wrapped in parens: `x = (pull(vals, i))`                                    |
| PASS | pop(a)                    | removes and returns the last element of the array                                                                        |
| NONE | shift(a)                  | removes and returns the first element of the array, shifting the other elements down into its place                      |
|      |                           |                                                                                                                          |
| PASS | clear(a)                  | set the length of array to 0                                                                                             |
|      |                           |                                                                                                                          |
| PASS | removeswap(a,i)           | a faster way to remove elements. Moves last element into index i. Does not preserve ordering.                            |
|      |                           |                                                                                                                          |
| PASS | resetlen(a,n)             | set length to a given value                                                                                              |
|      |                           |                                                                                                                          |
| PASS | free(a)                   | frees the array contents; also sets the array ptr back to NULL, causing access to fail. The array can be pushed to again |
| NONE | free2d(a)                 | frees 2d/jagged arrays - all of the subarrays are freed, then the containing array                                       |
| NONE | free2dt(ht2, a)           | as above, but for when the inner arrays have a different header type                                                     |
|      |                           |                                                                                                                          |
|      |                           |                                                                                                                          |
|      | / Accessing elements /    |                                                                                                                          |
| ---- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| PASS | bc(a,i)                   | checks that i is within the bounds of a                                                                                  |
| NONE | get(a,i)                  | expands to an l-value of the ith  element in the array, or asserts if i is out of bounds                                 |
| PASS | last(a)                   | expands to an l-value of the last element in the array                                                                   |
|      |                           |                                                                                                                          |
|      |                           |                                                                                                                          |
|      | Array header/metadata     |                                                                                                                          |
| ---- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| PASS | hdr(a)                    | 'returns' a pointer to the header, which you can then use like a normal struct                                           |
| PASS | len(a)                    | number of elements in the array. Safe to pass `NULL`.                                                                    |
| PASS | cap(a)                    | number of possible elements in the array before it needs to grow. Safe to pass `NULL`.                                   |
| ???? | count(a)                  | (alias for `ahd_len` (kept for compatibility with `stb_stretchybuffer.h`                                                 |
| PASS | _len(a)                   | number of elements in the array. Not safe to pass `NULL`.                                                                |
| PASS | _cap(a)                   | number of possible elements in the array before it needs to grow. Not safe to pass `NULL`.                               |
|      |                           |                                                                                                                          |
| PASS | size(a)                   | number of bytes in the array (excluding the header) for `len` elements                                                   |
| PASS | totalsize(a)              | number of bytes in the array (including the header)                                                                      |
| NONE | capsize(a)                | number of bytes in the array (excluding the header) for `cap` elements                                                   |
| NONE | totalcapsize(a)           | number of bytes in the array (including the header)                                                                      |
|      |                           |                                                                                                                          |
|      |                           |                                                                                                                          |
|      | Extracting array subsets  | Creates new array to be freed at some point                                                                              |
| ---- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| PASS | dup(a)                    | duplicate the array                                                                                                      |
| PASS | sub(a,f,n)                | duplicate subset of the array                                                                                            |
| TODO | slice()                   | window into array (elements will update along with original)                                                             |
|      |                           |                                                                                                                          |
| TODO | unique()                  |                                                                                                                          |
| TODO | uniqueadj()               | linear time simplification of above - only excludes adjacent elements: {a,b,b,c,b} -> {a,b,c,b}                          |
|      |                           |                                                                                                                          |
| TODO | union(a,b)                |                                                                                                                          |
| TODO | intersection(a,b)         |                                                                                                                          |
| TODO | difference(a,b)           |                                                                                                                          |
| TODO | symdiff(a,b)              |                                                                                                                          |
|      |                           |                                                                                                                          |
| TODO | shuffle(a)                |                                                                                                                          |
| TODO | sample(a,n)               |                                                                                                                          |
|      |                           |                                                                                                                          |
|      | / Rearranging array /     | Modify the array itself (nothing new created on the heap)                                                                |
| ---- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| PASS | sortu(a,mem,dir)          | Sort a in dir direction based on an unsigned integer member of each element (mem is the address of that)                 |
| PASS | sorts(a,mem,dir)          | Sort a in dir direction based on a signed integer member of each element (mem is the address of that)                    |
| PASS | sorti(a,mem,dir)          | Sort a in dir direction based on any integer member of each element (mem is the address of that)                         |
| PASS | sortf(a,mem,dir)          | Sort a in dir direction based on a floating point member of each element (mem is the address of that)                    |
| TODO | sortstr(a,mem,dir)        | Sort a in dir direction based on a string pointer member of each element (mem is the address of that)                    |
| TODO | sortchr(a,t,dir)          | Sort a in dir direction based on a character array member of each element (mem is the address of that)                   |
| PASS | reverse(a)                | reverse the order of the elements in the array                                                                           |
| PASS | rotr(a,n)                 | move each element to the next index. The last element moves to the first position                                        |
| TODO | rotl(a,n)                 | move each element to the previous index. The first element moves to the last position                                    |
|      |                           |                                                                                                                          |
|      |                           |                                                                                                                          |
|      | / Array processing /      |                                                                                                                          |
| ---- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| NONE | mapt(i,ta,a,va,tb,b,vb)   |                                                                                                                          |
| NONE | map(i,t,a,b,v)            |                                                                                                                          |
| NONE | mapfn(fn,a,b,udata)       |                                                                                                                          |
|      |                           |                                                                                                                          |
| NONE | reduce(a,ex)              |                                                                                                                          |
| NONE | reducefn(fn,a,acc,udata)  |                                                                                                                          |
|      |                           |                                                                                                                          |
| NONE | filter(i,t,v,a,b,tr)      |                                                                                                                          |
| NONE | reject(i,t,v,a,b,tr)      |                                                                                                                          |
| NONE | count(i,t,v,a,b,tr)       |                                                                                                                          |
| NONE | uncount(i,t,v,a,b,tr)     |                                                                                                                          |
| NONE |                           |                                                                                                                          |
| NONE | find(a,i,t,v,fnd,tr)      |                                                                                                                          |
| NONE | findi(a,i,t,v,tr)         |                                                                                                                          |
| NONE | findv(a,t,v,fnd,tr)       |                                                                                                                          |
|      |                           |                                                                                                                          |
|      | / Scoped memory /         |                                                                                                                          |
| ---- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| PASS | exitscope                 |                                                                                                                          |
| PASS | scoped(t,a)               |                                                                                                                          |
| PASS | scoped_init(t,a,init)     |                                                                                                                          |
| NONE | scoped2d(t,a)             |                                                                                                                          |
| NONE | scoped2d_init(t,a,init)   |                                                                                                                          |
*************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AHD_INCLUDED

#if ! (defined(AHD_REALLOC) && defined(AHD_FREE))
#include <stdlib.h>
#define AHD_REALLOC(ptr, size) realloc(ptr, size)
#define AHD_FREE(ptr) free(ptr)
#endif/*stdlib*/

#if ! (defined(AHD_MEMCPY) && defined(AHD_MEMMOVE))
#include <string.h>
#define AHD_MEMCPY memcpy
#define AHD_MEMCMP memcmp
#define AHD_MEMMOVE memmove
#endif/*stdlib*/

#ifndef  AHD_VSNPRINTF
# define AHD_VSNPRINTF(...) vsnprintf(__VA_ARGS__)
#endif

#ifndef ahd_int
typedef unsigned long long ahd_int;
#endif

#ifndef ahd_enum
typedef int ahd_enum;
#endif

//  array ptr
//       v
// |-hdr-|------------------------|
typedef struct ahd_arr {
	ahd_int cap;
	ahd_int len;
} ahd_arr;

// TODO:
typedef struct ahd_rc {
	ahd_int rc;
	void (*free)(void*);
} ahd_rc;

// for use with enums
typedef struct ahd_kind {
	ahd_enum kind;
} ahd_kind;

typedef struct ahd_ts {
	ahd_int lock;
	// TODO: anything else? signals?
} ahd_ts; // or mtx?

#define ahd_t(t) ahd_## t t

/* Usage: creating a custom header
 * typedef struct typesafe_refcounted_array { ahd_t(arr); ahd_t(rc); int my_custom_int; ahd_t(ts); } typesafe_refcounted_array;
 * int *Nums = 0;
 * ahd_push(typesafe_refcounted_array, Nums, 3);
 * #define tsrc_push(a,v) ahd_push(typesafe_refcounted_array, a, v)
 * tsrc_push(Nums, 62);
 */

#ifndef AHD_NO_DEFAULT_ARR /***************************************************/
/* Adding elements */
#define arr_push(a,v)               ahd_push(ahd_arr,a,v)
#define arr_add(a,n)                ahd_add(ahd_arr,a,n)
#define arr_insert(a,i,v)           ahd_insert(ahd_arr,a,i,v)

#define arr_append(a,src,len,size)  ahd_append(ahd_arr,a,src,len,size)
#define arr_concat(a,b)             ahd_concat(ahd_arr,a,b)
#define arr_pusharr(a,arr,n)        ahd_pusharr(ahd_arr,a,arr,n)
#define arr_pushptr(a,arr,n)        ahd_pushptr(ahd_arr,a,arr,n)
#define arr_pushstr(a,str)          ahd_pushstr(ahd_arr,a,str)
#define arr_pushstrlit(a,str)       ahd_pushstrlit(ahd_arr,a,str)
#define arr_pushstrn(a,str,n)       ahd_pushstr(ahd_arr,a,str,n)
#define arr_pusharray(a,arr)        ahd_pusharray(ahd_arr,a,arr)


/* Removing elements */
#define arr_pop(a)                  ahd_pop(ahd_arr,a)
#define arr_shift(a)                ahd_shift(ahd_arr,a)
#define arr_remove(a,i,n)           ahd_remove(ahd_arr,a,i,n)
#define arr_pull(a,i)               ahd_pull(ahd_arr,a,i)
#define arr_removeswap(a,i)         ahd_removeswap(ahd_arr,a,i)

#define arr_clear(a)                ahd_clear(ahd_arr,a)
#define arr_resetlen(a,n)           ahd_resetlen(ahd_arr,a,n)

#define arr_free(a)                 ahd_free(ahd_arr,a)
#define arr_free2dt(ht2,a)          ahd_free2dt(ahd_arr,ht2,a)
#define arr_free2d(a)               ahd_free2d(ahd_arr,a)


/* Accessing elements */
#define arr_bc(a,i)                 ahd_bc(ahd_arr,a,i)
#define arr_get(a,i)                ahd_get(ahd_arr,a,i)
#define arr_el(a,i)                 ahd_el(ahd_arr,a,i)
#define arr_last(a)                 ahd_last(ahd_arr,a)


/* Array header/metadata */
#define arr_hdr(a)                  ahd_hdr(ahd_arr,a)

#define arr_len(a)                  ahd_len(ahd_arr,a)
#define arr_cap(a)                  ahd_cap(ahd_arr,a)
#define arr_size(a)                 ahd_size(ahd_arr,a)
#define arr_totalsize(a)            ahd_totalsize(ahd_arr,a)
#define arr_capsize(a)              ahd_capsize(ahd_arr,a)
#define arr_totalcapsize(a)         ahd_totalcapsize(ahd_arr,a)

#define arr__len(a)                 ahd__len(ahd_arr,a)
#define arr__cap(a)                 ahd__cap(ahd_arr,a)
#define arr__size(a)                ahd__size(ahd_arr,a)
#define arr__totalsize(a)           ahd__totalsize(ahd_arr,a)
#define arr__capsize(a)             ahd__capsize(ahd_arr,a)
#define arr__totalcapsize(a)        ahd__totalcapsize(ahd_arr,a)


/* Extracting array subsets */
#define arr_dup(a)                  ahd_dup(ahd_arr,a)
#define arr_sub(a,f,n)              ahd_sub(ahd_arr,a,f,n)
#define arr_slice()                 ahd_slice()

#define arr_unique(a)               ahd_unique(ahd_arr,a)

#define arr_union(a,b)              ahd_union(ahd_arr,a,b)
#define arr_intersection(a,b)       ahd_intersection(ahd_arr,a,b)


/* Rearranging array */
#define arr_sorti(a,mem,dir)        ahd_sorti(ahd_arr,a,mem,dir)
#define arr_sortu(a,mem,dir)        ahd_sortu(ahd_arr,a,mem,dir)
#define arr_sortint(a,mem,dir)      ahd_sortint(ahd_arr,a,mem,dir)
#define arr_sortf(a,mem,dir)        ahd_sortf(ahd_arr,a,mem,dir)
#define arr_reverse(a)              ahd_reverse(ahd_arr,a)
#define arr_rotr(a, n)              ahd_rotr(ahd_arr,a,n)
#define arr_rotl(a, n)              ahd_rotl(ahd_arr,a,n)

#define arr_eq(a,b)              ahd_eq(ahd_arr,a,b)

/* Array processing */
#if 1
#define arr_each(a,i)                  ahd_each(ahd_arr,a,i)
#define arr_each_r(a,i)                ahd_each_r(ahd_arr,a,i)
#define arr_each_v(a,i,t,v)            ahd_each_v(ahd_arr,a,i,t,v)
#define arr_each_rv(a,i,t,v)           ahd_each_rv(ahd_arr,a,i,t,v)
#else // 0
#define arr_foreach(a,i)                  for ahd_each(ahd_arr,a,i)
#define arr_foreach_r(a,i)                for ahd_each_r(ahd_arr,a,i)
#define arr_foreach_v(a,i,t,v)            for ahd_each_v(ahd_arr,a,i,t,v)
#define arr_foreach_rv(a,i,t,v)           for ahd_each_rv(ahd_arr,a,i,t,v)
#endif // 0

#define arr_mapt(i,ta,a,va,tb,b,vb)    ahd_mapt(ahd_arr,i,ta,a,va,tb,b,vb)
#define arr_mapv(i,t,a,b,v)            ahd_mapv(ahd_arr,i,t,a,b,v)
#define arr_mapfn(fn,a,b,udata)        ahd_mapfn(ahd_arr,fn,a,b,udata)

#define arr_reduce(a,ex)               ahd_reduce(ahd_arr,a,ex)
#define arr_reducefn(fn,a,acc,udata)   ahd_reducefn(ahd_arr,fn,a,acc,udata)

#define arr_filter(i,t,v,a,b,tr)       ahd_filter(ahd_arr,i,t,v,a,b,tr)
#define arr_count(i,t,v,a,b,tr)        ahd_count(ahd_arr,i,t,v,a,b,tr)
#define arr_countx(a,i,x,tr)           ahd_countx(ahd_arr,a,i,x,tr)

#define arr_find(a,i,t,v,fnd,tr)       ahd_find(ahd_arr,a,i,t,v,fnd,tr)
#define arr_findi(a,i,t,v,tr)          ahd_findi(ahd_arr,a,i,t,v,tr)
#define arr_findv(a,t,v,fnd,tr)        ahd_findv(ahd_arr,a,t,v,fnd,tr)

/* Scoped memory */
#define arr_exitscope                  ahd_exitscope
#define arr_scoped(t,a)                ahd_scoped(ahd_arr,t,a)
#define arr_scoped2(t,a,b)             ahd_scoped2(ahd_arr,t,a,b)
#define arr_scoped_init(t,a,init)      ahd_scoped_init(ahd_arr,t,a,init)
#define arr_scoped2d(t,a)              ahd_scoped2d(ahd_arr,t,a)
#define arr_scoped2d_init(t,a,init)    ahd_scoped2d_init(ahd_arr,t,a,init)

#endif/*AHD_NO_DEFAULT_ARR*****************************************************/





/******************************************************************************/
/* Helper macros **************************************************************/
/******************************************************************************/
#define AHD_LN(x)  AHD_CAT(x, __LINE__)

#define AHD_CAT(a,b)  AHD_CAT2(a,b)
#define AHD_CAT2(a,b) AHD_CAT1(a,b)
#define AHD_CAT1(a,b) a##b

#define ahd_n_ln AHD_LN(ahd_n_)
#define ahd_i_ln AHD_LN(ahd_i_)

// TODO: used?
#define ahd_ptr2i(a,p)        (((unsigned char *)(p) - (unsigned char *)(a)) / sizeof(*(a)))

#define ahd_if(a,v)           ((a) ? (v) : 0)

#define ahd__data(ht,a) (a), sizeof(ht), sizeof(*(a))

/******************************************************************************/
/* Adding elements ************************************************************/
/******************************************************************************/
#define ahd_push(ht,a,v)      (ahd_maybegrow(ht,a,1), (a)[ahd__len(ht,a)++] = (v), ahd__len(ht,a)-1)
#define ahd_add(ht,a,n)       ((ahd_maybegrow(ht,a,n), ahd__len(ht,a)+=(n)) - (n))
#define ahd_concat(ht,a,b)        ahd__pushsize(ht, a, b,   ahd_len(ht, b),             ahd_size(ht,b))
#define ahd_pusharr(ht,a,arr,els) ahd__pushsize(ht, a, arr, els,                        (els)*sizeof(*(a)) )
#define ahd_pushptr(ht,a,ptr,els) ahd__pushsize(ht, a, ptr, els,                        (els)*sizeof(*(a)) )
#define ahd_pushstrlit(ht,a,str)  ahd__pushsize(ht, a, str, sizeof(str),                sizeof(str) )
#define ahd_pushstr(ht,a,str)     (*(char **)&(a) = ahd__pushstr(&(a), sizeof(ht), sizeof(*(a)), str, ~0))
#define ahd_pushstrn(ht,a,str,n)  (*(char **)&(a) = ahd__pushstr(&(a), sizeof(ht), sizeof(*(a)), str,  n))
#define ahd_pusharray(ht,a,arr)   ahd__pushsize(ht, a, arr, sizeof(arr)/sizeof(*(arr)), sizeof(arr))

#define ahd__pushsize(ht,a,src,len,size) (AHD_MEMMOVE((a)+ahd_add(ht,a,len), (src), (size)), ahd__len(ht,a)-(len))

#define ahd_insert(ht,a,i,v)  (ahd_add(ht,a,1),\
	                           AHD_MEMMOVE((a)+(i)+1, (a)+(i), (ahd__len(ht,a)-(i)-1) * sizeof(*(a)) ),\
	                           (a)[(i)] = (v), (i) )

/*TODO: #define ahd_push_ts(ht,a,v)   do{ \
	(ahd_maybegrow(ht,a,1), (a)[ahd__len(ht,a)++] = (v), ahd__len(ht,a)-1)} while(0)*/

#define ahd_needgrow(ht,a,n)  ((a)==0 || ahd__len(ht,a)+(n) > ahd__cap(ht,a))
#define ahd_maybegrow(ht,a,n) ahd_if(ahd_needgrow(ht,a,(n)), ahd_grow(ht,a,n))
#define ahd_grow(ht,a,n)      (*((void **)&(a)) = ahd__grow(ahd_if(a, ahd_hdr(ht,a)), (n), \
			                  sizeof(*(a)), sizeof(ht)))

#if AHD_DEBUG // control whether callsite is recorded
# define AHD_DBG(fn, ...) fn##_dbg(__VA_ARGS__, int line, char const *file, char const *func, char const *call)

# define ahd__grow(...) ahd__grow_dbg(__VA_ARGS__, __LINE__, __FILE__, __func__, "ahd__grow("#__VA_ARGS__")")

# define AHD_DBG_UNUSED (void)line, (void)file, (void)func, (void)call
#else //AHD_DEBUG
# define AHD_DBG(fn, ...) fn(__VA_ARGS__)

# define ahd__grow(...) ahd__grow(__VA_ARGS__)

# define AHD_DBG_UNUSED
#endif//AHD_DEBUG

// TODO: should this be arr ptr, rather than base?
static void *
AHD_DBG(ahd__grow, void *ptr, ahd_int inc, ahd_int itemsize, ahd_int headersize)//, int cap, int len)
// TODO: static int ahd__grow(void **ptr, ahd_int inc, ahd_int itemsize, ahd_int headersize)//, int cap, int len)
{
    AHD_DBG_UNUSED;
	ahd_arr *head      = (ahd_arr *)ptr;
	ahd_int dbl_cap    = ahd_if(ptr, head->cap  * 2);
	ahd_int min_needed = ahd_if(ptr, head->len) + inc;
	min_needed         = (min_needed > 64) ? min_needed : 64;
	ahd_int new_cap    = (dbl_cap > min_needed) ? dbl_cap   : min_needed; // could round to next highest pow2?
	head               = (ahd_arr *) AHD_REALLOC(ptr, itemsize * new_cap + headersize);
	if (head) {
		if (!ptr) {
			char *hdr_bytes = ((char *)&head->len) - 1,
				 *hdr_guard = ((char *)head) + headersize;
			while(++hdr_bytes < hdr_guard)
			{ *hdr_bytes = 0; } /* zero init for any size header */
		}
		head->cap = new_cap;
		return (char *)head + headersize;
	}
	else {
#ifdef AHD_BUFFER_OUT_OF_MEMORY
		AHD_BUFFER_OUT_OF_MEMORY ;
#endif
		return (char*)(uintptr_t)headersize; // try to force a NULL pointer exception later
	}
}

#if 1
static ahd_int
ahd__pushstr(char **arr, ahd_int hdr_size, ahd_int el_size, char *str, ahd_int n)
{
	ahd_arr *head = (ahd_arr *)(*arr - hdr_size);
	ahd_int len = head->len;
	ahd_int strLen = 0;
	char *str_ = str;
	while(*(str_++) && (!~n || n--)) {++strLen;}

	if(head->len + strLen + 1 >= head->cap)
	{ *arr = (char *)ahd__grow(*arr-hdr_size, strLen + 1, el_size, hdr_size); }

	AHD_MEMMOVE(*arr + len, str, strLen);

	*arr[len+strLen] = '\0';

	return len;
}
#endif


/******************************************************************************/
/* Removing elements **********************************************************/
/******************************************************************************/
#define ahd_pop(ht,a)         ((a)[--ahd__len(ht,a)])
#define ahd_shift(ht,a)       ahd_pull(ht,a,0)
#define ahd_clear(ht,a)       ahd_if(a, ahd__len(ht,a) = 0)
#define ahd_resetlen(ht,a,n)  (ahd_clear(ht,a), ahd_add(ht,a,n))
#define ahd_remove(ht,a,i,n)   AHD_MEMMOVE((a)+(i), (a)+(i)+(n), ((ahd__len(ht,a)-=(n))-(i)) * sizeof(*(a)) )
// NOTE: (similar to delix)
#define ahd_removeswap(ht,a,i) ((a)[i] = (a)[--ahd__len(ht,a)])
// Remove and return value
// this should work as expected as long as you don't wrap in parens: x = (ahd_pull(ahd_arr, arr, 3));
#define ahd_pull(ht,a,i)       (a)[i], ahd_remove(ht,a,i,1)



/******************************************************************************/
/* Array metadata *************************************************************/
/******************************************************************************/
#define ahd_len(ht,a)           ahd_if(a, ahd__len(ht,a))
#define ahd_cap(ht,a)           ahd_if(a, ahd__cap(ht,a))
#define ahd_size(ht,a)          ahd_if(a, ahd__size(ht,a))
#define ahd_capsize(ht,a)       ahd_if(a, ahd__capsize(ht,a))
#define ahd_totalsize(ht,a)     ahd_if(a, ahd__totalsize(ht,a))
#define ahd_totalcapsize(ht,a)  ahd_if(a, ahd__totalcapsize(ht,a))

#define ahd_last(ht,a)        ((a)[ahd__len(ht,a)-1])

#define ahd_hdr(ht, a)        ((ht *)(a) - 1)

// these don't check if array is non-NULL
#define ahd__len(ht,a)          (ahd_hdr(ht,a)->len)
#define ahd__cap(ht,a)          (ahd_hdr(ht,a)->cap)
#define ahd__size(ht,a)         (ahd__len(ht,a) * sizeof(*(a)) )
#define ahd__capsize(ht,a)      (ahd__cap(ht,a) * sizeof(*(a)) )
#define ahd__totalsize(ht,a)    (ahd__len(ht,a) * sizeof(*(a)) + sizeof(ht) )
#define ahd__totalcapsize(ht,a) (ahd__cap(ht,a) * sizeof(*(a)) + sizeof(ht) )

// sets the array back to NULL, so any attempts to access contents fail, or the array can be pushed to again
#define ahd_free(ht,a)        (ahd_if(a, (AHD_FREE(ahd_hdr(ht,a)),0)), (a) = 0)
#define ahd_free2dt(ht,ht2,a)    do { \
		for(ahd_int ahd_i_ln = 0; ahd_i_ln < ahd_len(ht,a); ++ahd_i_ln) \
		{ ahd_free(ht2,(a)[ahd_i_ln]); } \
        ahd_free(ht,a); \
	} while(0)
#define ahd_free2d(ht,a)     ahd_free2dt(ht,ht,a)
// 3D?

static void ahd__free2d(void *outer, ahd_int el_size_outer, ahd_int hdr_size_outer, ahd_int hdr_size_inner)
#ifdef AHD_IMPLEMENTATION
{
	char *p = (char *)outer;
	if(p) {
		ahd_arr *head = (ahd_arr *)(p - hdr_size_outer);
		ahd_int len = head->len;
		ahd_int i = 0;
		for(; i < len; ++i, p += el_size_outer) {
			char *p2 = *(char **)p;
			if(p2)
			{ AHD_FREE(p2 - hdr_size_inner); }
		}
		AHD_FREE(head);
	}
}
#else
;
#endif// AHD_IMPLEMENTATION


#define ahd_union(ht,a,b)
#define ahd_intersection(ht,a,b)

#define ahd_reverse(ht,a) ahd__reverse(ahd__data(ht,a))

static void ahd__memswap(void *el_a, void *el_b, ahd_int size) {
	char *a = (char *)el_a,
		 *b = (char *)el_b;
	for(;size--; ++a, ++b) {
		char t = *a;
		*a = *b;
		*b = t;
	}
}

static void ahd__reverse(void *arr, ahd_int hdr_size, ahd_int el_size)
{
	char *a = (char *)arr;
	ahd_int len = ((ahd_arr *)(a - hdr_size))->len;
	char *at  = a,
		 *mid = a + (len / 2) * el_size,
		 *ta  = a + (len - 1) * el_size;

	for(; at < mid; at += el_size, ta -= el_size)
	{ ahd__memswap(at, ta, el_size); }
}


/******************************************************************************/
/* Extracting array subsets****************************************************/
/******************************************************************************/
#define ahd_dup(ht,a)       ahd__dup(ahd__data(ht,a))

#define ahd_sub(ht,a,f,n)    ahd__sub(ahd__data(ht,a), f, n)
// TODO: refactor with similar fns
static void *
AHD_DBG(ahd__dup, void *arr, ahd_int hdr_size, ahd_int el_size) {
    AHD_DBG_UNUSED;
	ahd_arr *head          = (ahd_arr *)((char *)arr - hdr_size);
	ahd_int total_cap_size = hdr_size + head->cap * el_size;
	ahd_arr *new_head      = (ahd_arr *) AHD_REALLOC(0, total_cap_size);
	if (head)
	{ return (char *)AHD_MEMMOVE(new_head, head, total_cap_size) + hdr_size; }
	else {
#ifdef AHD_BUFFER_OUT_OF_MEMORY
		AHD_BUFFER_OUT_OF_MEMORY ;
#endif
		return (char*)(uintptr_t)hdr_size; // try to force a NULL pointer exception later
	}

}

static void *ahd__sub(void *arr, ahd_int hdr_size, ahd_int el_size, ahd_int first, ahd_int n) {
	char *new_arr = (char *)ahd__grow(0, n, el_size, hdr_size);
	ahd_arr *head = (ahd_arr *)(new_arr - hdr_size);
	head->len = n;
	AHD_MEMMOVE(new_arr, ((char *)arr + first*el_size), n*el_size);
	return new_arr;
}

#define ahd_slice()
// TODO: don't repeat i
#define ahd_unique(ht,a)




#ifndef AHD_ASSERT
#define AHD_ASSERT(x) ((x) ? 1 : *(volatile int *)0 == 0)
#endif/*AHD_ASSERT*/

#ifndef  AHD_BOUNDS_CHECK
# define AHD_BOUNDS_CHECK 1
#endif// AHD_BOUNDS_CHECK

#if AHD_BOUNDS_CHECK
// NOTE: doing the unsafe len here as the purpose is to fail loudly on error
// if a is NULL, then it's an illegal access anyway
static inline ahd_int ahd__bc(ahd_int len, ahd_int i) { AHD_ASSERT(i < len && "bounds check failed"); return i; }
#else
# define ahd__bc(n,i) (i)
#endif/*AHD_BOUNDS_ASSERT*/

#define ahd_bc(ht,a,i) ahd__bc(ahd__len(ht,a), i)

#define ahd_get(ht,a,i) ((a)[ahd__bc(ahd__len(ht,a), i)])

/* // negative numbers allow access from end */
/* static inline ahd_int */
/* ahd__get(ahd_int len, signed long long i) { */
/* 	ahd_int index = (ahd_int)((i >= 0) ? i */
/* 	                                   : len + i); */
/* 	AHD_BOUNDS_ASSERT(ahd__bc(len, index)); */
/* 	return index; */
/* } */

/******************************************************************************/
/* Array element rearranging **************************************************/
/******************************************************************************/
#define ahd_is_signed(v) (~((v)^(v))<0)

#define ahd__sortx(x,ht,a,mem,dir) \
	ahd__sort##x(ahd__data(ht,a), mem, (ahd_int)sizeof(*(mem)), dir)

#define ahd_sorti(ht,a,mem,dir)   ahd__sortx(i,ht,a,mem,dir)
#define ahd_sortu(ht,a,mem,dir)   ahd__sortx(u,ht,a,mem,dir)
#define ahd_sortint(ht,a,mem,dir) ahd__sortx(int[ahd_is_signed(*(mem))],ht,a,mem,dir)
#define ahd_sortf(ht,a,mem,dir)   ahd__sortx(f,ht,a,mem,dir)

#define AHD_STACK_BUF_SIZE 256

#define ahd__max(a,b) ((a) >= (b) ? (a) : (b))
#define ahd__min(a,b) ((a) <  (b) ? (a) : (b))

#define ahd_rotr(ht,a,n) ahd__memrotr(a, sizeof(*(a)), ahd_len(ht,a), n)

/* TODO: add temp var ptr? */
static void ahd__memrotr(void *mem, ahd_int el_size, ahd_int len, ahd_int rot_n) {
	/* TODO: how does this compare to just allocating and freeing the memory */
	char buf[AHD_STACK_BUF_SIZE];
	ahd_int size           = el_size * rot_n,
	        n_bytes        = (len - rot_n)*el_size,
	        max_size       = ahd__min(size, sizeof(buf)),
			size_remaining = size,
			size_copied    = 0,
			cpy_size       = ahd__min(size_remaining, max_size);

	for(; size_remaining;
		size_remaining -= (cpy_size = ahd__min(size_remaining, max_size)),
		size_copied += cpy_size)
	{
		char *bytes = (char *)mem + size_copied;
		AHD_MEMMOVE(buf, bytes + n_bytes, cpy_size);
		AHD_MEMMOVE(bytes + cpy_size, bytes, n_bytes);
		AHD_MEMMOVE(bytes, buf, cpy_size);
	}
}

typedef enum ahd_sort_dir { ahd_ASC = 1, ahd_DESC = -1 } ahd_sort_dir;
typedef enum ahd_sort_type {
	/* sizeof char      == 1 << 0, */
	/* sizeof short     == 1 << 1, */
	/* sizeof int       == 1 << 2, */
	/* sizeof long long == 1 << 3, */
	ahd_INT  = 1 << 4,
	ahd_FLT  = 1 << 5,
	ahd_STR  = 1 << 6,
	ahd_CHR  = 1 << 7, /* for local string arrays */
	ahd_SIGNSHIFT = 8,
	ahd_SIGN = 1 << ahd_SIGNSHIFT,
} ahd_sort_type;

/* TODO: sort_ _int, _flt, _str, _chr, _fn*/


static int
ahd__sorti(void *array, ahd_int hdr_size, ahd_int el_size, void *member, ahd_int member_size, ahd_sort_dir dir)
{
	int result = 1;
	char *arr = (char *)array;
	ahd_arr *head = (ahd_arr *)(arr - hdr_size);
	ahd_int len = head->len,
			member_offset = (ahd_int)((char *)member - arr);
	char *guard = arr + len * el_size, *at, *a, *b;

	/* insertion sort */
	for(at = arr; at + el_size < guard; at += el_size)
	{
		for(a = at + member_offset; ; a -= el_size)
		{
			int cmp = 0;
			b = a + el_size;
			switch(member_size) {
				case sizeof (char): {
					char A = *(char *)a, B = *(char *)b;
					cmp = A > B ? 1 : A < B ? -1 : 0;
				} break;

				case sizeof (short): {
					short A = *(short *)a, B = *(short *)b;
					cmp = A > B ? 1 : A < B ? -1 : 0;
				} break;

				case sizeof (int): {
					int A = *(int *)a, B = *(int *)b;
					cmp = A > B ? 1 : A < B ? -1 : 0;
				} break;

				case sizeof (long long): {
					long long A = *(long long *)a, B = *(long long *)b;
					cmp = A > B ? 1 : A < B ? -1 : 0;
				} break;

				/* case sizeof (char *):               cmp = strcmp(*(char **)a, *(char **)b);          break; */
				/* case sizeof (char *):               cmp = strncmp((char *)a, (char *)b, str_len);          break; */

				default: result = 0;
			}

			if(cmp * dir > 0) {
				// TODO: speed up with memmove (don't swap all of them)
				ahd__memswap(a-member_offset, b-member_offset, el_size);
				if(a == arr) { break; }
			} else break;
		}
	}

	return result;
}

static int
ahd__sortu(void *array, ahd_int hdr_size, ahd_int el_size, void *member, ahd_int member_size, ahd_sort_dir dir)
{
	int result = 1;
	char *arr = (char *)array;
	ahd_arr *head = (ahd_arr *)(arr - hdr_size);
	ahd_int len = head->len,
			member_offset = (ahd_int)((char *)member - arr);
	char *guard = arr + len * el_size, *at, *a, *b;

	/* insertion sort */
	for(at = arr; at + el_size < guard; at += el_size)
	{
		for(a = at + member_offset; ; a -= el_size)
		{
			int cmp = 0;
			b = a + el_size;

			switch(member_size) {
				case sizeof (unsigned char): {
					unsigned char A = *(char *)a, B = *(char *)b;
					cmp = A > B ? 1 : A < B ? -1 : 0;
				} break;

				case sizeof (unsigned short): {
					unsigned short A = *(short *)a, B = *(short *)b;
					cmp = A > B ? 1 : A < B ? -1 : 0;
				} break;

				case sizeof (unsigned int): {
					unsigned int A = *(int *)a, B = *(int *)b;
					cmp = A > B ? 1 : A < B ? -1 : 0;
				} break;

				case sizeof (unsigned long long): {
					unsigned long long A = *(long long *)a, B = *(long long *)b;
					cmp = A > B ? 1 : A < B ? -1 : 0;
				} break;

				default: result = 0;
			}

			if(cmp * dir > 0) {
				// TODO: speed up with memmove (don't swap all of them)
				ahd__memswap(a-member_offset, b-member_offset, el_size);
				if(a == arr) { break; }
			} else break;
		}
	}

	return result;
}

/* USAGE: ahd__sortint[is_signed(val)](...) */
typedef int (*ahd__sort_int_t)(void *, ahd_int, ahd_int, void *, ahd_int, ahd_sort_dir);
ahd__sort_int_t ahd__sortint[2] = { ahd__sortu, ahd__sorti };

static int
ahd__sortf(void *array, ahd_int hdr_size, ahd_int el_size, void *member,  ahd_int member_size, ahd_sort_dir dir)
{
	int result = 1;
	char *arr = (char *)array;
	ahd_arr *head = (ahd_arr *)(arr - hdr_size);
	ahd_int len = head->len,
			member_offset = (ahd_int)((char *)member - arr);
	char *guard = arr + len * el_size, *at, *a, *b;
	double Dir = (double)dir;

	/* insertion sort */
	for(at = arr; at + el_size < guard; at += el_size) {
		for(a = at + member_offset; ; a -= el_size) {
			double cmp = 0.0;
			b = a + el_size;
			switch(member_size) {
				case sizeof (float):  cmp = (double)(*(float *)a  - *(float  *)b); break;
				case sizeof (double): cmp =          *(double *)a - *(double *)b;  break;
			}

			if(cmp * Dir > 0.0) {
				// TODO: speed up with memmove (don't swap all of them)
				ahd__memswap(a - member_offset, b - member_offset, el_size);
				if(a == arr) { break; }
			} else break;
		}
	}

	return result;
}


#define ahd_eq(ht,a,b) (sizeof(*(a))  == sizeof(*(b))  && \
                        ahd_len(ht,a) == ahd_len(ht,b) && \
                        AHD_MEMCMP((a), (b), ahd_size(ht,a)) == 0)


#define ahd_foronce(cond) ahd_n_ln = 0; cond && ! ahd_n_ln

#ifdef AHD_NO_DECLS
#define ahd_decl(t)
#else
#define ahd_decl(t) t
#endif

// TODO: regularize naming:
//  - _i => just index
//  - no suffix => struct with .v and .i
#define ahd_each(ht,a,i) \
	(ahd_decl(ahd_int) i = 0; i < ahd_len(ht,a); ++i)
#define ahd_each_r(ht,a,i) \
	(ahd_decl(ahd_int) i = ahd_len(ht,a); i-- != 0 ;)

#define ahd_each_v(ht,a,i,t,v) \
	(ahd_decl(ahd_int) i = 0, ahd_foronce(ahd_len(ht,a))++;) \
	for(t v = (a)[i]; i < ahd_len(ht,a); v = (a)[++i])
#define ahd_each_rv(ht,a,i,t,v) \
	(ahd_int i##_ = ahd_len(ht,a), i = i##_-1, ahd_foronce(ahd_len(ht,a))++;) \
	for(t v = (a)[i]; i##_ != 0; i = --i##_-1, v = (a)[ahd_if(i##_, i)])

// NOTE: may break if you push to it during iteration
#define ahd_all(t, v, a) \
    (t v = (a); v < (a) + ahd_len(a); ++v)


// statement
// should this create new array? map to 2nd array? change in place?
#define ahd_mapt(ht,i,ta,a,va,tb,b,vb) if(a) \
			for(ahd_decl(ahd_int) i = ((a) == (b) || ahd_resetlen(ht, b, ahd__len(ht,a)), 0), ahd_foronce(1);) \
			for(t vb = {0}; ! ahd_n_ln++; ) \
			for(t va = (a)[i]; i < ahd__len(ht,a); (b)[i] = vb, ++i, va = (a)[i])
// for when the type remains the same:
#define ahd_mapv(ht,a,b,i,t,v) if(a) \
			for(ahd_decl(ahd_int) i = (a) == (b) || (ahd_resetlen(ht, b, ahd__len(ht,a)), 0), ahd_foronce(1)++;) \
			for(t v = {0}; i < ahd__len(ht,a); (b)[i] = v, memset(&v,0,sizeof(v)), ++i)
// TODO ahd_map(ht,a,b)
/* Usage:
 * int *lengths = 0;
 * arr_mapt(i_str, char *, strings, str, int, lengths, len)
 * { len = strlen(str); }
 *
 * int *lengths = 0;
 * arr_mapt(i_str, strings, lengths, int, len)
 * { len = strlen(strings[i_str]); }

 * int *lengths = 0;
 * arr_map(i_str, strings, lengths)
 * { lengths[i_str] = strlen(strings[i_str]); }
 */

// expression
#define ahd_mapfn(ht,fn,a,b,udata) (ahd_resetlen(ht, b, ht__len(ht,a)), \
		                            ahd__map(a, sizeof(*(a)), b, sizeof(*(b)), sizeof(ht), ahd_len(a), fn, udata))

typedef struct ahd_any {
	char *base;
	ahd_int hdr_size;
	ahd_int el_size;
} ahd_any;

typedef struct ahd_loop_info {
	ahd_int i;
	ahd_int n;
	void *udata;
	ahd_any a, b;
} ahd_loop_info;
typedef int ahd_loopfn(ahd_loop_info *info, void *a, void *b);

static void ahd__map(void *arr_a, ahd_int el_size_a, void *arr_b, ahd_int el_size_b,
                     ahd_int hdr_size, ahd_int len, ahd_loopfn fn, void *udata)
{
	char *a = (char *)arr_a, *b = (char *)arr_b;
	ahd_loop_info info = { 0 };
	info.n = len,     info.udata = udata;
	info.a.base = a,  info.a.hdr_size = hdr_size, info.a.el_size = el_size_a;
	info.b.base = b,  info.b.hdr_size = hdr_size, info.b.el_size = el_size_b;

	for(; info.i < len; ++info.i, a += el_size_a, b += el_size_b)
	{ fn(&info, a, b); }
}
/* Usage:
 * struct str{ char *str; int len; }

 * int map_strwlen(ahd_loop_info info, void *a, void *b, void *udata) {
 *     info; // UNUSED
 *     char **str = a;
 *     struct str *val = b;
 *     struct str new_val = { *str, strlen(*str) };
 *     *val = new_val;
 *     return 0;
 * }
 * ...
 * arr_mapfn(map_strwlen, strings, strs, 0);
 */

#define ahd_reduce(ht,a,ex)
#define ahd_reducefn(ht,fn,a,acc,udata) ahd__reduce(acc, sizeof(acc), a, sizeof(*(a)), sizeof(ht), ahd_len(ht,a), fn, udata)

static void ahd__reduce(void *accum, ahd_int el_size_acc,
		         void *value, ahd_int el_size_val, ahd_int hdr_size,
		         ahd_int len, ahd_loopfn fn, void *udata)
{
	char *acc = (char *)accum,
	     *val = (char *)value;
	ahd_loop_info info = { 0 };
	info.n = len;
	info.udata = udata;
	info.a.base = acc,  info.a.el_size = el_size_acc,  info.a.hdr_size = 0;
	info.b.base = val,  info.b.el_size = el_size_val,  info.b.hdr_size = hdr_size;
	for(; info.i < len; ++info.i, val += el_size_val)
	{ fn(&info, acc, val); }
}
/* Usage:
 * int reduce_sumstrlen(ahd_loop_info info, void *acc, void *val) {
 *     info; // UNUSED
 *     char **str = val;
 *     int *sum_len = acc;
 *     *sum_len += strlen(*str);
 *     return 0;
 * }
 * ...
 * arr_reducefn(reduce_sumstrlen, strings, sum_len, 0);
 */

#define ahd_filterreject(ht,i,t,v,a,b,tr,reset,test) if(a) \
			for(ahd_decl(ahd_int) i = (ahd_clear(ht,b), 0), (reset), ahd_foronce(1)++;) \
			for(ahd_decl(t) v = (a)[i]; \
				i < ahd__len(a); \
				(test) && ahd_push(ht,b,(a)[i]), ++i, (reset))
#define ahd_filter(ht,i,t,v,a,b,tr) ahd_filterreject(ht,i,t,v,a,b,tr,tr=0, tr)
#define ahd_reject(ht,i,t,v,a,b,tr) ahd_filterreject(ht,i,t,v,a,b,tr,tr=1,!tr)
#define ahd_countx(ht,a,i,x,tr) \
	for(ahd_decl(ahd_int) i = (x) = 0; i < arr_len(a); (x) += !!(tr), ++i)

/* Usage:
 * obj_t *filtered_objects = 0;
 * arr_filter(objects, i, obj_t, obj, objects, filtered_objects, found)
 * { found = obj.is_valid; }
 * printf("Original length: %d, filtered length: %d", arr_len(objects), arr_len(filtered_objects));
 *
 * arr_filter(objects, i, objects, filtered_objects, found)
 * { found = objects[i].is_valid; }
 * printf("Original length: %d, filtered length: %d", arr_len(objects), arr_len(filtered_objects));
 */

/* #define ahd_reject(ht,i,t,a,b,tr) if(a) \ */
/* 			for(ahd_decl(ahd_int) tr = 0, i = (ahd_clear(ht,b), 0); \ */
/* 				i < ahd__len(a); \ */
/* 				!tr && ahd_push(ht,b,(a)[i]), ++i, tr = 1) */

#define ahd_filterfn()
static void ahd__filter(void *arr, void *out, ahd_int len, ahd_int el_size, ahd_int hdr_size, ahd_loopfn fn, void *udata) {
	char *a = (char *)arr;
	ahd_loop_info info = { 0 };
	info.n = len,     info.udata = udata;
	info.a.base = a,  info.a.el_size = el_size,  info.a.hdr_size = hdr_size;

	for(; info.i < len; ++info.i, a += el_size)
	{
		if(fn(&info, a, out))
		{
			//TODO
		}
	}
}

#define ahd_split()
#define ahd_join()
// NOTE: should be able to have the index and found bools as internally or externally scoped
// due to the way that comma op works (?)
#define ahd_find(ht,a,i,t,v,fnd,tr) if(a) \
			for(ahd_int AHD_LN(ahd_dummy), i = 0, tr = 0, (fnd) = 0; \
				i < ahd__len(ht,a) && !(fnd); \
				tr ? (ahd_if(fnd, (fnd)=&(a)[i]), i) : ++i)

#define ahd_findi(ht,a,i,t,v,tr)   ahd_find(ht, a, i,        t, v, 0,   tr)
#define ahd_findv(ht,a,t,v,fnd,tr) ahd_find(ht, a, ahd_i_ln, t, v, fnd, tr)

#define ahd_findlast(ht,a,i,t,v,fnd,tr)
/* Usage:
 * f32 *greater_than3;
 * ahd_find(ahd_arr, arr, _, f32, val, greater_than3, found)
 * { found = val > 3.f; }
 * if(greater_than3) { assert(*greater_than3 > 3.f); }
 *
 * int found, found_index;
 * ahd_find(ahd_arr, arr, found_index, f32, val, 0, found)
 * { found = val > 3.f && found_index % 2; }
 * if(found) { assert(arr[found_index] > 3.f); }

 * int found, i_obj;
 * my_findi(objects, i_obj, obj_t, obj, found)
 * { found = obj.is_valid; }
 * if(found) { assert(objects[i_obj].is_valid); }
 */

#define ahd_exitscope continue
// NOTE: adding to NULL ptr is technically UB
#define ahd_scope(init,end)            for(init, *ahd_n_ln = 0; ! ahd_n_ln++; end)
#define ahd_scoped_init(ht,t,a,init)   ahd_scope(t *a = (init), (ahd_free(ht,a),0))
#define ahd_scoped(ht,t,a)             ahd_scope(t *a = 0,      (ahd_free(ht,a),0))
#define ahd_scoped2d_init(ht,t,a,init) ahd_scope(t ** a = (init), (ahd_free2d(ht,a),0))
#define ahd_scoped2d(ht,t,a)           ahd_scope(t ** a = 0,      (ahd_free2d(ht,a),0))
/* Usage:
 * arr_scoped(char *, strings)
 * {
 *     arr_push(strings, "Hello");
 *     arr_push(strings, "World");
 *     printf(strings[0]);
 *     if(strings[1][0] == 'W')
 *     { ahd_exitscope; }
 *
 *     puts(strings[1]);
 * } // no leaks
 *
 * puts(Strings[1]); // fails at compile time - Strings has not been declared
 *
 * // stackable (don't semicolon)
 * ahd_scoped(ahd_arr, char *, Strings)
 * ahd_scoped(ahd_arr, int, Ints)
 *
 */


/* // TODO: make atomic */
/* static ahd_int ahd_rc_inc(ahd_rc *rc) */
/* { return ++rc->rc; } */

/* static ahd_int ahd_rc_dec(ahd_rc *rc, void *raw) { */
/* 	void (*freefn)(void*) = rc->free ? rc->free : AHD_FREE; */
/* 	if(--rc->rc == 0) */
/* 	{ */
/*         (rc->free */
/*          ? rc->free(raw) */
/*          : AHD_FREE(raw)); */
/*     } */
/*     return rc->rc; */
/* } */

#ifdef AHD_IMPLEMENTATION
#include <stdarg.h>
// TODO: variants:
//          - append to current string
//          - append a separate string (separated by '\0')
//          - rewrite
//          - from_offset (covers above 2)
static size_t
arr_vprintf(char *arr[], char const *fmt, va_list args)
{
    AHD_ASSERT(arr);

    size_t len             = arr_len(*arr);
    size_t zero_term       = !!(len > 0 && arr[0][len-1] == '\0');
    size_t cat_start       = len - zero_term;
    size_t chars_available = arr_cap(*arr) - cat_start;

    va_list size_args; va_copy(size_args, args);
    size_t chars_required = AHD_VSNPRINTF(NULL, 0, fmt, size_args) + 1; // zero terminator
    arr_add(*arr, chars_required - zero_term);

    {
        chars_available = arr_cap(*arr) - cat_start;

        /* size_t chars_printed = */
        AHD_VSNPRINTF(*arr + cat_start, (int)chars_available, fmt, args);
        AHD_ASSERT((chars_required - zero_term) <= chars_available && "didn't grow enough?");
    }

    if (*arr)
    {   arr_last(*arr) = '\0';   }

    return chars_required;
}

// TODO: appendf vs replacef vs print_at_offset_f
static int
arr_printf(char *arr[], char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int result = (int)arr_vprintf(arr, fmt, args);
    va_end(args);
    return result;
}

static int
arr_puts_n(const char *str, size_t str_len, char *arr[])
{
    AHD_ASSERT(arr);
    size_t chars_n   = str_len + 1; // zero terminator
    size_t len       = arr_len(*arr);
    int    zero_term = len > 0 && arr[0][len-1] == '\0';

    size_t offset  = arr_add(*arr, chars_n - zero_term) - zero_term;

    AHD_MEMMOVE(*arr + offset, str, str_len);
    arr_last(*arr) = '\0';

    return (int)chars_n;
}

static int
arr_puts(const char *str, char *arr[]) // TODO: restrict ptrs
{   return arr_puts_n(str, strlen(str), arr);   }

static int
arr_putc(int ch, char *arr[])
{
    size_t len       = arr_len(*arr);
    int    zero_term = len > 0 && arr[0][len-1] == '\0';

    if (zero_term)
    {   arr[0][len-1] = (char)ch;   } // overwrite the null terminator
    else
    {   arr_push(*arr, (char)ch);   } // append the new character

    arr_push(*arr, '\0');
    return ch;
}

static void
arr_rewind(char *arr[])
{   arr_clear(*arr);   }

typedef struct ArrFile {
    int  (*const printf)(char *arr[], char const *fmt, ...);
    int  (*const puts  )(char const *str, char *arr[]);
    int  (*const putc  )(int ch, char *arr[]);
    void (*const rewind)(char *arr[]);
} ArrFile;

static ArrFile const Arr_File = {
    arr_printf,
    arr_puts,
    arr_putc,
    arr_rewind,
};
#endif//AHD_IMPLEMENTATION

#define AHD_INCLUDED
#endif/*AHD_INCLUDED*/

#ifdef __cplusplus
}
#endif
/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
