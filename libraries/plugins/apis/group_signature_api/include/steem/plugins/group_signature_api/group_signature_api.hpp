#pragma once
#include <steem/plugins/json_rpc/utility.hpp>
#include<steem/plugins/json_rpc/json_rpc_plugin.hpp>
#include<steem/plugins/group_signature/group_signature.hpp>

#include <steem/protocol/types.hpp>

#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/vector.hpp>
// #include <fc/fwd.hpp>
// #include <fc/array.hpp>
// #include <fc/io/raw_fwd.hpp>

namespace steem { namespace plugins { namespace group_signature {
    namespace detail
    {
        class group_signature_api_impl;
    };

    struct extract_args {
        string userID; 
    };
    
    struct extract_return {
        string b0,b3,b4,b5;
    };

    

typedef steem::plugins::json_rpc:: void_type get_vk_args;
typedef steem::plugins::json_rpc:: void_type get_pk_args;

    struct get_vk_return {
        string vk;
    };
    struct get_pk_return{
        string pk;
    };
    struct get_ok_args {
        string e1;
        string e2;
    };
    struct get_ok_return {
        string ok1;
        string ok2;
    };

    class group_signature_api {
        public:
        group_signature_api();
        ~group_signature_api();

        DECLARE_API ( (extract) (get_vk) (get_pk) (get_ok) )

        private:
        std::unique_ptr< detail::group_signature_api_impl > my;
    };

} } }

// FC_REFLECT( steem::plugins::group_signature::get_vk_args, (string1) )
FC_REFLECT( steem::plugins::group_signature::get_ok_args, (e1)(e2) )
FC_REFLECT( steem::plugins::group_signature::get_vk_return, (vk) )
FC_REFLECT( steem::plugins::group_signature::get_pk_return, (pk) )

FC_REFLECT( steem::plugins::group_signature::get_ok_return, (ok1)(ok2) )
FC_REFLECT( steem::plugins::group_signature::extract_args, (userID))
FC_REFLECT( steem::plugins::group_signature::extract_return,(b0)(b3)(b4)(b5))