#pragma once
#include <steem/chain/steem_fwd.hpp>

#include <steem/plugins/json_rpc/json_rpc_plugin.hpp>

#include <steem/plugins/group_signature/group_signature_plugin.hpp>

#include <appbase/application.hpp>

#define STEEM_GROUP_SIGNATURE_API_PLUGIN_NAME "group_signature_api"

namespace steem { namespace plugins { namespace group_signature {
    using namespace appbase;

    class group_signature_api_plugin : public appbase::plugin< group_signature_api_plugin >
    {
        public:
        APPBASE_PLUGIN_REQUIRES(
            (steem::plugins::group_signature::group_signature_plugin)
            (steem::plugins::json_rpc::json_rpc_plugin)
            )

        group_signature_api_plugin();
        virtual ~group_signature_api_plugin();

        static const std::string& name() { static std::string name = STEEM_GROUP_SIGNATURE_API_PLUGIN_NAME; return name; }

        virtual void set_program_options( options_description& cli, options_description& cfg ) override;

        virtual void plugin_initialize( const variables_map& options ) override;
        virtual void plugin_startup() override;
        virtual void plugin_shutdown() override; 

        std::shared_ptr< class group_signature_api > api; 
};
} } }