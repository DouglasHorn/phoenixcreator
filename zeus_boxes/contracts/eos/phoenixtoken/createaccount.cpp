
#include <eosio/action.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/eosio.hpp>
#include <eosio/symbol.hpp>

typedef eosio::name account_name;
typedef uint16_t weight_type;
struct permission_level_weight {
  permission_level permission;
  weight_type weight;
};
struct key_weight {
  public_key key;
  weight_type weight;
};
struct wait_weight {
  uint32_t wait_sec;
  weight_type weight;
};
struct authority {
  uint32_t threshold;
  vector<key_weight> keys;
  vector<permission_level_weight> accounts;
  vector<wait_weight> waits;
};
struct newaccount {
  account_name creator;
  account_name name;
  authority owner;
  authority active;
};
void phoenixtoken::createacc(name account_name, eosio::public_key pubkey) {
  require_auth(get_self());
//   asset stake_cpu(90'000'000, CHAIN_SYMBOL);
//   asset stake_net(10'000'000, CHAIN_SYMBOL);
//   asset buy_ram(100'000'000, CHAIN_SYMBOL);
  asset stake_cpu(1'0000, CHAIN_SYMBOL);
  asset stake_net(1'0000, CHAIN_SYMBOL);
  asset buy_ram(10'0000, CHAIN_SYMBOL);
  check(buy_ram.amount > 0, "Not enough balance to buy ram");

  authority owner_auth =
      authority{.threshold = 1,
                .keys = {key_weight{.key = pubkey, .weight = 1}},
                .accounts = {},
                .waits = {}};
  authority active_auth = owner_auth;

  newaccount new_account = newaccount{.creator = _self,
                                      .name = account_name,
                                      .owner = owner_auth,
                                      .active = active_auth};

  action(permission_level{_self, name("active")}, name("eosio"),
         name("newaccount"), new_account)
      .send();

  action(permission_level{_self, name("active")}, name("eosio"), name("buyram"),
         make_tuple(_self, account_name, buy_ram))
      .send();

  action(permission_level{_self, name("active")}, name("eosio"),
         name("delegatebw"),
         make_tuple(_self, account_name, stake_net, stake_cpu, true))
      .send();
}