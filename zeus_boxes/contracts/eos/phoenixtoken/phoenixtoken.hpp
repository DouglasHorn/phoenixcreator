#pragma once

#define __TEST__
#define USE_ADVANCED_IPFS
// #define USE_IPFS_WARMUPROW

#define VACCOUNTS_DELAYED_CLEANUP 0

#include "../dappservices/advanced_multi_index.hpp"
#include "../dappservices/multi_index.hpp"
// #include "../dappservices/vaccounts.hpp"
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include "../phoenix/constants.hpp"

#define DAPPSERVICES_ACTIONS()                                                 \
  XSIGNAL_DAPPSERVICE_ACTION                                                   \
  IPFS_DAPPSERVICE_ACTIONS

#define DAPPSERVICE_ACTIONS_COMMANDS() IPFS_SVC_COMMANDS()

#define CONTRACT_NAME() phoenixtoken

using std::string;

using namespace std;
using namespace eosio;

CONTRACT_START()
public:
TABLE globals {
  std::vector<uint64_t> latest_post_indexes = std::vector<uint64_t>{};
  uint64_t next_post_id = 0;
  bool paused = false;
};
// just so it is added to the ABI, as singletons are currently not
typedef eosio::multi_index<"globals"_n, globals> globals_t_abi;
typedef eosio::singleton<"globals"_n, globals> globals_sgt;
globals_sgt _globals;

struct transferv_payload {
  name from;
  name to;
  asset quantity;
  string memo;
  EOSLIB_SERIALIZE(transferv_payload, (from)(to)(quantity)(memo))
};

ACTION create(name issuer, asset maximum_supply);
ACTION issue(name to, asset quantity, string memo);
// requires _self auth
ACTION transfer(name from, name to, asset quantity, string memo);
// requires vaccount auth
ACTION transferv(transferv_payload payload);
ACTION open(const name &owner, const symbol &symbol);
// ACTION withdraw(const name& owner, const symbol& symbol);
void on_transfer(const eosio::name& from, const eosio::name& to, const eosio::asset& quantity, const std::string& memo);

inline asset get_supply(symbol_code sym) const;
inline asset get_balance(name owner, symbol sym) const;

using create_action = eosio::action_wrapper<"create"_n, &phoenixtoken::create>;
using issue_action = eosio::action_wrapper<"issue"_n, &phoenixtoken::issue>;
using transfer_action =
    eosio::action_wrapper<"transfer"_n, &phoenixtoken::transfer>;
using open_action = eosio::action_wrapper<"open"_n, &phoenixtoken::open>;

private:
TABLE account {
  asset balance;
  uint64_t primary_key() const { return balance.symbol.code().raw(); }
  EOSLIB_SERIALIZE(account, (balance))
};

TABLE currency_stats {
  asset supply;
  asset max_supply;
  name issuer;

  uint64_t primary_key() const { return supply.symbol.code().raw(); }
};

typedef dapp::multi_index<"accounts"_n, account> accounts_t;
typedef eosio::multi_index<".accounts"_n, account> accounts_t_v_abi;
typedef eosio::multi_index<"accounts"_n, dapp::shardbucket> accounts_t_abi;

typedef eosio::multi_index<"stat"_n, currency_stats> stats;

void sub_balance(name owner, asset value);
void add_balance(name owner, asset value);
void _transfer(const name &from, const name &to, const asset &quantity);
void check_user(const name &name);
void check_running();

public:
phoenixtoken(name receiver, name code, datastream<const char *> ds)
    : contract(receiver, code, ds), _globals(receiver, receiver.value) {}

private:
globals get_globals() {
  globals g = _globals.get_or_default(globals());
  return g;
}

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

struct [[eosio::table]] user_info {
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

}
;