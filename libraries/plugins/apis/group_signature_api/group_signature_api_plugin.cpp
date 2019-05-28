#include <steem/plugins/group_signature_api/group_signature_api_plugin.hpp>
#include <steem/plugins/group_signature_api/group_signature_api.hpp>

namespace steem { namespace plugins { namespace group_signature {
    group_signature_api_plugin::group_signature_api_plugin() {}
    group_signature_api_plugin::~group_signature_api_plugin() {}

    void group_signature_api_plugin::set_program_options( options_description& cli, options_description& cfg) {}

    void group_signature_api_plugin::plugin_initialize( const variables_map& options)
    {
        api = std::make_shared< group_signature_api >();
    }

    void group_signature_api_plugin::plugin_startup() {}
    void group_signature_api_plugin:: plugin_shutdown() {}

} } }