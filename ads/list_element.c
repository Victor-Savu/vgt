#include <vgt/list_element.h>
#include <vgt/list_element_cls.h>


Obj leGet(ListElement e)
{
    if (!e) return 0;
    return e->data;
}

ListElement leSplit(ListElement e)
{
    if (!e) return 0;

    ListElement r = e->next;
    ListElement f = r;

    while (e && f) {
        e->next = f->next;
        e = e->next;
        if (e) {
            f->next = e->next;
            f = f->next;
        }
    }

    return r;
}

ListElement leMerge(ListElement e, ListElement f, const CompareMethod cmp)
{
    if (!e) return f;
    if (!f) return e;

    struct ListElement M;
    ListElement m = &M;
    while (e && f) {
        if (cmp(e->data, f->data)) {
            m->next = e;
            e = e->next;
        } else {
            m->next = f;
            f = f->next;
        }
        m = m->next;
    }
    if (e) {
        m->next = e;
    } else {
        m->next = f;
    }

    return M.next;
}

ListElement leSort(ListElement e, const CompareMethod cmp)
{
    ListElement f = leSplit(e);
    if (!f) return e;
    return leMerge(leSort(e, cmp), leSort(f, cmp), cmp);
}

