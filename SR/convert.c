#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "convert.h"

/****************************************************************************
 * * NAME:binarySearch
 * * PURPOSR:perform binary search in uint16 array
 * * ENTRY:table- point to uint16 array
 * *tablen- length of array
 * *code- word to search
 * * EXIT:int16 - index of object, -1 for no match
 * * AUTHOR: lvjie November 18, 2003
 * ****************************************************************************/
static short binarySearch(const unsigned short *table,
                          unsigned short tablen,
                          unsigned short code)
{
    unsigned short head,tail,middle;

    head = 0;
    tail = tablen-1;
    if ((code < table[head])||(code > table[tail]))
        return(-1);

    while (head <= tail)
    {
        middle = (head+tail)/2;
        if (code == table[middle])
            return (middle);
        else if (code > table[middle])
            head = middle+1;
        else
            tail = middle-1;
    }

    return (NOT_SUPPORTED);
}

/* convert gb2312 word to unicode word.
 *  return ? if unsupported */
unsigned short gbc2uc( unsigned short gbc )
{
    short index = binarySearch(gbkAGbkcode, CODE_NUM, gbc);

    if ( index == NOT_SUPPORTED ) {
        return '?';
    }
    else {
        return gbkAUnicode[index];
    }
}

static int getUniLenOfGbStr( const unsigned char *p )
{
    int len = 0;

    while ( *p ) {
        if ( *p & 0x80 ) {
            p += 2;
        }
        else {
            p += 1;
        }
        len++;
    }
    return len;
}

/****************************************************************************
 * * NAME:gb2uni
 * * PURPOSR:Convert gbk string to unicode string
 * *the byte order of unicode is dependent on CPU
 * *the two-byte code in GBK is always big-endian
 * * ENTRY:gbstrgbk string
 * *unibufoutput buffer
 * *buflenlength of buffer
 * * EXIT:length of unicode string
 * ****************************************************************************/
int gb2uni( const unsigned char *gbstr, unsigned short *unibuf, int buflen )
{
    int unilen, i;
    unilen = getUniLenOfGbStr(gbstr)+2;
    if ( !unibuf || ( buflen <= 0 ) ) {
        return unilen;
    }

    if ( unilen > buflen ) {
        unilen = buflen;
    }

    for ( i = 0; i < unilen; i++ ) {
        if ( *gbstr & 0x80 ) {
            /* gbk-code is big-endian */
            unsigned short gbc = ( gbstr[0] << 8 ) + gbstr[1];
            unibuf[i] = gbc2uc(gbc);
            gbstr += 2;
        }
        else {
            unibuf[i] = *gbstr;
            gbstr += 1;
        }
    }

    return unilen;
}

/* convert unicode word to gb2312 word.
 *  return ?? if unsupported */
unsigned short uc2gbc( unsigned short uc )
{
    short index = binarySearch(uniAUnicode, CODE_NUM, uc);

    if ( index == NOT_SUPPORTED ) {
        return 0x3f3f;/* ?? */
    }
    else {
        return uniAGbkcode[index];
    }
}

static int getGbLenOfUniStr( const unsigned short *p )
{
    int len = 0;

    while ( *p ) {
        if ( *p < 0x80 ) {
            len += 1;
        }
        else {
            len += 2;/* convert unsupport char to ?? */
        }
        p++;
    }
    return len;
}

/****************************************************************************
 * * NAME:uni2gb
 * * PURPOSR:Convert unicode string to gbk string
 * *the byte order of unicode MUST be consistent with underlying CPU
 * *the two-byte code in GBK is always big-endian
 * * ENTRY:unistrunicode string
 * *gbbufoutput buffer
 * *buflenlength of buffer
 * * EXIT:length of gbk string
 * ****************************************************************************/
int uni2gb(const unsigned short *unistr, unsigned char *gbbuf, int buflen )
{
    int gblen, i;
    gblen = getGbLenOfUniStr(unistr)+2;
    if ( !gbbuf || ( buflen <= 0 ) ) {
        return gblen;
    }

    if ( gblen > buflen ) {
        gblen = buflen;
    }

    i = 0;
    while ( i < gblen ) {
        if ( *unistr < 0x80 ) {
            gbbuf[i] = (char)(*unistr);
            i++;
        }
        else {
            /* gbk-code is big-endian */
            unsigned short t = uc2gbc(*unistr);
            gbbuf[i++] = (unsigned char)( t >> 8 );
            gbbuf[i++] = (unsigned char)( t & 0xff );
        }
        unistr++;
    }

    return gblen;
}

typedef   unsigned char  __u8;
typedef   unsigned short __u16;
//typedef   unsigned short wchar_t;


/*
 *  * Sample implementation from Unicode home page.
 *   * http://www.stonehand.com/unicode/standard/fss-utf.html
 *    */
