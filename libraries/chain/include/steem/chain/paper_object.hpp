//增加paper_object和paper_content_object
#pragma once
#include <steem/chain/steem_fwd.hpp>

#include <steem/protocol/authority.hpp>
#include <steem/protocol/steem_operations.hpp>

#include <steem/chain/steem_object_types.hpp>
#include <steem/chain/witness_objects.hpp>


namespace steem { namespace chain {
    using steem::protocol::signature_data;
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
            
            string            author;
            account_name_type account;

            signature_data    signature;
            shared_string     permlink;
            time_point_sec last_update;
            time_point_sec    created;
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
                    member< paper_object, string, &paper_object::author >,
                    member< paper_object, shared_string, &paper_object::permlink >
                >,
                composite_key_compare< std::less< account_name_type >, strcmp_less >
                >
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
        allocator< paper_content_object >
    > paper_content_index;

} } 

FC_REFLECT( steem::chain::paper_object,
             (id)(author)(account)(signature)(permlink)
             (last_update)(created)
        )

CHAINBASE_SET_INDEX_TYPE( steem::chain::paper_object, steem::chain::paper_index )

FC_REFLECT( steem::chain::paper_content_object,
              (id)(paper)(title)(body) )

CHAINBASE_SET_INDEX_TYPE( steem::chain::paper_content_object, steem::chain::paper_content_index )

namespace helpers
{
    using steem::chain::shared_string;

    template <>
    class index_statistic_provider<steem::chain::paper_index>
    {
    public:
        typedef steem::chain::paper_index IndexType;
        
        index_statistic_info gather_statistics(const IndexType& index, bool onlyStaticInfo) const
        {
            index_statistic_info info;
            gather_index_static_data(index, &info);

            if(onlyStaticInfo == false)
            {
                for(const auto& o : index)
                {
                    info._item_additional_allocation += o.permlink.capacity()*sizeof(shared_string::value_type);
                }
            }

            return info;
        }
    };

    template <>
    class index_statistic_provider<steem::chain::paper_content_index>
    {
    public:
        typedef steem::chain::paper_content_index IndexType;

        index_statistic_info gather_statistics(const IndexType& index, bool onlyStaticInfo) const
        {
            index_statistic_info info;
            gather_index_static_data(index, &info);

            if(onlyStaticInfo == false)
            {
                for(const auto& o : index)
                {
                    info._item_additional_allocation += o.title.capacity()*sizeof(shared_string::value_type);
                    info._item_additional_allocation += o.body.capacity()*sizeof(shared_string::value_type);
                }
            }

            return info;
        }
    };
}