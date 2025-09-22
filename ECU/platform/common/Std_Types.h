#ifndef STD_TYPES_H
#define STD_TYPES_H

#include <stdint.h>

typedef uint8_t   uint8;
typedef uint16_t  uint16;
typedef uint32_t  uint32;
typedef int32_t   sint32;
typedef int16_t   sint16;

typedef uint8     boolean;
#ifndef TRUE
#define TRUE  ((boolean)1u)
#endif
#ifndef FALSE
#define FALSE ((boolean)0u)
#endif

typedef uint8 Std_ReturnType;
#ifndef E_OK
#define E_OK     ((Std_ReturnType)0u)
#endif
#ifndef E_NOT_OK
#define E_NOT_OK ((Std_ReturnType)1u)
#endif

#endif /* STD_TYPES_H */
