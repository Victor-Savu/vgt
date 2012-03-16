#include <vgt/list.h>
#include <vgt/obj.h>

#include <stdio.h>
#include <vgt/vec.h>

void test_create_and_destroy();
void test_insertion_and_removal();
void test_iteration();
void test_data_ownage();
void test_split();
void test_merge();
void test_sort();

int main()
{
    test_create_and_destroy();
    test_insertion_and_removal();
    test_iteration();
    test_data_ownage();
    test_split();
    test_merge();
    test_sort();
}

bool cmp(Obj a, Obj b)
{
    uint* x = a;
    uint* y = b;
    return (*x < *y);
}

void test_create_and_destroy()
{
    List l = lCreate(cmp, 0);
    lDestroy(l);
}

void test_insertion_and_removal()
{
    printf("Insertion and removal:\n");
    uint* data = oCreate(10 * sizeof (uint));
    int i=0;
    for (i=0; i<10; i++) data[i] = i+1;
    List l = lCreate(cmp, 0);

    uint* p = data+10;
    while (p > data) lInsert(l, 0, --p);

    ListElement e = 0;
    while (lSize(l)) {
        e = lRemove(l, 0);
        printf("%u ", * oCast(uint*, leGet(e)) );
        oDestroy(e);
    }
    printf("\n");

    lDestroy(l);
    oDestroy(data);
}

void test_iteration()
{
    printf("Iteration:\n");
    uint* data = oCreate(10 * sizeof (uint));
    int i=0;
    for (i=0; i<10; i++) data[i] = i+1;
    List l = lCreate(cmp, 0);

    uint* p = data+10;
    while (p > data) lInsert(l, 0, --p);

    ListElement e = 0;
    while ( (e = lNext(l, e)) ) {
        printf("%u ", * oCast(uint*, leGet(e)) );
    }
    printf("\n");

    lDestroy(l);
    oDestroy(data);
}

void test_data_ownage()
{
    printf("Data ownage:\n");
    List l = lCreate(cmp, oDestroy);

    unsigned int i=0;
    for (i=0;i<100; i++) lInsert(l, 0, oCreate(sizeof (uint)) );

    ListElement e = 0;
    while ( (e = lNext(l, e)) ) {
        *oCast(uint*, leGet(e)) = i--;
    }
    while ( (e = lNext(l, e)) ) {
        printf("%u ", * oCast(uint*, leGet(e)) );
    }
    printf("\n");

    lDestroy(l);
}

void test_split()
{
    printf("Split:\n");
    List l = lCreate(cmp, oDestroy);

    unsigned int i=0;
    for (i=0;i<100; i++) lInsert(l, 0, oCreate(sizeof (uint)) );

    ListElement e = 0;
    while ( (e = lNext(l, e)) ) {
        *oCast(uint*, leGet(e)) = i--;
    }

    List r = lSplit(l);
    e = 0;
    while ( (e = lNext(l, e)) ) {
        printf("%u ", * oCast(uint*, leGet(e)) );
    }
    printf("\n");
    e = 0;
    while ( (e = lNext(r, e)) ) {
        printf("%u ", * oCast(uint*, leGet(e)) );
    }
    printf("\n");

    lDestroy(l);
    lDestroy(r);
}


void test_merge()
{
    printf("Merge:\n");
    List l = lCreate(cmp, oDestroy);
    List r = lCreate(cmp, oDestroy);

    unsigned int i=0;
    for (i=0;i<100; i++) lInsert(l, 0, oCreate(sizeof (uint)) );
    for (i=0;i<100; i++) lInsert(r, 0, oCreate(sizeof (uint)) );

    ListElement e = 0;
    ListElement f = 0;
    while ( (e = lNext(l, e))  && (f = lNext(r, f))) {
        *oCast(uint*, leGet(e)) = 100 - i;
        *oCast(uint*, leGet(f)) = 150 - (i--);
    }

    e = 0;
    while ( (e = lNext(l, e)) ) {
        printf("%u ", * oCast(uint*, leGet(e)) );
    }
    printf("\n");
    f = 0;
    while ( (f = lNext(r, f)) ) {
        printf("%u ", * oCast(uint*, leGet(f)) );
    }
    printf("\n");

    printf("Merging...");fflush(stdout);
    l = lMerge(l, r);
    printf("Done!\n");fflush(stdout);
    e = 0;
    while ( (e = lNext(l, e)) ) {
        printf("%u ", * oCast(uint*, leGet(e)) );
    }
    printf("\n");

    lDestroy(l);
}


void test_sort()
{
    printf("Sort:\n");
    List l = lCreate(cmp, oDestroy);

    unsigned int i=0;
    for (i=0;i<100; i++) lInsert(l, 0, oCreate(sizeof (uint)) );

    ListElement e = 0;
    while ( (e = lNext(l, e)) ) {
        *oCast(uint*, leGet(e)) = i--;
    }
    unused( lSort(l) );
    e = 0;
    while ( (e = lNext(l, e)) ) {
        printf("%u ", * oCast(uint*, leGet(e)) );
    }
    printf("\n");

    lDestroy(l);
}

