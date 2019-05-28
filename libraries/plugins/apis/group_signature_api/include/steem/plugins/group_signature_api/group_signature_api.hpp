#pragma once
#include <steem/plugins/json_rpc/utility.hpp>

#include <steem/protocol/types.hpp>

#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/vector.hpp>

namespace steem { namespace plugins { namespace group_signature {
    namespace detail
    {
        class group_signature_api_impl;
    }

    struct get_vk_args {
        std::vector<string> string1;
    };

    struct get_ok_args {
        std::vector<string> string2;
    };

    struct get_vk_return {
        string sum1;
    };

    struct get_ok_return {
        string sum2;
    };

    class group_signature_api {
        public:
        group_signature_api();
        ~group_signature_api();

        DECLARE_API ( (get_vk) (get_ok) )

        private:
        std::unique_ptr< detail::group_signature_api_impl > my;
    };

} } }

FC_REFLECT( steem::plugins::group_signature::get_vk_args, (string1) )
FC_REFLECT( steem::plugins::group_signature::get_ok_args, (string2) )
FC_REFLECT( steem::plugins::group_signature::get_vk_return, (sum1) )
FC_REFLECT( steem::plugins::group_signature::get_ok_return, (sum2) )