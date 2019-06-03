#include <steem/plugins/group_signature_api/group_signature_api_plugin.hpp>
#include <steem/plugins/group_signature_api/group_signature_api.hpp>

namespace steem { namespace plugins { namespace group_signature {
    namespace detail
    {
        class group_signature_api_impl{
            public:

            const group_signature::group_signature_plugin& _gp;

            group_signature_api_impl(): _gp(appbase::app().get_plugin<steem::plugins::group_signature::group_signature_plugin>() )
            {}

            ~group_signature_api_impl() {}
            //参数必须写，可能是使用的函数定义不是用的 宏的问题？
            get_vk_return get_vk(const get_vk_args &args)
            {
                get_vk_return a;
                char temp1[400] = {'\0'};
                element_snprintf(temp1,320,"%B", _gp.my->vk);
                a.vk = temp1;
                return a;
            }

            get_ok_return get_ok(const get_ok_args &args)const {
                get_ok_return b;
                const char *e1 = args.e1.c_str();
                const char *e2 = args.e2.c_str();
                oktype ok(_gp.my->pairing);
                _gp.my->openkey(e1,e2,&ok);
                char temp1[400] = {'\0'};
                element_snprintf(temp1,320,"%B", ok.ok1);
                b.ok1 = temp1;
                char temp2[400] = {'\0'};
                element_snprintf(temp2,320,"%B", ok.ok2);
                b.ok2 = temp2;
                return b;
            }

            extract_return extract(const extract_args &args)
            {
                extract_return key;
                ZR_ELEMENT(UID,_gp.my->pairing);
                const char *u = args.userID.c_str();
                element_from_hash(UID,(void*)u,10);
                usktype usk(_gp.my->pairing);
                _gp.my->Extract_administrator(UID,&usk);
                char temp1[400] = {'\0'};
                element_snprintf(temp1,320,"%B", usk.b0);
                key.b0 = temp1;
                char temp2[400] = {'\0'};
                element_snprintf(temp2,320,"%B", usk.b3);
                key.b3 = temp2;
                char temp3[400] ={'\0'};
                element_snprintf(temp3,320,"%B", usk.b4);
                key.b4 = temp3;
                char temp4[400] ={'\0'};
                element_snprintf(temp4,320,"%B", usk.b5);
                key.b5 = temp4;
                return key;
            }
        };
    }

    group_signature_api::group_signature_api()
    {
        auto gp_plugin = appbase::app().find_plugin<steem::plugins::group_signature::group_signature_plugin>();
        if(gp_plugin !=  nullptr){
            my = std::make_unique<detail::group_signature_api_impl>();
        }
        else{
            FC_ASSERT(false,"group_signature_plugin is not enabled");
        }
        JSON_RPC_REGISTER_API(STEEM_GROUP_SIGNATURE_API_PLUGIN_NAME);
    }

    group_signature_api::~group_signature_api() {}

    DEFINE_LOCKLESS_APIS( group_signature_api, (get_vk) (get_ok) (extract));
    
}
}
}