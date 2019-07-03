#pragma once

// #include<steem/plugins/p2p/p2p_plugin.hpp>
#include<steem/chain/steem_fwd.hpp>
#include<steem/plugins/group_signature/group_signature_plugin.hpp>
#include<appbase/application.hpp>

#define STEEM_GROUP_SIGNATURE_PLUGIN_NAME "group_signature"

namespace steem { namespace plugins { namespace group_signature{
namespace bpo = boost::program_options;

namespace detail{class group_signature_plugin_impl;}
using namespace appbase;


class group_signature_plugin : public plugin<group_signature_plugin>{

    public:

        group_signature_plugin();

        ~group_signature_plugin();

        APPBASE_PLUGIN_REQUIRES()

        
        virtual void set_program_options(bpo::options_description &,
                                    bpo::options_description &config_file_options) override;

        static const std:: string &name(){
            static std::string name = STEEM_GROUP_SIGNATURE_PLUGIN_NAME;
            return name;
        }

        virtual void plugin_initialize(const boost::program_options::variables_map& options) override;//生成秘钥
        virtual void plugin_startup() override;//交换秘钥，生成公钥
        virtual void plugin_shutdown() override;
    // private:
        std::unique_ptr< detail::group_signature_plugin_impl >my;
        
};

} } }

