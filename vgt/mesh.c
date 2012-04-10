#include <vgt/mesh.h>
#include <vgt/mesh_cls.h>

#include <vgt/edge_cls.h>
#include <math/obj.h>
#include <math/vertex.h>
#define EMPTY_MESH  { 0, 0, 0, 0}

#include <stdio.h>
#include <stdlib.h>

#include <GL/glut.h>

Mesh mCopy(Mesh restrict m)
{
    if (!m) return 0;

    Mesh restrict c = oCopy(m, sizeof(struct Mesh));
    c->vert = oCopy(m->vert, c->n_vert * sizeof(Vertex));
    c->edges = oCopy(m->edges, c->n_edges * sizeof(struct Edge));

    return c;
}

void mDestroy(Mesh restrict m)
{
    mClear(m);
    oDestroy(m);
}

Mesh mClear(Mesh restrict m)
{
    if (m) {
        oDestroy(m->vert);
        oDestroy(m->norm);
        oDestroy(m->edges);
    }
    return m;
}
/*
inline static
int left_cmp(const void* a_, const void* b_) {
    const Edge a = oCast(const Edge, a_);
    const Edge b = oCast(const Edge, b_);

    if (a->v < b->v) return -1;
    if (a->v > b->v) return  1;

    if (m->edges[a->n].v < m->edges[b->n].v) return -1;
    if (m->edges[a->n].v > m->edges[b->n].v) return  1;

    return 0;
}

inline static
int right_cmp(const void* a_, const void* b_) {
    const Edge a = oCast(const Edge, a_);
    const Edge b = oCast(const Edge, b_);

    if (m->edges[a->n].v < m->edges[b->n].v) return -1;
    if (m->edges[a->n].v > m->edges[b->n].v) return  1;

    if (a->v < b->v) return -1;
    if (a->v > b->v) return  1;

    return 0;
}

inline static
int cross_cmp(const Edge a, const Edge b) {

    if (a->v < m->edges[b->n].v) return -1;
    if (a->v > m->edges[b->n].v) return  1;

    if (m->edges[a->n].v < b->v) return -1;
    if (m->edges[a->n].v > b->v) return  1;

    return 0;
}
*/

