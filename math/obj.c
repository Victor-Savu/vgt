#include <math/obj.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void oShortPrint(Obj i, FILE* f)
{
    fprintf(f, "%d", *oCast(short*, i));
    safe(fflush(f););
}

void oIntPrint(Obj i, FILE* f)
{
    fprintf(f, "%d", *oCast(int*, i));
    safe(fflush(f););
}

void oLongPrint(Obj i, FILE* f)
{
    fprintf(f, "%ld", *oCast(long*, i));
    safe(fflush(f););
}

void oUshortPrint(Obj i, FILE* f)
{
    fprintf(f, "%u", *oCast(unsigned short*, i));
    safe(fflush(f););
}

void oUintPrint(Obj i, FILE* f)
{
    fprintf(f, "%u", *oCast(unsigned int*, i));
    safe(fflush(f););
}

void oUlongPrint(Obj i, FILE* f)
{
    fprintf(f, "%lu", *oCast(unsigned long*, i));
    safe(fflush(f););
}

void oFloatPrint(Obj i, FILE* f)
{
    fprintf(f, "%f", *oCast(float*, i));
    safe(fflush(f););
}

void oDoublePrint(Obj i, FILE* f)
{
    fprintf(f, "%lf", *oCast(double*, i));
    safe(fflush(f););
}

void oLongDoublePrint(Obj i, FILE* f)
{
    fprintf(f, "%Lf", *oCast(long double*, i));
    safe(fflush(f););
}



Obj oCreate(size_t size) //__attribute__((alloc_size(1)))
{
    Obj restrict o = malloc(size);
    if (o) {
        memset(o, 0, size);
    } else {
        fprintf(stderr, "Out of memory.\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    return o;
}

void oDestroy(Obj restrict o)
{
    if (o) free(o);
}

Obj oCopy(Obj restrict o, size_t size)// __attribute__((alloc_size(2)))
{
    Obj restrict c = malloc(size);
    if (c) {
        memcpy(c, o, size);
    } else {
        fprintf(stderr, "Out of memory.\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    return c;
}

Obj oCopyTo(Obj restrict dst, Obj restrict src, size_t size)
{
    check(dst);
    check(src);
    check(size);
    return memcpy(dst, src, size);;
}