struct utf8_table {
    int     cmask;
    int     cval;
    int     shift;
    long    lmask;
    long    lval;
};

static struct utf8_table utf8_table[] =
{
    {0x80,  0x00,   0*6,    0x7F,           0,         /* 1 byte sequence */},
    {0xE0,  0xC0,   1*6,    0x7FF,          0x80,      /* 2 byte sequence */},
    {0xF0,  0xE0,   2*6,    0xFFFF,         0x800,     /* 3 byte sequence */},
    {0xF8,  0xF0,   3*6,    0x1FFFFF,       0x10000,   /* 4 byte sequence */},
    {0xFC,  0xF8,   4*6,    0x3FFFFFF,      0x200000,  /* 5 byte sequence */},
    {0xFE,  0xFC,   5*6,    0x7FFFFFFF,     0x4000000, /* 6 byte sequence */},
    {0,                            /* end of table    */}
};

int
utf8_mbtowc(wchar_t *p, const __u8 *s, int n)
{
    long l;
    int c0, c, nc;
    struct utf8_table *t;

    nc = 0;
    c0 = *s;
    l = c0;
    for (t = utf8_table; t->cmask; t++) {
        nc++;
        if ((c0 & t->cmask) == t->cval) {
            l &= t->lmask;
            if (l < t->lval)
                return -1;
            *p = l;
            return nc;
        }
        if (n <= nc)
            return -1;
        s++;
        c = (*s ^ 0x80) & 0xFF;
        if (c & 0xC0)
            return -1;
        l = (l << 6) | c;
    }
    return -1;
}

int
utf8_mbstowcs(wchar_t *pwcs, const __u8 *s, int n)
{
    __u16 *op;
    wchar_t v_op;
    const __u8 *ip;
    int size;

    op = (__u16 *)pwcs;
    ip = s;
    while (*ip && n > 0) {
        if (*ip & 0x80) {
            size = utf8_mbtowc(&v_op, ip, n);
            *op=v_op;
            if (size == -1) {
                /* Ignore character and move on */
                ip++;
                n--;
            } else {
                op++;
                ip += size;
                n -= size;
            }
        } else {
            *op++ = *ip++;
            n--;
        }
    }
    return (op - (__u16 *)pwcs);
}


utf8_wctomb(__u8 *s, wchar_t wc, int maxlen)
{
    long l;
    int c, nc;
    struct utf8_table *t;

    if (s == 0)
        return 0;

    l = wc;
    nc = 0;
    for (t = utf8_table; t->cmask && maxlen; t++, maxlen--) {
        nc++;
        if (l <= t->lmask) {
            c = t->shift;
            *s = t->cval | (l >> c);
            while (c > 0) {
                c -= 6;
                s++;
                *s = 0x80 | ((l >> c) & 0x3F);
            }
            return nc;
        }
    }
    return -1;
}

int
utf8_wcstombs(__u8 *s, const wchar_t *pwcs, int maxlen)
{
    const __u16 *ip;
    __u8 *op;
    int size;

    op = s;
    ip = (__u16 *)pwcs;
    while (*ip && maxlen > 0) {
        if (*ip > 0x7f) {
            size = utf8_wctomb(op, *ip, maxlen);
            if (size == -1) {
                /* Ignore character and move on */
                maxlen--;
            } else {
                op += size;
                maxlen -= size;
            }
        } else {
            *op++ = (__u8) *ip;
        }
        ip++;
    }
    return (op - s);
}

static    wchar_t *pw;
static    unsigned short * ps;
static    unsigned char *pc;

unsigned char utf16[100];

int utf8_to_gb2312(unsigned char * utf8,unsigned char * gb2312,int size)
{
    int i;

    if(!utf8||!gb2312)
        return -1;

    memset(utf16,0,sizeof(utf16));

    //utf8 to utf16
    pc=(unsigned char *)utf8;
    pw=(wchar_t *)utf16;

    utf8_mbstowcs(pw, pc, sizeof(utf16));

    //utf16 to gb2312
    ps=(unsigned short *)utf16;
    pc=(unsigned char *)gb2312;

    uni2gb(ps, pc, size);

    return 0;
}

int gb2312_to_utf8(unsigned char * gb2312,unsigned char * utf8,int size)
{
    if(!utf8||!gb2312)
        return -1;

    memset(utf16,0,sizeof(utf16));

    //gb2312 to utf16
    ps=(unsigned short *)utf16;
    pc=(unsigned char *)gb2312;

    gb2uni(pc, ps, sizeof(utf16));

    //utf16 to utf8
    memset(utf8,0,size);
    pw=(wchar_t *)utf16;
    pc=(unsigned char *)utf8;

    utf8_wcstombs(pc, pw, size);

    return 0;
}
