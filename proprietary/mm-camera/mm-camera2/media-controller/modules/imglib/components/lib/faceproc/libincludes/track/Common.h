#ifndef COMMON_H__
#define COMMON_H__


#if (!defined( WIN32 ) && !defined( WIN64 ))
#ifndef FACEPROCDEF_H__
#define FACEPROCDEF_H__
typedef unsigned char RAWIMAGE;  /* RawImageData */

typedef void          VOID;
typedef int           BOOL;
typedef int           INT32;
typedef unsigned int  UINT32;
typedef float         FLOAT32;
typedef double        FLOAT64;


#ifndef TRUE
#define TRUE  (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef NULL
#define NULL (0)
#endif

typedef struct tagPOINT {
	INT32   x;
	INT32   y;
} POINT;

typedef struct tagRECT {
	INT32   left;
	INT32   top;
	INT32   right;
	INT32   bottom;
} RECT;
#endif /* FACEPROCDEF_H__ */
#endif  /* WIN32 && WIN64 */



/* ERROR CODE */
#define RET_NORMAL           (0)   /* Normal end */
#define RET_ERR_VARIOUS      (-1)  /* Undefined error */
#define RET_ERR_INITIALIZE   (-2)  /* Initialization error */
#define RET_ERR_INVALIDPARAM (-3)  /* Parameter error */
#define RET_ERR_ALLOCMEMORY  (-4)  /* Memory allocation error */

#ifndef MAX
#define	MAX(a, b) (((a) > (b)) ? (a) : (b))	/* Maximum value */
#endif

#ifndef MIN
#define	MIN(a, b) (((a) < (b)) ? (a) : (b))	/* Minimum value */
#endif

#ifndef CLAMP
#define	CLAMP(_min,x,_max)   MIN( MAX((_min),(x)), (_max) )
#endif



#endif
