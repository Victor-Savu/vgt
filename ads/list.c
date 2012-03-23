#include <ads/list.h>
#include <ads/list_cls.h>

#include <ads/list_element.h>
#include <ads/list_element_cls.h>

#include <stdio.h>

#include <math/obj.h>

void do_nothing(Obj o)
{
    unused(o);
}

bool compare_addresses(Obj a, Obj b)
{
    return a<b;
}

List lCreate(const CompareMethod cmp, const DeleteMethod del)
{
    List l = oCreate(sizeof (struct List));
    if (cmp) {
        l->cmp = cmp;
    } else {
        l->cmp = compare_addresses;
    }
    if(del) {
        l->del = del;
    } else {
        l->del = do_nothing;
    }
    l->size = 0;
    l->head = 0;

    return l;
}


ListElement lInsert(List l, ListElement at, Obj o)
{
    ListElement e = oCreate(sizeof (struct ListElement));
    e->data = o;

    if (at) {
        e->next = at->next;
        at->next = e;
    } else {
        e->next = l->head;
        l->head = e;
    }
    l->size++;

    return e;
}

ListElement lImport(List l, ListElement at, ListElement e)
{
    if (!e) {
        safe( fprintf(stderr, "[!] Tried to import a null element into a list.\n"); fflush(stderr); )
        return at;
    }

    if (at) {
        e->next = at->next;
        at->next = e;
    } else {
        e->next = l->head;
        l->head = e;
    }
    l->size++;
    return e;
}

List lSplit(List l)
{
    if (!l) return 0;

    List r = lCreate(l->cmp, l->del);

    r->head = leSplit(l->head);
    r->size = l->size / 2;
    l->size -= r->size;

    return r;
}

List lMerge(List l, List r)
{
    if (!l) return r;
    if (!r) return l;

    if (!lSize(l)) {
        lDestroy(l);
        return r;
    }
    if (!lSize(r)) {
        lDestroy(r);
        return l;
    }

    if ((l->cmp != r->cmp) || (l->del != r->del)) {
        fprintf(stderr, "[!] Cannot merge lists with different comparrison or delete methods.\n");
        fflush(stderr);
        return 0;
    }

    printf(" ..."); fflush(stdout);
    l->head = leMerge(l->head, r->head, l->cmp);
    printf(" ... "); fflush(stdout);
    r->head = 0;
    l->size += r->size;
    r->size = 0;

    lDestroy(r);

    return l;
}

List lSort(List l)
{
    if (l) l->head = leSort(l->head, l->cmp);
    return l;
}


ListElement lBegin(List l)
{
    return l->head;
}


ListElement lNext(List l, ListElement e)
{
    if (e) {
        return e->next;
    } else {
        return l->head;
    }
}


uint64_t lSize(List l)
{
    return l->size;
}


ListElement lRemove(List l, ListElement e)
{
    ListElement d = 0;
    if (e) {
        d = e->next;
        if (d) e->next = d->next;
    } else {
        d = l->head;
        if (d) l->head = d->next;
    }
    if (d) {
        l->size--;
        d->next = 0;
    }
    return d;
}

void lDelete(List l, ListElement e)
{
    ListElement d = lRemove(l, e);
    if (d) l->del(d->data);
    oDestroy(d);
}

void lDestroy(List l)
{
    while (lSize(l)) lDelete(l, 0);
    oDestroy(l);
}

