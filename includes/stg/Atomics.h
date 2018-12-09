/* ----------------------------------------------------------------------------
 *
 * (c) The GHC Team, 2005-2018
 *
 * This file provides shims for C11 atomic operations.
 *
 * If compiled with C11 (or later) the shims translate directly to the
 * corresponding C11 operations. Otherwise, they are implemented using
 * the operations from SMP.h.
 *
 * Do not #include this file directly: #include "Rts.h" instead.
 *
 * To understand the structure of the RTS headers, see the wiki:
 *   http://ghc.haskell.org/trac/ghc/wiki/Commentary/SourceTree/Includes
 *
 * NOTE: assumes that "SMP.h" is included
 *
 * -------------------------------------------------------------------------- */

#pragma once

/*
 * Do not define functions in .hc files. See SMP.h for the explanation.
 */
#if !IN_STG_CODE || IN_STGCRUN

/*
 * C11: atomic_load_explicit(p, memory_order_acquire).
 */
EXTERN_INLINE StgWord acquire_load(StgVolatilePtr p);

/*
 * C11: atomic_load_explicit(p, memory_order_relaxed).
 */

EXTERN_INLINE StgWord relaxed_load(StgVolatilePtr p);

/*
 * C11: atomic_store_explicit(p, n, memory_order_relaxed).
 */

EXTERN_INLINE void relaxed_store(StgVolatilePtr p, StgWord n);

/*
 * C11: atomic_store_explicit(p, n, memory_order_release).
 */
EXTERN_INLINE void release_store(StgVolatilePtr p, StgWord n);

/*
 * C11: atomic_compare_exchange_strong_explicit(p, &o, n,
 *          memory_order_relaxed, memory_order_relaxed).
 *      return o;
 *
 * Atomically does this: 
 * 
 * relaxed_cas(p, o, n) {
 *    r = relaxed_load(p);
 *    if (r == o) { relaxed_store(p, n); }
 *    return r;
 * }
 */
EXTERN_INLINE StgWord relaxed_cas(StgVolatilePtr p, StgWord o, StgWord n);

/*
 * C11: atomic_compare_exchange_strong_explicit(p, &o, n,
 *          memory_order_release, memory_order_relaxed).
 *      return o;
 *
 * Atomically does this: 
 * 
 * release_cas(p, o, n) {
 *    r = relaxed_load(p);
 *    if (r == o) { release_store(p, n); }
 *    return r;
 * }
 */
EXTERN_INLINE StgWord release_cas(StgVolatilePtr p, StgWord o, StgWord n);

/*
 * C11: atomic_compare_exchange_strong_explicit(p, &o, n,
 *          memory_order_acq_rel, memory_order_acquire).
 *      return o;
 *
 * Atomically does this: 
 * 
 * acq_rel_cas(p, o, n) {
 *    r = acquire_load(p);
 *    if (r == o) { relaxed_store(p, n); }
 *    return r;
 * }
 */
EXTERN_INLINE StgWord acq_rel_cas(StgVolatilePtr p, StgWord o, StgWord n);

/* ----------------------------------------------------------------------------
   Implementations
   ------------------------------------------------------------------------- */

#if (defined(THREADED_RTS) && \
     defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
#include <stdatomic.h>

EXTERN_INLINE StgWord
acquire_load(StgVolatilePtr p)
{
    return atomic_load_explicit((volatile _Atomic StgWord*)p,
            memory_order_acquire);
}

EXTERN_INLINE StgWord
relaxed_load(StgVolatilePtr p)
{
    return atomic_load_explicit((volatile _Atomic StgWord*)p,
            memory_order_relaxed);
}

EXTERN_INLINE void
relaxed_store(StgVolatilePtr p, StgWord n)
{
    return atomic_store_explicit((volatile _Atomic StgWord*)p, n,
            memory_order_relaxed);
}

EXTERN_INLINE void
release_store(StgVolatilePtr p, StgWord n)
{
    return atomic_store_explicit((volatile _Atomic StgWord*)p, n,
            memory_order_release);
}

EXTERN_INLINE StgWord
relaxed_cas(StgVolatilePtr p, StgWord o, StgWord n)
{
    atomic_compare_exchange_strong_explicit(
        (volatile _Atomic StgWord*)p, &o, n,
        memory_order_relaxed, memory_order_relaxed);
    return o;
}

EXTERN_INLINE StgWord
release_cas(StgVolatilePtr p, StgWord o, StgWord n)
{
    atomic_compare_exchange_strong_explicit(
        (volatile _Atomic StgWord*)p, &o, n,
        memory_order_release, memory_order_relaxed);
    return o;
}

EXTERN_INLINE StgWord
acq_rel_cas(StgVolatilePtr p, StgWord o, StgWord n)
{
    atomic_compare_exchange_strong_explicit(
        (volatile _Atomic StgWord*)p, &o, n,
        memory_order_acq_rel, memory_order_acquire);
    return o;
}

/* ---------------------------------------------------------------------- */
#else /* !THREADED_RTS or !C11 */

EXTERN_INLINE StgWord
acquire_load(StgVolatilePtr p)
{
    StgWord r = *p;
    load_load_barrier();
    return r;
}

EXTERN_INLINE StgWord
relaxed_load(StgVolatilePtr p)
{
    return *p;
}

EXTERN_INLINE void
relaxed_store(StgVolatilePtr p, StgWord n)
{
    *p = n;
}

EXTERN_INLINE void
release_store(StgVolatilePtr p, StgWord n)
{
    write_barrier();
    *p = n;
}

EXTERN_INLINE StgWord
relaxed_cas(StgVolatilePtr p, StgWord o, StgWord n)
{
   return cas(p, o, n);
}

EXTERN_INLINE StgWord
release_cas(StgVolatilePtr p, StgWord o, StgWord n)
{
    write_barrier();
    return relaxed_cas(p, o, n);
}

EXTERN_INLINE StgWord
acq_rel_cas(StgVolatilePtr p, StgWord o, StgWord n)
{
    write_barrier();
    StgWord r = relaxed_cas(p, o, n);
    load_load_barrier();
    return r;
}

#endif /* THREADED_RTS and C11 */

#endif /* !IN_STG_CODE || IN_STGCRUN */
