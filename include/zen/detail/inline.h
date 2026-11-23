#ifndef ZEN_DETAIL_INLINE_H
#define ZEN_DETAIL_INLINE_H

/* MSVC's C mode does not implement C99 inline semantics; __inline is its
   equivalent. Every public header uses this single internal definition. */
#if defined(_MSC_VER) && !defined(__clang__)
#define ZEN_STATIC_INLINE static __inline
#else
#define ZEN_STATIC_INLINE static inline
#endif

#endif /* ZEN_DETAIL_INLINE_H */
