#pragma once

#define __TEST__
#define USE_ADVANCED_IPFS
// #define USE_IPFS_WARMUPROW

#include "../dappservices/advanced_multi_index.hpp"
#include "../dappservices/cron.hpp"
#include "../dappservices/multi_index.hpp"
#include "../dappservices/vaccounts.hpp"
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>

// #define DAPPSERVICES_ACTIONS()                                                 \
//   XSIGNAL_DAPPSERVICE_ACTION                                                   \
//   CRON_DAPPSERVICE_ACTIONS                                                     \
//   IPFS_DAPPSERVICE_ACTIONS                                                     \
//   VACCOUNTS_DAPPSERVICE_ACTIONS

// #define DAPPSERVICE_ACTIONS_COMMANDS()                                         \
//   CRON_SVC_COMMANDS()                                                          \
//   IPFS_SVC_COMMANDS()                                                          \
//   VACCOUNTS_SVC_COMMANDS()
// #define CONTRACT_NAME() phoenix

using std::string;

using namespace std;
using namespace eosio;

namespace phoenix {
/**
 * User
 */
struct user_profile_info {
  std::string displayName;
  std::string logoSrc;
  std::string headerSrc;
  std::string description;
  std::string website;
  std::map<name, std::string> social;
  bool explicit_content = false;
};
struct pledge_tier {
  std::string title;
  std::string description;
  std::vector<std::string> benefits;
  float usd_value;
};

struct [[eosio::table("")]] user_info {
  name username;
  name linked_name = ""_n;     // account of EOS name
  name created_account = ""_n; // free WAX poenix account created by user
  user_profile_info profile_info;
  std::vector<pledge_tier> tiers;
  // as there's no support for secondary indexes on vRAM, we need to keep the
  // "foreign key" relationships in the user
  std::vector<uint64_t> post_indexes = std::vector<uint64_t>{};

  auto primary_key() const { return username.value; }
};

typedef dapp::multi_index<name("users"), user_info> users_table;
typedef eosio::multi_index<".users"_n, user_info> users_table_v_abi;
TABLE shardbucket {
  std::vector<char> shard_uri;
  uint64_t shard;
  uint64_t primary_key() const { return shard; }
};
typedef eosio::multi_index<"users"_n, shardbucket> users_table_abi;

} // namespace phoenix