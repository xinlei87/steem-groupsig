#include <fc/macros.hpp>

#if defined IS_TEST_NET && defined STEEM_ENABLE_SMT

FC_TODO(Extend testing scenarios to support multiple NAIs per account)

#include <boost/test/unit_test.hpp>

#include <steem/protocol/exceptions.hpp>
#include <steem/protocol/hardfork.hpp>

#include <steem/chain/database.hpp>
#include <steem/chain/database_exceptions.hpp>
#include <steem/chain/steem_objects.hpp>
#include <steem/chain/smt_objects.hpp>

#include "../db_fixture/database_fixture.hpp"

using namespace steem::chain;
using namespace steem::protocol;
using fc::string;
using boost::container::flat_set;

BOOST_FIXTURE_TEST_SUITE( smt_tests, smt_database_fixture )

BOOST_AUTO_TEST_CASE( asset_symbol_validate )
{
   try
   {
      auto check_validate = [&]( const std::string& name, uint8_t decimal_places )
      {
         asset_symbol_type sym = name_to_asset_symbol( name, decimal_places );
         sym.validate();
      };

      // specific cases in https://github.com/steemit/steem/issues/1738
      check_validate( "0", 0 );
      check_validate( "d2", 1 );
      check_validate( "da1", 1 );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( smt_create_validate )
{
   try
   {
      ACTORS( (alice) );

      smt_create_operation op;
      op.control_account = "alice";
      op.smt_creation_fee = ASSET( "1.000 TESTS" );
      op.symbol = get_new_smt_symbol( 3, db );
      op.precision = op.symbol.decimals();
      op.validate();

      // Test invalid control account name.
      op.control_account = "@@@@@";
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );
      op.control_account = "alice";

      // Test invalid creation fee.
      // Negative fee.
      op.smt_creation_fee.amount = -op.smt_creation_fee.amount;
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );
      // Valid MAX_SHARE_SUPPLY
      op.smt_creation_fee.amount = STEEM_MAX_SHARE_SUPPLY;
      op.validate();
      // Invalid MAX_SHARE_SUPPLY+1
      ++op.smt_creation_fee.amount;
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );
      // Invalid currency
      op.smt_creation_fee = ASSET( "1.000000 VESTS" );
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );
      // Valid currency, but doesn't match decimals stored in symbol.
      op.smt_creation_fee = ASSET( "1.000 TESTS" );
      op.precision = 0;
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );
      op.precision = op.symbol.decimals();

      // Test symbol
      // Vesting symbol used instaed of liquid one.
      op.symbol = op.symbol.get_paired_symbol();
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );
      // Legacy symbol used instead of SMT.
      op.symbol = STEEM_SYMBOL;
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( smt_create_authorities )
{
   try
   {
      SMT_SYMBOL( alice, 3 );

      smt_create_operation op;
      op.control_account = "alice";
      op.symbol = alice_symbol;
      op.smt_creation_fee = ASSET( "1.000 TESTS" );

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      expected.insert( "alice" );
      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( smt_create_apply )
{
   try
   {
      ACTORS( (alice)(bob) )

      generate_block();

      FUND( "alice", 10 * 1000 * 1000 );

      set_price_feed( price( ASSET( "1.000 TBD" ), ASSET( "1.000 TESTS" ) ) );

      const dynamic_global_property_object& dgpo = db->get_dynamic_global_properties();
      asset required_creation_fee = dgpo.smt_creation_fee;
      FC_ASSERT( required_creation_fee.amount > 0, "Expected positive smt_creation_fee." );
      unsigned int test_amount = required_creation_fee.amount.value;

      smt_create_operation op;
      op.control_account = "alice";
      op.symbol = get_new_smt_symbol( 3, db );
      op.precision = op.symbol.decimals();

      // Fund with STEEM, and set fee with SBD.
      FUND( "alice", test_amount );
      // Declare fee in SBD/TBD though alice has none.
      op.smt_creation_fee = asset( test_amount, SBD_SYMBOL );
      // Throw due to insufficient balance of SBD/TBD.
      FAIL_WITH_OP(op, alice_private_key, fc::assert_exception);

      // Now fund with SBD, and set fee with STEEM.
      convert( "alice", asset( test_amount, STEEM_SYMBOL ) );
      // Declare fee in STEEM though alice has none.
      op.smt_creation_fee = asset( test_amount, STEEM_SYMBOL );
      // Throw due to insufficient balance of STEEM.
      FAIL_WITH_OP(op, alice_private_key, fc::assert_exception);

      // Push valid operation.
      op.smt_creation_fee = asset( test_amount, SBD_SYMBOL );
      PUSH_OP( op, alice_private_key );

      // Check the SMT cannot be created twice even with different precision.
      create_conflicting_smt(op.symbol, "alice", alice_private_key);

      // Check that another user/account can't be used to create duplicating SMT even with different precision.
      create_conflicting_smt(op.symbol, "bob", bob_private_key);

      // Check that invalid SMT can't be created
      create_invalid_smt("alice", alice_private_key);

      // Check fee set too low.
      asset fee_too_low = required_creation_fee;
      unsigned int too_low_fee_amount = required_creation_fee.amount.value-1;
      fee_too_low.amount -= 1;

      SMT_SYMBOL( bob, 0 );
      op.control_account = "bob";
      op.symbol = bob_symbol;
      op.precision = op.symbol.decimals();

      // Check too low fee in STEEM.
      FUND( "bob", too_low_fee_amount );
      op.smt_creation_fee = asset( too_low_fee_amount, STEEM_SYMBOL );
      FAIL_WITH_OP(op, bob_private_key, fc::assert_exception);

      // Check too low fee in SBD.
      convert( "bob", asset( too_low_fee_amount, STEEM_SYMBOL ) );
      op.smt_creation_fee = asset( too_low_fee_amount, SBD_SYMBOL );
      FAIL_WITH_OP(op, bob_private_key, fc::assert_exception);

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( setup_emissions_validate )
{
   try
   {
      ACTORS( (alice) );
      generate_block();

      asset_symbol_type alice_symbol = create_smt("alice", alice_private_key, 3);

      uint64_t h0 = fc::sha256::hash( "alice" )._hash[0];
      uint32_t h0lo = uint32_t( h0 & 0x7FFFFFF );
      uint32_t an = h0lo % (SMT_MAX_NAI+1);

      FC_UNUSED(an);

      smt_setup_emissions_operation op;
      // Invalid account name.
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.control_account = "alice";
      // schedule_time <= STEEM_GENESIS_TIME;
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      fc::time_point now = fc::time_point::now();
      op.schedule_time = now;
      // Empty emissions_unit.token_unit
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.emissions_unit.token_unit["alice"] = 10;
      // Both absolute amount fields are zero.
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.lep_abs_amount = ASSET( "0.000 TESTS" );
      // Amount symbol does NOT match control account name.
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.lep_abs_amount = asset( 0, alice_symbol );
      // Mismatch of absolute amount symbols.
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.rep_abs_amount = asset( -1, alice_symbol );
      // Negative absolute amount.
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.rep_abs_amount = asset( 0, alice_symbol );
      // Both amounts are equal zero.
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.rep_abs_amount = asset( 1000, alice_symbol );
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( set_setup_parameters_validate )
{
   try
   {
      smt_set_setup_parameters_operation op;
      op.control_account = "####";
      STEEM_REQUIRE_THROW( op.validate(), fc::exception ); // invalid account name

      op.control_account = "dany";
      op.validate();

      op.setup_parameters.emplace(smt_param_allow_vesting());
      op.setup_parameters.emplace(smt_param_allow_voting());
      op.validate();

      op.setup_parameters.emplace(smt_param_allow_vesting({false}));
      op.setup_parameters.emplace(smt_param_allow_voting({false}));
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( setup_emissions_authorities )
{
   try
   {
      SMT_SYMBOL( alice, 3 );

      smt_setup_emissions_operation op;
      op.control_account = "alice";
      fc::time_point now = fc::time_point::now();
      op.schedule_time = now;
      op.emissions_unit.token_unit["alice"] = 10;
      op.lep_abs_amount = op.rep_abs_amount = asset(1000, alice_symbol);

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );
      expected.insert( "alice" );
      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( set_setup_parameters_authorities )
{
   try
   {
      smt_set_setup_parameters_operation op;
      op.control_account = "dany";

      flat_set<account_name_type> auths;
      flat_set<account_name_type> expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      expected.insert( "dany" );
      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( setup_emissions_apply )
{
   try
   {
      ACTORS( (alice)(bob) )

      generate_block();

      smt_setup_emissions_operation fail_op;
      fail_op.control_account = "alice";
      fc::time_point now = fc::time_point::now();
      fail_op.schedule_time = now;
      fail_op.emissions_unit.token_unit["bob"] = 10;

      // Do invalid attempt at SMT creation.
      create_invalid_smt("alice", alice_private_key);      

      // Fail due to non-existing SMT (too early).
      FAIL_WITH_OP(fail_op, alice_private_key, fc::assert_exception)

      // Create SMT(s) and continue.
      auto smts = create_smt_3("alice", alice_private_key);
      {
         const auto& smt1 = smts[0];
         const auto& smt2 = smts[1];

         // Do successful op with one smt.
         smt_setup_emissions_operation valid_op = fail_op;
         valid_op.symbol = smt1;
         valid_op.lep_abs_amount = valid_op.rep_abs_amount = asset( 1000, valid_op.symbol );
         PUSH_OP(valid_op,alice_private_key)

         // Fail with another smt.
         fail_op.symbol = smt2;
         fail_op.lep_abs_amount = fail_op.rep_abs_amount = asset( 1000, fail_op.symbol );
         // TODO: Replace the code below with account setup operation execution once its implemented.
         const steem::chain::smt_token_object* smt = db->find< steem::chain::smt_token_object, by_symbol >( fail_op.symbol );
         FC_ASSERT( smt != nullptr, "The SMT has just been created!" );
         FC_ASSERT( smt->phase < steem::chain::smt_token_object::smt_phase::setup_completed, "Who closed setup phase?!" );
         db->modify( *smt, [&]( steem::chain::smt_token_object& token )
         {
            token.phase = steem::chain::smt_token_object::smt_phase::setup_completed;
         });
         // Fail due to closed setup phase (too late).
         FAIL_WITH_OP(fail_op, alice_private_key, fc::assert_exception)
      }

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( set_setup_parameters_apply )
{
   try
   {
      ACTORS( (dany)(eddy) )
      
      generate_block();

      FUND( "dany", 5000000 );

      set_price_feed( price( ASSET( "1.000 TBD" ), ASSET( "1.000 TESTS" ) ) );
      convert( "dany", ASSET( "5000.000 TESTS" ) );
      
      smt_set_setup_parameters_operation fail_op;
      fail_op.control_account = "dany";

      // Do invalid attempt at SMT creation.
      create_invalid_smt("dany", dany_private_key);
      
      // Fail due to non-existing SMT (too early).
      FAIL_WITH_OP(fail_op, dany_private_key, fc::assert_exception)
      
      // Create SMT(s) and continue.
      auto smts = create_smt_3("dany", dany_private_key);
      {
         const auto& smt1 = smts[0];
         const auto& smt2 = smts[1];

         // "Reset" parameters to default value.
         smt_set_setup_parameters_operation valid_op = fail_op;
         valid_op.symbol = smt1;
         PUSH_OP_TWICE(valid_op, dany_private_key);

         // Fail with wrong key.
         fail_op.symbol = smt2;
         fail_op.setup_parameters.clear();
         fail_op.setup_parameters.emplace( smt_param_allow_vesting() );
         fail_op.setup_parameters.emplace( smt_param_allow_voting() );
         FAIL_WITH_OP(fail_op, eddy_private_key, fc::exception);

         // Set both explicitly to false.
         valid_op.setup_parameters.clear();
         valid_op.setup_parameters.emplace( smt_param_allow_vesting({false}) );
         valid_op.setup_parameters.emplace( smt_param_allow_voting({false}) );
         PUSH_OP(valid_op, dany_private_key);

         // Set one to true and another one to false.
         valid_op.setup_parameters.clear();
         valid_op.setup_parameters.emplace( smt_param_allow_vesting() );
         PUSH_OP(valid_op, dany_private_key);

         // TODO:
         // - check applying smt_set_setup_parameters_operation after setup completed
      }

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( runtime_parameters_windows_validate )
{
   try
   {
      BOOST_REQUIRE( SMT_VESTING_WITHDRAW_INTERVAL_SECONDS > SMT_UPVOTE_LOCKOUT );

      smt_set_runtime_parameters_operation op;

      op.control_account = "{}{}{}{}";
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.control_account = "alice";
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      smt_param_windows_v1 windows;
      windows.reverse_auction_window_seconds = 2;
      windows.cashout_window_seconds = windows.reverse_auction_window_seconds;
      op.runtime_parameters.insert( windows );
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.runtime_parameters.clear();
      windows.reverse_auction_window_seconds = 2;
      windows.cashout_window_seconds = windows.reverse_auction_window_seconds - 1;
      op.runtime_parameters.insert( windows );
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.runtime_parameters.clear();
      windows.reverse_auction_window_seconds = SMT_VESTING_WITHDRAW_INTERVAL_SECONDS;
      windows.cashout_window_seconds = windows.reverse_auction_window_seconds + 1;
      op.runtime_parameters.insert( windows );
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.runtime_parameters.clear();
      windows.reverse_auction_window_seconds = 1;
      windows.cashout_window_seconds = SMT_VESTING_WITHDRAW_INTERVAL_SECONDS;
      op.runtime_parameters.insert( windows );
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.runtime_parameters.clear();
      windows.reverse_auction_window_seconds = 0;
      windows.cashout_window_seconds = SMT_UPVOTE_LOCKOUT;
      op.runtime_parameters.insert( windows );
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.runtime_parameters.clear();
      windows.reverse_auction_window_seconds = 0;
      windows.cashout_window_seconds = windows.reverse_auction_window_seconds + SMT_UPVOTE_LOCKOUT + 1;
      op.runtime_parameters.insert( windows );
      op.validate();

      op.runtime_parameters.clear();
      windows.reverse_auction_window_seconds = 0;
      windows.cashout_window_seconds = SMT_VESTING_WITHDRAW_INTERVAL_SECONDS - 1;
      op.runtime_parameters.insert( windows );
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( runtime_parameters_regeneration_period_validate )
{
   try
   {
      smt_set_runtime_parameters_operation op;
      op.control_account = "alice";

      smt_param_vote_regeneration_period_seconds_v1 regeneration;

      regeneration.vote_regeneration_period_seconds = SMT_VESTING_WITHDRAW_INTERVAL_SECONDS;
      op.runtime_parameters.insert( regeneration );
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.runtime_parameters.clear();
      regeneration.vote_regeneration_period_seconds = 0;
      op.runtime_parameters.insert( regeneration );
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );

      op.runtime_parameters.clear();
      regeneration.vote_regeneration_period_seconds = SMT_VESTING_WITHDRAW_INTERVAL_SECONDS - 1;
      op.runtime_parameters.insert( regeneration );
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( runtime_parameters_authorities )
{
   try
   {
      smt_set_runtime_parameters_operation op;
      op.control_account = "alice";

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      expected.insert( "alice" );
      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( runtime_parameters_apply )
{
   try
   {
      ACTORS( (alice) )

      generate_block();

      set_price_feed( price( ASSET( "1.000 TBD" ), ASSET( "1.000 TESTS" ) ) );

      smt_set_runtime_parameters_operation op;

      op.control_account = "alice";

      smt_param_windows_v1 windows;
      windows.reverse_auction_window_seconds = 0;
      windows.cashout_window_seconds = windows.reverse_auction_window_seconds + SMT_UPVOTE_LOCKOUT + 1;

      smt_param_vote_regeneration_period_seconds_v1 regeneration;
      regeneration.vote_regeneration_period_seconds = SMT_VESTING_WITHDRAW_INTERVAL_SECONDS / 2;
      regeneration.votes_per_regeneration_period = 1;

      smt_param_rewards_v1 rewards;
      rewards.content_constant = 1;
      rewards.percent_curation_rewards = 2;
      rewards.percent_content_rewards = 3;

      op.runtime_parameters.insert( windows );
      op.runtime_parameters.insert( regeneration );
      op.runtime_parameters.insert( rewards );

      //First we should create SMT
      FAIL_WITH_OP(op, alice_private_key, fc::assert_exception)

      // Create SMT(s) and continue.
      auto smts = create_smt_3("alice", alice_private_key);
      {
         //Make transaction again.
         op.symbol = smts[2];
         PUSH_OP(op, alice_private_key);
      }

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( smt_setup_validate )
{
   try
   {
      smt_setup_operation op;
      fc::time_point_sec start_time        = fc::variant( "2021-01-01T00:00:00" ).as< fc::time_point_sec >();
      fc::time_point_sec start_time_plus_1 = start_time + fc::seconds(1);
      // Do minimal operation setup that allows successful validatation.
      {
         smt_capped_generation_policy gpolicy;
         uint64_t max_supply = STEEM_MAX_SHARE_SUPPLY / 6000;
         // set steem unit, total is 100 STEEM-satoshis = 0.1 STEEM
         gpolicy.pre_soft_cap_unit.steem_unit.emplace( "founder", 100 );
         // set token unit, total is 5 token-satoshis = 0.0005 SMT
         gpolicy.pre_soft_cap_unit.token_unit.emplace( "$from", 5 );
         // Note - no soft cap -> no soft cap unit
         gpolicy.min_steem_units_commitment.fillin_nonhidden_value( 1 );
         gpolicy.hard_cap_steem_units_commitment.fillin_nonhidden_value( max_supply );
         gpolicy.soft_cap_percent = STEEM_100_PERCENT;

         // Note that neither tested SMT nor even its creator is necessary to validate this operation.
         op.control_account = "alice";
         op.decimal_places = 4;
         op.initial_generation_policy = gpolicy;
         op.generation_begin_time = start_time;
         op.generation_end_time = op.announced_launch_time = op.launch_expiration_time = start_time_plus_1;
         op.smt_creation_fee = asset( 1000000, SBD_SYMBOL );
      }

      op.validate();
      // TODO put other negative scenarios here.

      // Launch expiration time can't be earlier than announced launch time.
      op.launch_expiration_time = start_time;
      STEEM_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.launch_expiration_time = start_time_plus_1; // Restored valid value.
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( smt_refund_validate )
{
   try
   {
      ACTORS( (creator) )
      generate_block();
      asset_symbol_type creator_symbol = create_smt("creator", creator_private_key, 0);

      smt_refund_operation op;
      op.executor = "executor";
      op.contributor = "contributor";
      op.contribution_id = 0;
      op.smt = creator_symbol;
      op.amount = ASSET( "1.000 TESTS" );
      op.validate();

      op.executor = "@@@@@";
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );
      op.executor = "executor";

      op.contributor = "@@@@@";
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );
      op.contributor = "contributor";

      op.smt = op.amount.symbol;
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );
      op.smt = creator_symbol;

      op.amount = asset( 1, creator_symbol );
      STEEM_REQUIRE_THROW( op.validate(), fc::exception );
      op.amount = ASSET( "1.000 TESTS" );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( smt_refund_authorities )
{
   try
   {
      smt_refund_operation op;
      op.executor = "executor";

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      expected.insert( "executor" );
      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( smt_transfer_validate )
{
   try
   {
      ACTORS( (alice) )

      generate_block();

      asset_symbol_type alice_symbol = create_smt("alice", alice_private_key, 0);

      transfer_operation op;
      op.from = "alice";
      op.to = "bob";
      op.amount = asset(100, alice_symbol);
      op.validate();

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( smt_transfer_apply )
{
   // This simple test touches SMT account balance objects, related functions (get/adjust)
   // and transfer operation that builds on them.
   try
   {
      ACTORS( (alice)(bob) )

      generate_block();

      // Create SMT.
      asset_symbol_type alice_symbol = create_smt("alice", alice_private_key, 0);
      asset_symbol_type bob_symbol = create_smt("bob", bob_private_key, 1);

      // Give some SMT to creators.
      FUND( "alice", asset( 100, alice_symbol ) );
      FUND( "bob", asset( 110, bob_symbol ) );

      // Check pre-tranfer amounts.
      FC_ASSERT( db->get_balance( "alice", alice_symbol ).amount == 100, "SMT balance adjusting error" );
      FC_ASSERT( db->get_balance( "alice", bob_symbol ).amount == 0, "SMT balance adjusting error" );
      FC_ASSERT( db->get_balance( "bob", alice_symbol ).amount == 0, "SMT balance adjusting error" );
      FC_ASSERT( db->get_balance( "bob", bob_symbol ).amount == 110, "SMT balance adjusting error" );

      // Transfer SMT.
      transfer( "alice", "bob", asset(20, alice_symbol) );
      transfer( "bob", "alice", asset(50, bob_symbol) );

      // Check transfer outcome.
      FC_ASSERT( db->get_balance( "alice", alice_symbol ).amount == 80, "SMT transfer error" );
      FC_ASSERT( db->get_balance( "alice", bob_symbol ).amount == 50, "SMT transfer error" );
      FC_ASSERT( db->get_balance( "bob", alice_symbol ).amount == 20, "SMT transfer error" );
      FC_ASSERT( db->get_balance( "bob", bob_symbol ).amount == 60, "SMT transfer error" );

      validate_database();
   }
   FC_LOG_AND_RETHROW()   
}

BOOST_AUTO_TEST_CASE( comment_votable_assers_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Test Comment Votable Assets Validate" );
      ACTORS((alice));

      generate_block();

      std::array<asset_symbol_type, SMT_MAX_VOTABLE_ASSETS + 1> smts;
      /// Create one more than limit to test negative cases
      for(size_t i = 0; i < SMT_MAX_VOTABLE_ASSETS + 1; ++i)
      {
         asset_symbol_type smt = create_smt("alice", alice_private_key, 0);
         smts[i] = std::move(smt);
      }

      {
         comment_options_operation op;

         op.author = "alice";
         op.permlink = "test";
            
         BOOST_TEST_MESSAGE( "--- Testing valid configuration: no votable_assets" );
         allowed_vote_assets ava;
         op.extensions.insert( ava );
         op.validate();
      }

      {
         comment_options_operation op;

         op.author = "alice";
         op.permlink = "test";
            
         BOOST_TEST_MESSAGE( "--- Testing valid configuration of votable_assets" );
         allowed_vote_assets ava;
         for(size_t i = 0; i < SMT_MAX_VOTABLE_ASSETS; ++i)
         {
            const auto& smt = smts[i];
            ava.add_votable_asset(smt, share_type(10 + i), (i & 2) != 0);
         }
         
         op.extensions.insert( ava );
         op.validate();
      }

      {
         comment_options_operation op;

         op.author = "alice";
         op.permlink = "test";
            
         BOOST_TEST_MESSAGE( "--- Testing invalid configuration of votable_assets - too much assets specified" );
         allowed_vote_assets ava;
         for(size_t i = 0; i < smts.size(); ++i)
         {
            const auto& smt = smts[i];
            ava.add_votable_asset(smt, share_type(20 + i), (i & 2) != 0);
         }
         
         op.extensions.insert( ava );
         STEEM_REQUIRE_THROW( op.validate(), fc::assert_exception );
      }

      {
         comment_options_operation op;

         op.author = "alice";
         op.permlink = "test";
            
         BOOST_TEST_MESSAGE( "--- Testing invalid configuration of votable_assets - STEEM added to container" );
         allowed_vote_assets ava;
         const auto& smt = smts.front();
         ava.add_votable_asset(smt, share_type(20), false);
         ava.add_votable_asset(STEEM_SYMBOL, share_type(20), true);
         op.extensions.insert( ava );
         STEEM_REQUIRE_THROW( op.validate(), fc::assert_exception );
      }
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( asset_symbol_vesting_methods )
{
   try
   {
      BOOST_TEST_MESSAGE( "Test asset_symbol vesting methods" );

      asset_symbol_type Steem = STEEM_SYMBOL;
      FC_ASSERT( Steem.is_vesting() == false );
      FC_ASSERT( Steem.get_paired_symbol() == VESTS_SYMBOL );

      asset_symbol_type Vests = VESTS_SYMBOL;
      FC_ASSERT( Vests.is_vesting() );
      FC_ASSERT( Vests.get_paired_symbol() == STEEM_SYMBOL );

      asset_symbol_type Sbd = SBD_SYMBOL;
      FC_ASSERT( Sbd.is_vesting() == false );
      FC_ASSERT( Sbd.get_paired_symbol() == SBD_SYMBOL );

      ACTORS( (alice) )
      generate_block();
      auto smts = create_smt_3("alice", alice_private_key);
      {
         for( const asset_symbol_type& liquid_smt : smts )
         {
            FC_ASSERT( liquid_smt.is_vesting() == false );
            auto vesting_smt = liquid_smt.get_paired_symbol();
            FC_ASSERT( vesting_smt != liquid_smt );
            FC_ASSERT( vesting_smt.is_vesting() );
            FC_ASSERT( vesting_smt.get_paired_symbol() == liquid_smt );
         }
      }
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( vesting_smt_creation )
{
   try
   {
      BOOST_TEST_MESSAGE( "Test Creation of vesting SMT" );
      
      ACTORS((alice));
      generate_block();

      asset_symbol_type liquid_symbol = create_smt("alice", alice_private_key, 6);
      // Use liquid symbol/NAI to confirm smt object was created.
      auto liquid_object_by_symbol = db->find< smt_token_object, by_symbol >( liquid_symbol );
      FC_ASSERT( ( liquid_object_by_symbol != nullptr ) );
      auto liquid_object_by_nai = db->find< smt_token_object, by_nai >( liquid_symbol.to_nai() );
      FC_ASSERT( ( liquid_object_by_nai != nullptr ) );
      FC_ASSERT( ( liquid_object_by_symbol == liquid_object_by_nai ) );

      asset_symbol_type vesting_symbol = liquid_symbol.get_paired_symbol();
      // Use vesting symbol/NAI to confirm smt object was created.
      auto vesting_object_by_symbol = db->find< smt_token_object, by_symbol >( vesting_symbol );
      FC_ASSERT( ( vesting_object_by_symbol != nullptr ) );
      auto vesting_object_by_nai = db->find< smt_token_object, by_nai >( vesting_symbol.to_nai() );
      FC_ASSERT( ( vesting_object_by_nai != nullptr ) );
      FC_ASSERT( ( vesting_object_by_symbol == vesting_object_by_nai ) );

      // Check that liquid and vesting objecta are the same one.
      FC_ASSERT( ( liquid_object_by_symbol == vesting_object_by_symbol ) );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()
#endif