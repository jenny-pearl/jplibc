#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

//typedef	struct __sFILE {
//	unsigned char *_LIBC_UNSAFE_INDEXABLE	_p;	/* current position in (some) buffer */
//	int	_r;		/* read space left for getc() */
//	int	_w;		/* write space left for putc() */
//	short	_flags;		/* flags, below; this FILE is free if 0 */
//	short	_file;		/* fileno, if Unix descriptor, else -1 */
//	struct	__sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
			/* struct __sbuf { */
			/* 	unsigned char *_LIBC_COUNT(_size)	_base; */
			/* 	int		_size; */
			/* }; */
//	int	_lbfsize;	/* 0 or -_bf._size, for inline putc */
//
//	/* operations */
//	void	*_cookie;	/* cookie passed to io functions */
//	int	(* _Nullable _close)(void *);
//	int	(* _Nullable _read) (void *, char *_LIBC_COUNT(__n), int __n);
//	fpos_t	(* _Nullable _seek) (void *, fpos_t, int);
//	int	(* _Nullable _write)(void *, const char *_LIBC_COUNT(__n), int __n);
//
//	/* separate buffer for long sequences of ungetc() */
//	struct	__sbuf _ub;	/* ungetc buffer */
//	struct __sFILEX *_extra; /* additions to FILE to not break ABI */
//	int	_ur;		/* saved _r when _r is counting ungetc data */
//
//	/* tricks to meet minimum requirements even when malloc() fails */
//	unsigned char _ubuf[3];	/* guarantee an ungetc() buffer */
//	unsigned char _nbuf[1];	/* guarantee a getc() buffer */
//
//	/* separate buffer for fgetln() when line crosses buffer boundary */
//	struct	__sbuf _lb;	/* buffer for fgetln() */
//
//	/* Unix stdio files get aligned to block boundaries on fseek() */
//	int	_blksize;	/* stat.st_blksize (may be != _bf._size) */
//	fpos_t	_offset;	/* current lseek offset (see WARNING) */
//} FILE;

uint32_t jp_printf(FILE * restrict stream, const char *format, ...)
{
	return write(1, format, strlen(format));
}

int main(void)
{
	FILE *fp = fopen("test.txt", "r");
	printf("%d\n", fp->_r);
	return 0;
}