Mesh mReadOff(Mesh restrict m, const char* restrict filename)
{

    FILE* f = fopen(filename, "r");
    if (!f) {
        safe( printf("[x] %s: Failed to open file [%s].", __func__, filename); );
    }


    if (!m) m = oCreate(sizeof (struct Mesh));

    char line[256];

    {// read header
        unsigned int aux, vert, faces;
        if (!fgets(line, sizeof(line), f)) { printf("[x] %s:Error reading from file %s.\n", __func__, filename); exit(EXIT_FAILURE);} // OFF
        if (!fgets(line, sizeof(line), f)) { printf("[x] %s:Error reading from file %s.\n", __func__, filename); exit(EXIT_FAILURE);}; // #vertices #faces #<unused>
        fputs(line, stdout);
        if (sscanf(line, "%u %u %u", &vert, &faces, &aux) != 3) {
            fprintf(stderr, "[x] %s: Failed to read #vert #faces #<unused> from  file [%s].",
                    __func__, filename);
            fflush(stderr);
            exit(0);
        }
        printf("%u vertices, %u faces\n", vert, faces);

        m->n_edges = faces * 3;
        m->n_vert = vert;
        m->vert = oCreate(m->n_vert * sizeof(Vertex));
        m->norm = oCreate(m->n_vert * sizeof(Vertex));
        m->edges = oCreate(m->n_edges * sizeof (struct Edge));
    }

    printf("%u vertices, %u edges\n", oCast(unsigned int,m->n_vert), oCast(unsigned int, m->n_edges));


    {// read vertices
        Vertex* const end = m->vert + m->n_vert;
        Vertex* i;
        float x, y, z;
        double gx=0, gy=0, gz=0;
        for (i = m->vert; i < end; i++) {
            if (!fgets(line, sizeof(line), f)) { printf("[x] %s:Error reading from file %s.\n", __func__, filename); exit(EXIT_FAILURE);};
            if (sscanf(line, "%f %f %f\n", &x, &y, &z) != 3) {
                fprintf(stderr, "[x] %s: failed to read vertex #%u  file [%s].",
                        __func__, oCast(unsigned int, i - m->vert),  filename);
                fflush(stderr);
                exit(0);
            }
            vSet(i, x, y, z);
            gx += x;
            gy += y;
            gz += z;
        }
        gx /= m->n_vert;
        gy /= m->n_vert;
        gz /= m->n_vert;
//        Vertex g = {gx, gy, gz};
//        for (i = m->vert; i < end; i++) vSubI(i, &g);
    }

    {// read faces
        Edge restrict e = m->edges;
        Vertex* restrict n = m->norm;
        unsigned int a, b, c, cnt;
        Vertex p, q, norm;
        ind i;

        for (i = 0; i < m->n_edges; i+=3, e+=3, n+=3) {
            if (!fgets(line, sizeof(line), f)) { printf("[x] %s:Error reading from file %s.\n", __func__, filename); exit(EXIT_FAILURE);};
            if (sscanf(line, "%u %u %u %u\n", &cnt, &a, &b, &c) != 4) {
                fprintf(stderr, "[x] %s: failed to read face #%u  file [%s].",
                        __func__, oCast(unsigned int, i/3),  filename);
                fflush(stderr);
                exit(0);
            }
            e[0].v = a;   e[1].v = b;   e[2].v = c;
            e[0].n = i+1; e[1].n = i+2; e[2].n = i;
            // mark the fact that they have no opposing edge
            e[0].o = m->n_edges; e[1].o = m->n_edges; e[2].o = m->n_edges;

            ignore vCross( vSub(m->vert+b, m->vert+a, &p),
                           vSub(m->vert+c, m->vert+a, &q),
                           &norm);
            vAddI(m->norm + a, &norm);
            vAddI(m->norm + b, &norm);
            vAddI(m->norm + c, &norm);
        }
    }
/*
    {//Smooth out the normals once more
        Edge const begin = m->edges;
        Edge const end = begin + m->n_edges;
        Edge e;
        for (e = begin; e < end; e++) ignore vAddI(m->norm + e->v, m->norm + begin[e->n].v);
    }
*/
    // find opposing edges

    Edge restrict left = oCopy(m->edges, m->n_edges * sizeof(struct Edge));
    Edge restrict right = oCopy(m->edges, m->n_edges * sizeof(struct Edge));

    int left_cmp(const void* a_, const void* b_) {
        const Edge a = oCast(const Edge, a_);
        const Edge b = oCast(const Edge, b_);

        if (a->v < b->v) return -1;
        if (a->v > b->v) return  1;

        if (m->edges[a->n].v < m->edges[b->n].v) return -1;
        if (m->edges[a->n].v > m->edges[b->n].v) return  1;

        return 0;
    }

    int right_cmp(const void* a_, const void* b_) {
        const Edge a = oCast(const Edge, a_);
        const Edge b = oCast(const Edge, b_);

        if (m->edges[a->n].v < m->edges[b->n].v) return -1;
        if (m->edges[a->n].v > m->edges[b->n].v) return  1;

        if (a->v < b->v) return -1;
        if (a->v > b->v) return  1;

        return 0;
    }

    int cross_cmp(const Edge a, const Edge b) {

        if (a->v < m->edges[b->n].v) return -1;
        if (a->v > m->edges[b->n].v) return  1;

        if (m->edges[a->n].v < b->v) return -1;
        if (m->edges[a->n].v > b->v) return  1;

        return 0;
    }

    qsort(left, m->n_edges, sizeof (struct Edge), left_cmp);
    qsort(right, m->n_edges, sizeof (struct Edge), right_cmp);

    Edge restrict l = left;
    Edge restrict r = right;

    Edge const end_l = left + m->n_edges;
    Edge const end_r = right + m->n_edges;

    while ((l < end_l) && (r < end_r)) {
        switch (cross_cmp(l, r)) {
            case -1:
                l++;
                break;
            case  1:
                r++;
                break;
            default:
                l++;r++;
                m->edges[l->n].o = r->n;
                m->edges[r->n].o = l->n;
                break;
        }
    }

    oDestroy(left);
    oDestroy(right);

    fclose(f);

    return m;
}

void mDisplay(Mesh restrict m)
{
    byte* used = oCreate( ((m->n_edges >> 3) + 1) * sizeof(byte));
    glBegin(GL_TRIANGLES);
    ind i=0;
    for (i=0; i < m->n_edges; i++) {
        if (!(used[i >> 3] & (1 << (i & 7)))) {
            const ind ea = i;
            const ind eb = m->edges[ea].n;
            const ind ec = m->edges[eb].n;

            used[ea >> 3] |= 1 << (ea & 7);
            used[eb >> 3] |= 1 << (eb & 7);
            used[ec >> 3] |= 1 << (ec & 7);

            const ind ia = m->edges[ea].v;
            const ind ib = m->edges[eb].v;
            const ind ic = m->edges[ec].v;

            Vertex* const restrict va = m->vert + ia;
            Vertex* const restrict vb = m->vert + ib;
            Vertex* const restrict vc = m->vert + ic;

            Vertex* const restrict na = m->norm + ia;
            Vertex* const restrict nb = m->norm + ib;
            Vertex* const restrict nc = m->norm + ic;

            glNormal3v(*na);
            glVertex3v(*va);
            glNormal3v(*nb);
            glVertex3v(*vb);
            glNormal3v(*nc);
            glVertex3v(*vc);
    //        printf("Triangle : "); vPrint(va, stdout); vPrint(vb, stdout); vPrint(vc, stdout);
    //        printf("\n");
        }
    }
    glEnd();
    oDestroy(used);
}
