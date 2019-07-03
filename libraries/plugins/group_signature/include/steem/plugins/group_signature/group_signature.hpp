#pragma once

#include <steem/plugins/group_signature/group_signature_object.hpp>
#include<iostream>
#include <vector>
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
        std::string vk_string;

        mpktype *mpk;
        pairing_t pairing;

        element_t GID;
        element_t* Ei = NULL;

        std::vector<std::string> si_string;
        std::vector<std::string> ti_string; 
        std::vector<std::string> Ei_string;

        std::vector<std::string> si_string_received;
        std::vector<std::string> ti_string_received;
        std::vector<std::string> g_alpha_i_string_received;

        group_signature_plugin_impl();
        ~group_signature_plugin_impl();

        void L(element_t I, element_t *l);
        bool VerifyShare(std::string Si_string, std::string Ti_string, std::vector<std::string> Ei_string);
        void GenSiTi(element_t *F, element_t *temp);
        void EiGen(element_t *F, element_t *G, int threshold, element_t *temp);
        void SharesGen(element_t *s_ptr, element_t *t_ptr, element_t *E_ptr);
        void FGGen(element_t s, int threshold, element_t *temp);
        void GrpSetup(element_t alpha, element_t r1);
        void Extract_administrator(element_t UID, usktype *usk);
        void openkey(const char* e1, const char* e2, oktype *ok);
        bool Verify(element_t m, signaturetype *sig);
        bool VerifyPOK(signaturetype *sig);
        bool open(char * userID, const char * lameda,element_t e);
        void mpkGen();
        void g_alphaGen();

        bool VerifyShareSelf(element_t Si, element_t Ti, element_t *Ei, int threshold, int i);


    };

}

} } }