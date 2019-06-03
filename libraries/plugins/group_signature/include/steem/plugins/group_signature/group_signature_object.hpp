#pragma once

#include<pbc/pbc.h>


namespace steem
{
namespace plugins
{
namespace group_signature
{

class MPK
{
public:
    element_t g, g2, h1, u0, u1, u2, u3, u4, n, h;
    MPK(pairing_t pairing)
    {
        element_init_G1(g, pairing);
        element_init_G1(h, pairing);
        element_init_G2(g2, pairing);
        element_init_G1(h1, pairing);
        element_init_G2(u0, pairing);
        element_init_G2(u1, pairing);
        element_init_G2(u2, pairing);
        element_init_G2(u3, pairing);
        element_init_G2(u4, pairing);
        element_init_GT(n, pairing);
    }
    ~MPK()
    {
        element_clear(g);
        element_clear(g2);
        element_clear(h1);
        element_clear(u0);
        element_clear(u1);
        element_clear(u2);
        element_clear(u3);
        element_clear(u4);
        element_clear(n);
    }
};

class GSK
{
public:
    element_t a0, a2, a3, a4, a5;
    GSK(pairing_t pairing)
    {
        element_init_G2(a0, pairing);
        element_init_G2(a2, pairing);
        element_init_G2(a3, pairing);
        element_init_G2(a4, pairing);
        element_init_G1(a5, pairing);
    }
    ~GSK()
    {
        element_clear(a0);
        element_clear(a2);
        element_clear(a3);
        element_clear(a4);
        element_clear(a5);
    }
};


class USK
{
public:
    element_t b0, b3, b4, b5;
    USK(pairing_t pairing)
    {
        element_init_G2(b0, pairing);
        element_init_G2(b3, pairing);
        element_init_G2(b4, pairing);
        element_init_G1(b5, pairing);
    }
    ~USK()
    {
        element_clear(b0);
        element_clear(b3);
        element_clear(b4);
        element_clear(b5);
    }
};

class OK
{
public:
    element_t ok1, ok2;
    OK(pairing_t pairing)
    {
        element_init_GT(ok1, pairing);
        element_init_GT(ok2, pairing);
    }
    ~OK()
    {
        element_clear(ok1);
        element_clear(ok2);
    }
};

class SHARE
{
public:
    element_t si, ti;
    element_t *ei = NULL;
    // int i;
    int n;
    SHARE(pairing_t pairing)
    {
        element_init_Zr(si, pairing);
        element_init_Zr(ti, pairing);
    }
    ~SHARE()
    {
        element_clear(si);
        element_clear(ti);

        for (int i = 0; i < n; i++)
        {
            element_clear(ei[i]);
        }

        delete ei;
    }
};

typedef MPK mpktype;
typedef GSK gsktype;
typedef USK usktype;
typedef OK oktype;
typedef SHARE sharetype;


#define G1_ELEMENT(var, pairing) element_t var; element_init_G1(var, pairing);
#define G2_ELEMENT(var, pairing) element_t var; element_init_G2(var, pairing);
#define GT_ELEMENT(var, pairing) element_t var; element_init_GT(var, pairing);
#define ZR_ELEMENT(var, pairing) element_t var; element_init_Zr(var, pairing);

#define G1_ELEMENT_ARRAY(var, n, p)    \
    element_t *var = new element_t[n]; \
    for (int i = 0; i < n; i++)        \
    {                                  \
        element_init_G1(var[i], p);    \
    }
#define G2_ELEMENT_ARRAY(var, n, p)    \
    element_t *var = new element_t[n]; \
    for (int i = 0; i < n; i++)        \
    {                                  \
        element_init_G2(var[i], p);    \
    }
#define GT_ELEMENT_ARRAY(var, n, p)       \
    element_t *var = new element_t[n];    \
    for (int i = 0; i < n; i++)           \
    {                                     \
        element_init_GT(var[i], p); \
    }
#define ZR_ELEMENT_ARRAY(var, n, p)       \
    element_t *var = new element_t[n];    \
    for (int i = 0; i < n; i++)           \
    {                                     \
        element_init_Zr(var[i], p); \
    }
#define INIT_ARRAY_1(var, n)    \
    for (int i = 0; i < n; i++) \
    {                           \
        element_set1(var[i]);   \
    }
#define INIT_ARRAY_0(var, n)    \
    for (int i = 0; i < n; i++) \
    {                           \
        element_set0(var[i]);   \
    }
#define CLEAR_ELEMENT(var, n)     \
    for (int i = 0; i < n; i++) \
    {                           \
        element_clear(var[i]);  \
    }
} // namespace group_signature
} // namespace plugins

} // namespace steem
