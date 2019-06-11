#pragma once
#include<steem/protocol/types.hpp>

namespace steem { namespace protocol{

//写入数据库中的signature
    struct signature_data{
        steem::protocol::user_signature_type c0,c5,c6,e1,e2,e3,c,s1,s2,s3;

    };

}}


FC_REFLECT( steem::protocol::signature_data, (c0)(c5)(c6)(e1)(e2)(c)(s1)(s2)(s3))