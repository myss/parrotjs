
#ifndef PJS_MATH_H
#define PJS_MATH_H

/* TODO
    It's probably not the way to go, and it even may not work as expected.
    It should later be implemented in parrot in a platform independent way!
    I'm not sure if compiler optimizations break this.
*/

/*
    DO N0T USE: 
        MACRO(something(x))
    BUT:
        y = something(x); 
        MACRO(y)
*/

#define PJS_NAN             (0.0 / 0.0)
#define PJS_POSINF          (1.0 / 0.0)
#define PJS_NEGINF          (-1.0 / 0.0)

#define PJS_IS_NAN(f)       ((f) != (f))

#define PJS_IS_POSINF(f)    ((f) == PJS_POSINF)
#define PJS_IS_NEGINF(f)    ((f) == PJS_NEGINF)

#define PJS_IS_INF(f)       (PJS_IS_POSINF(f) || PJS_IS_NEGINF(f))
#define PJS_IS_NORMAL(f)    (! ( PJS_IS_NAN(f) || PJS_IS_INF(f) ) )

#define PJS_ABS(f)          ((f) < 0 ? -(f) : (f))

#define PJS_TO_UINTVAL(f)   (PJS_IS_NORMAL(f) ? ((UINTVAL) PJS_ABS(f)) : 0)

/* It seems that casting to UINTVAL then to INTVAL works for what ECMA spec 
   requires. But will it work for all platforms? 
   TODO investigate it.
 */
#define PJS_TO_INTVAL(f)    (PJS_IS_NORMAL(f) ? ((INTVAL) ((UINTVAL) (f))) : 0)

#define PJS_IS_INTVAL(f)    ((f) == (FLOATVAL) PJS_TO_INTVAL(f))
#define PJS_IS_UINTVAL(f)   ((f) == (FLOATVAL) PJS_TO_UINTVAL(f))

#endif
