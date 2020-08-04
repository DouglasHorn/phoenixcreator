
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
void phoenixtoken::createacc(createacc_payload payload) {
  require_vaccount(payload.vaccount);

  // create a "random" name that is predictable completely but appearing random
  // is good enough as this is not security related at all
  auto mixed = tapos_block_prefix() * tapos_block_num() +
               current_time_point().sec_since_epoch();
  const char *mixedChar = reinterpret_cast<const char *>(&mixed);
  checksum256 hash = sha256(mixedChar, sizeof(mixed));
  std::array<uint8_t, 32U> hash_as_bytes = hash.extract_as_byte_array();
  uint64_t value = 0;
  for (uint8_t i = 0; i < 12; i++) {
    value <<= 5;
    uint8_t charValue = hash_as_bytes[i] % 32;
    // set it to a non '.' character
    if (charValue == 0) {
      // try once again to get a random non '.' character
      charValue = hash_as_bytes[31 - i] % 32;
      if (charValue == 0) {
        charValue = i + 1;
      }
    }
    value |= charValue;
  }
  // for correct encoding this needs to be shifted again
  // https://github.com/EOSIO/eosio.cdt/blob/master/libraries/eosiolib/core/eosio/name.hpp#L91
  value <<= 4;
  name account_name = name(value);
  #ifndef KYLIN
  // substring from 0 of length 4
    std::string phoenix_account_name = account_name.to_string().substr(0, 4) + ".phoenix";
    account_name = name(phoenix_account_name);
  #endif

  //   asset stake_cpu(90'000'000, CHAIN_SYMBOL);
  //   asset stake_net(10'000'000, CHAIN_SYMBOL);
  //   asset buy_ram(100'000'000, CHAIN_SYMBOL);
  asset stake_cpu(1'0000, CHAIN_SYMBOL);
  asset stake_net(1'0000, CHAIN_SYMBOL);
  asset buy_ram(5'0000, CHAIN_SYMBOL);
  check(buy_ram.amount > 0, "Not enough balance to buy ram");

  authority owner_auth =
      authority{.threshold = 1,
                .keys = {key_weight{.key = payload.pubkey, .weight = 1}},
                .accounts = {},
                .waits = {}};
  authority active_auth = owner_auth;

  newaccount new_account = newaccount{.creator = _self,
                                      .name = account_name,
                                      .owner = owner_auth,
                                      .active = active_auth};

  #ifndef KYLIN
  action(permission_level{name("phoenix"), name("active")}, name("eosio"),
         name("newaccount"), new_account)
      .send();
  #else
  action(permission_level{_self, name("active")}, name("eosio"),
         name("newaccount"), new_account)
      .send();
  #endif

  action(permission_level{_self, name("active")}, name("eosio"), name("buyram"),
         make_tuple(_self, account_name, buy_ram))
      .send();

  action(permission_level{_self, name("active")}, name("eosio"),
         name("delegatebw"),
         make_tuple(_self, account_name, stake_net, stake_cpu, true))
      .send();

  phoenix::logcreateacc_action logact(phoenix_account, {get_self(), "active"_n});
  logact.send(payload.vaccount, account_name, payload.pubkey);
}