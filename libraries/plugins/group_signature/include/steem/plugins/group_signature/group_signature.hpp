#pragma once

#include<steem/plugins/group_signature/group_signature_object.hpp>

namespace steem{ namespace plugins { namespace group_signature{


namespace detail {
    class group_signature_plugin_impl
    {
    private:
        /* data */

    public:
        int N;//管理员的数量
        int K;//门限
        int number;
        
        gsktype* gsk;
        element_t vk;

        mpktype *mpk;
        pairing_t pairing;

        element_t GID;

        group_signature_plugin_impl();
        ~group_signature_plugin_impl();
        void L(element_t I, element_t *l);
        bool VerifyShare(element_t Si, element_t Ti, element_t *Ei, int threshold, int i);
        void GenSiTi(element_t *F, element_t *temp);
        void EiGen(element_t *F, element_t *G, int threshold, element_t *temp);
        void SharesGen(element_t *si, element_t *ti, element_t *Ei);
        void FGGen(element_t s, int threshold, element_t *temp);
        void GrpSetup(element_t alpha, element_t r1);
        void Extract_administrator(element_t UID, usktype *usk);
        void openkey(const char* e1, const char* e2, oktype *ok);

    };

}

} } }