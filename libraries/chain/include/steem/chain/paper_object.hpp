#pragma once
#include <steem/chain/steem_fwd.hpp>

#include <steem/protocol/authority.hpp>
#include <steem/protocol/steem_operations.hpp>

#include <steem/chain/steem_object_types.hpp>
#include <steem/chain/witness_objects.hpp>


namespace steem { namespace chain {

    using protocol::beneficiary_route_type;
    using chainbase::t_vector;
    using chainbase::t_pair;

    struct strcmp_less
    {
        bool operator()( const shared_string& a, const shared_string& b )const
        {
            return less( a.c_str(), b.c_str() );
        }

#ifndef ENABLE_MIRA
        bool operator()( const shared_string& a, const string& b )const
        {
            return less( a.c_str(), b.c_str() );
        }

        bool operator()( const string& a, const shared_string& b )const
        {
            return less( a.c_str(), b.c_str() );
        }
#endif

        private:
            inline bool less( const char* a, const char* b )const
            {
                return std::strcmp( a, b ) < 0;
            }
    };

    class paper_object : public object < paper_object_type, paper_object >
    {
        STEEM_STD_ALLOCATOR_CONSTRUCTOR( paper_object )

        public:
            template< typename Constructor, typename Allocator >
            paper_object( Constructor&& c, allocator< Allocator> a )
                :permlink( a )
            {
                c( *this );
            }

            id_type           id;
            
            account_name_type author;
            //重新定义
            signature_type    sign;
            shared_string     permlink;

            time_point_sec    last_update;
            time_point_sec    created;
            time_point_sec    active;       
    };

    class paper_content_object : public object< paper_content_object_type, paper_content_object >
    {
        STEEM_STD_ALLOCATOR_CONSTRUCTOR( paper_content_object )

        public:
            template< typename Constructor, typename Allocator >
            paper_content_object( Constructor&& c, allocator< Allocator > a ) :
                title( a ), body( a )
            {
                c( *this);
            }

            id_type           id;
            
            paper_id_type     paper;

            shared_string     title;
            shared_string     body;
    };

    struct by_permlink;
    struct by_last_update;

    typedef multi_index_container<
        paper_object,
        indexed_by<
            ordered_unique< tag< by_id >, member< paper_object, paper_id_type, &paper_object::id> >,
            ordered_unique< tag< by_permlink >,
                composite_key< paper_object,
                    member< paper_object, account_name_type, &paper_object::author >,
                    member< paper_object, shared_string, &paper_object::permlink >
                >,
                composite_key_compare< std::less< account_name_type >, strcmp_less >
                >
            >
            
#ifndef IS_LOW_MEM
        ,
        ordered_unique< tag< by_last_update >,
            composite_key< paper_object,
                member< paper_object, account_name_type, &paper_object::author >,
                member< paper_object, time_point_sec, &paper_object::last_update >,
                member< paper_object, paper_id_type, &paper_object::id>
            >,
            composite_key_compare< std::less< account_name_type >, std::greater< time_point_sec >, std::less< paper_id_type > >
        >
#endif
    >,
    allocator< paper_object >
> paper_index;

    struct by_paper;

    typedef multi_index_container<
        paper_content_object,
        indexed_by<
            ordered_unique< tag< by_id >, member< paper_content_object, paper_content_id_type, &paper_content_object::id > >,
            ordered_unique< tag< by_paper >, member< paper_content_object, paper_id_type, &paper_content_object::paper > >
        >,
        allocator< paper_object >
    > paper_content_index;

} } 

FC_REFLECT( steem::chain::paper_object,
             (id)(author)(sign)(permlink)
             (last_update)(created)(active)
        )

CHAINBASE_SET_INDEX_TYPE( steem::chain::paper_object, steem::chain::paper_index )

FC_REFLECT( steem::chain::paper_content_object,
              (id)(paper)(title)(body) )

CHAINBASE_SET_INDEX_TYPE( steem::chain::paper_content_object, steem::chain::paper_content_index )

