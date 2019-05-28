#include <steem/plugins/group_signature_api/group_signature_api_plugin.hpp>
#include <steem/plugins/group_signature_api/group_signature_api.hpp>

namespace steem { namespace plugins { namespace group_signature {
    namespace detail
    {
        class group_signature_api_impl{
            public:
            group_signature_api_impl() {}
            ~group_signature_api_impl() {}

            get_vk_return get_vk(const get_vk_args &args) const {
                get_vk_return a;
                for (auto string : args.string1) {
                    a.sum1 = string;
                }
                return a;
            }

            get_ok_return get_ok(const get_ok_args &args) const {
                get_ok_return b;
                for (auto string : args.string2) {
                    b.sum2 = string;
                }
                return b;
            }
        };
    }

    group_signature_api::group_signature_api() : my(new detail::group_signature_api_impl()) {
        JSON_RPC_REGISTER_API(STEEM_GROUP_SIGNATURE_API_PLUGIN_NAME);
    }

    group_signature_api::~group_signature_api() {}

    DEFINE_LOCKLESS_APIS( group_signature_api, (get_vk) (get_ok) )
}
}
}