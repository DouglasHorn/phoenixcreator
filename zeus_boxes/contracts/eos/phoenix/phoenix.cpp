#include "phoenix.hpp"

// #include "eosio_token.hpp"
#include "constants.hpp"
#include "helpers.hpp"
#include "phoenixtoken-interface.hpp"

#include "main.cpp"
#include "pledges.cpp"

#ifdef __TEST__
template <eosio::name::raw A, typename B, typename... C>
inline void clear_table(multi_index<A, B, C...> &table) {
  auto it = table.begin();
  while (it != table.end()) {
    it = table.erase(it);
  }
}
void phoenix::testreset(uint64_t count) {
  require_auth(get_self());

  auto user = _users.find(name("tester1.phnx").value);
  if (user != _users.end()) {
    _users.erase(user);
    print("\n", "tester1.phnx deleted", "\n");
  } else {
    print("\n", "tester1.phnx didn't exist", "\n");
  }

  user = _users.find(name("tester2.phnx").value);
  if (user != _users.end()) {
    _users.erase(user);
    print("\n", "tester2.phnx deleted", "\n");
  } else {
    print("\n", "tester2.phnx didn't exist", "\n");
  }

    vkeys_t vkeys_table(get_self(), get_self().value, 1024, 64,
                      VACCOUNTS_SHARD_PINNING, false,
                      VACCOUNTS_DELAYED_CLEANUP);
  auto vkey = vkeys_table.find(name("tester1.phnx").value);
  if(vkey != vkeys_table.end()) {
    vkeys_table.erase(vkey);
  }
  vkey = vkeys_table.find(name("tester2.phnx").value);
  if(vkey != vkeys_table.end()) {
    vkeys_table.erase(vkey);
  }
  // {
  //   auto raw_table = users_table_abi(get_self(), get_self().value);
  //   clear_table(raw_table);
  //   _users.clear();
  // }

  // {
  //   auto raw_table = posts_table_abi(get_self(), get_self().value);
  //   clear_table(raw_table);
  //   _posts.clear();
  // }

  // {
  //   auto raw_table = post_key_enc_table_abi(get_self(), get_self().value);
  //   clear_table(raw_table);
  //   post_key_enc_table _posts_keys(get_self(), dsp_name.value, 1024, 64,
  //   false,
  //                                  false, VACCOUNTS_DELAYED_CLEANUP);
  //   _posts_keys.clear();
  // }

  // {
  //   auto raw_table = follows_table_abi(get_self(), name("from").value);
  //   clear_table(raw_table);
  //   follows_table _follows_from(get_self(), name("from").value, 1024, 64,
  //   false, false,
  //           VACCOUNTS_DELAYED_CLEANUP);
  //   _follows_from.clear();
  // }

  // {
  //   auto raw_table = follows_table_abi(get_self(), name("to").value);
  //   clear_table(raw_table);
  //   follows_table _follows_to(get_self(), name("to").value, 1024, 64, false,
  //   false,
  //           VACCOUNTS_DELAYED_CLEANUP);
  //   _follows_to.clear();
  // }

  // {
  //   auto raw_table = pledges_table_abi(get_self(), get_self().value);
  //   clear_table(raw_table);
  //   _pledges.clear();
  // }

  // {
  //   auto raw_table = pledges_rel_table_abi(get_self(), name("from").value);
  //   clear_table(raw_table);
  //   pledges_rel_table _pledges_from(get_self(), name("from").value, 1024, 64,
  //   false, false,
  //              VACCOUNTS_DELAYED_CLEANUP);
  //   _pledges_from.clear();
  // }

  // {
  //   auto raw_table = pledges_rel_table_abi(get_self(), name("to").value);
  //   clear_table(raw_table);
  //   pledges_rel_table _pledges_to(get_self(), name("to").value, 1024, 64,
  //   false, false,
  //              VACCOUNTS_DELAYED_CLEANUP);
  //   _pledges_to.clear();
  // }

  // {
  //   auto raw_table = ipfsentries_t(get_self(), get_self().value);
  //   clear_table(raw_table);
  // }

  // {
  //   auto raw_table = vkeys_t_abi(get_self(), get_self().value);
  //   clear_table(raw_table);
  // }

  // _globals.remove();
}
#endif

// EOSIO_DISPATCH_SVC_TRX(CONTRACT_NAME(), (login)(renewpledge))
extern "C" {
void apply(uint64_t receiver, uint64_t code, uint64_t action) {
  if (code == WEOSDT_EXT_SYMBOL.get_contract().value &&
      action == "transfer"_n.value) {
    eosio::execute_action(eosio::name(receiver), eosio::name(code),
                          &CONTRACT_NAME()::on_transfer);
  } else if (receiver == code) {
    switch (action) {
      EOSIO_DISPATCH_HELPER(CONTRACT_NAME(), DAPPSERVICE_ACTIONS_COMMANDS())
      EOSIO_DISPATCH_HELPER(
          CONTRACT_NAME(),
          (init)(setfeatured)(pause)(renewpledge)(signup)(login)(logcreateacc))
#ifdef __TEST__
      EOSIO_DISPATCH_HELPER(CONTRACT_NAME(), (testreset))
#endif
      EOSIO_DISPATCH_HELPER(CONTRACT_NAME(), (xdcommit)(xvinit)(xvauth))
      EOSIO_DISPATCH_HELPER(CONTRACT_NAME(), (xsignal))
    default:
      check(false, "unrecognized internal action: " + name(action).to_string());
    }
  }
  eosio_exit(0);
}
}