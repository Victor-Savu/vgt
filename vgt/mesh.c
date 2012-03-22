#include <vgt/mesh.h>
#include <vgt/mesh_cls.h>

#include <vgt/edge_cls.h>
#include <math/obj.h>
#include <math/vec.h>
#define EMPTY_MESH  { 0, 0, 0, 0}

#include <stdio.h>
#include <stdlib.h>

#include <GL/glut.h>

Mesh mCopy(Mesh restrict m)
{
    if (!m) return 0;

    Mesh restrict c = oCopy(m, sizeof(struct Mesh));
    c->vert = oCopy(m->vert, c->n_vert * sizeof(Vec));
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

Mesh mReadOff(Mesh restrict m, const char* restrict filename)
{

    FILE* f = fopen(filename, "r");
    if (!f) {
        safe( printf("[x] %s: Failed to open file [%s].", __func__, filename); )
    }


    if (!m) m = oCreate(sizeof (struct Mesh));

    char line[256];

    {// read header
        unsigned int aux, vert, faces;
        fgets(line, sizeof(line), f); // OFF
        fgets(line, sizeof(line), f); // #vertices #faces #<unused>
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
        m->vert = oCreate(m->n_vert * sizeof(Vec));
        m->norm = oCreate(m->n_vert * sizeof(Vec));
        m->edges = oCreate(m->n_edges * sizeof (struct Edge));
    }

    printf("%u vertices, %u edges\n", (uint)m->n_vert, (uint)m->n_edges);

    {// read vertices
        Vec* const end = m->vert + m->n_vert;
        Vec* i;
        float x, y, z;
        for (i = m->vert; i < end; i++) {
            fgets(line, sizeof(line), f);
            if (sscanf(line, "%f %f %f\n", &x, &y, &z) != 3) {
                fprintf(stderr, "[x] %s: failed to read vertex #%u  file [%s].",
                        __func__, oCast(unsigned int, i - m->vert),  filename);
                fflush(stderr);
                exit(0);
            }
            vSet(i, x, y, z);
        }
    }

    {// read faces
        Edge restrict e = m->edges;
        Vec* restrict n = m->norm;
        unsigned int a, b, c;
        Vec p, q, norm;
        ind i;
        for (i = 0; i < m->n_edges; i+=3, e+=3, n+=3) {
            fgets(line, sizeof(line), f);
            if (sscanf(line, "%u %u %u\n", &a, &b, &c) != 3) {
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

        // TODO: Smooth out the normals once more?
    }

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
    glBegin(GL_TRIANGLES);



    glEnd();
}
