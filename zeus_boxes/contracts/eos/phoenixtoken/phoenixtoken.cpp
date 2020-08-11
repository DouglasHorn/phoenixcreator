#include "phoenixtoken.hpp"
#include "phoenix-interface.hpp"

using namespace eosio;

#include "createaccount.cpp"

void phoenixtoken::create(name issuer, asset maximum_supply) {
  require_auth(get_self());

  auto sym = maximum_supply.symbol;
  eosio::check(sym.is_valid(), "invalid symbol name");
  eosio::check(maximum_supply.is_valid(), "invalid supply");
  eosio::check(maximum_supply.amount > 0, "max-supply must be positive");

  stats statstable(_self, sym.code().raw());
  auto existing = statstable.find(sym.code().raw());
  eosio::check(existing == statstable.end(),
               "token with symbol already exists");

  statstable.emplace(_self, [&](auto &s) {
    s.supply.symbol = maximum_supply.symbol;
    s.max_supply = maximum_supply;
    s.issuer = issuer;
  });
}

void phoenixtoken::issue(name to, asset quantity, string memo) {
  check_running();
  auto sym = quantity.symbol;
  eosio::check(sym.is_valid(), "invalid symbol name");
  eosio::check(memo.size() <= 256, "memo has more than 256 bytes");

  auto sym_name = sym.code().raw();
  stats statstable(_self, sym_name);
  auto existing = statstable.find(sym_name);
  eosio::check(existing != statstable.end(),
               "token with symbol does not exist, create token before issue");
  const auto &st = *existing;

  require_auth(st.issuer);
  eosio::check(quantity.is_valid(), "invalid quantity");
  eosio::check(quantity.amount > 0, "must issue positive quantity");

  eosio::check(quantity.symbol == st.supply.symbol,
               "symbol precision mismatch");
  eosio::check(quantity.amount <= st.max_supply.amount - st.supply.amount,
               "quantity exceeds available supply");

  statstable.modify(st, eosio::same_payer,
                    [&](auto &s) { s.supply += quantity; });

  // special check here because the EOS account => vAccount mapping
  // makes it impossible to know for any other issuer
  eosio::check(st.issuer == get_self(), "only phoenix account may issue");
  eosio::check(to == PHOENIX_VACCOUNT,
               "only phoenix vaccount may receive new supply");
  // check if phoenix vaccount exists
  phoenixtoken::check_user(PHOENIX_VACCOUNT);
  add_balance(to, quantity);
}

void phoenixtoken::transfer(name from, name to, asset quantity, string memo) {
  check_running();
  // invoked through transferv as an inline action from vaccounts
  // or from phoenix::pay_pledge as an inline action
  check(has_auth(get_self()) || has_auth(phoenix_account),
        "use the transferv action for transfers from vaccounts");
  eosio::check(from != to, "cannot transfer to self");
  phoenixtoken::check_user(from);
  phoenixtoken::check_user(to);

  eosio::check(memo.size() <= 256, "memo has more than 256 bytes");

  _transfer(from, to, quantity);
}

void phoenixtoken::open(const name &owner, const symbol &symbol) {
  check_running();
  // we want to open it in signup action and do not have vaccount auth
  require_auth(phoenix_account);
  // if(!has_auth(get_self())) {
  //   require_vaccount(owner);
  // }

  phoenixtoken::check_user(owner);

  auto sym_code_raw = symbol.code().raw();
  stats statstable(get_self(), sym_code_raw);
  const auto &st = statstable.get(sym_code_raw, "symbol does not exist");
  check(st.supply.symbol == symbol, "symbol precision mismatch");

  accounts_t acnts(
      get_self(),  // contract
      owner.value, // scope
      1024,        // optional: shards per table
      64,          // optional: buckets per shard
      false, // optional: pin shards in RAM - (buckets per shard) X (shards
             // per table) X 32B - 2MB in this example
      false, // optional: pin buckets in RAM - keeps most of the data in RAM.
             // should be evicted manually after the process
      VACCOUNTS_DELAYED_CLEANUP);

  auto it = acnts.find(sym_code_raw);
  if (it == acnts.end()) {
    acnts.emplace(get_self(), [&](auto &a) { a.balance = asset{0, symbol}; });
  }
}

// withdraw token from vaccount to EOSIO account
void phoenixtoken::withdrawv(withdrawv_payload payload) {
  check_running();
  require_vaccount(payload.vaccount);

  check_user(payload.vaccount);
  check(is_account(payload.to_eos_account),
        "withdrawal eos account does not exist");

  check(payload.quantity.symbol.is_valid(), "invalid quantity");
  check(payload.quantity.symbol == WEOSDT_EXT_SYMBOL.get_symbol(),
        "invalid token symbol");
  check(payload.quantity.amount > 0, "amount must be positive");

  // burn: update virtual token balance: "from" -> "phoenix"
  _transfer(payload.vaccount, PHOENIX_VACCOUNT, payload.quantity);
  transfer_action withdraw_action(WEOSDT_EXT_SYMBOL.get_contract(),
                                  {get_self(), "active"_n});
  // send real token "self" -> "to"
  withdraw_action.send(get_self(), payload.to_eos_account, payload.quantity,
                       "Phoenix Withdraw");
}

void phoenixtoken::payoutfees(name to) {
  check_running();
  require_auth(get_self());

  check_user(PHOENIX_FEES_VACCOUNT);
  check(is_account(to), "withdrawal eos account does not exist");

  auto fees =
      get_balance(PHOENIX_FEES_VACCOUNT, WEOSDT_EXT_SYMBOL.get_symbol());
  // burn: update virtual token balance: "from" -> "phoenix"
  _transfer(PHOENIX_FEES_VACCOUNT, PHOENIX_VACCOUNT, fees);
  transfer_action withdraw_action(WEOSDT_EXT_SYMBOL.get_contract(),
                                  {get_self(), "active"_n});
  // send real token "self" -> "to"
  withdraw_action.send(get_self(), to, fees, "fees payout");
}

void phoenixtoken::_transfer(const name &from, const name &to,
                             const asset &quantity) {
  auto sym = quantity.symbol.code().raw();
  stats statstable(_self, sym);
  const auto &st = statstable.get(sym);

  // require_recipient( from );
  // require_recipient( to );

  eosio::check(quantity.is_valid(), "invalid quantity");
  eosio::check(quantity.amount > 0, "must transfer positive quantity");
  eosio::check(quantity.symbol == st.supply.symbol,
               "symbol precision mismatch");
  sub_balance(from, quantity);
  add_balance(to, quantity);
}

void phoenixtoken::transferv(transferv_payload payload) {
  check_running();
  require_vaccount(payload.vaccount);

  phoenixtoken::transfer_action transfer(get_self(), {get_self(), "active"_n});
  transfer.send(payload.vaccount, payload.to, payload.quantity, payload.memo);
}

void phoenixtoken::add_balance(name owner, asset value) {
  accounts_t to_acnts(
      get_self(),  // contract
      owner.value, // scope
      1024,        // optional: shards per table
      64,          // optional: buckets per shard
      false, // optional: pin shards in RAM - (buckets per shard) X (shards
             // per table) X 32B - 2MB in this example
      false, // optional: pin buckets in RAM - keeps most of the data in RAM.
             // should be evicted manually after the process
      VACCOUNTS_DELAYED_CLEANUP);
  const auto &to = to_acnts.find(value.symbol.code().raw());
  if (to == to_acnts.end()) {
    to_acnts.emplace(get_self(), [&](auto &a) { a.balance = value; });
  } else {
    to_acnts.modify(to, get_self(), [&](auto &a) { a.balance += value; });
  }
}

void phoenixtoken::sub_balance(name owner, asset value) {
  accounts_t from_acnts(get_self(), owner.value, 1024, 64, false, false,
                        VACCOUNTS_DELAYED_CLEANUP);
  const auto &from =
      from_acnts.get(value.symbol.code().raw(), "no balance object found");
  eosio::check(from.balance.amount >= value.amount, "overdrawn balance");
  // CHANGED: never close balance on coldtoken
  // if (from.balance.amount == value.amount) {
  //   from_acnts.erase(from);
  // } else {
  from_acnts.modify(from, get_self(), [&](auto &a) { a.balance -= value; });
  // }
}

asset phoenixtoken::get_supply(symbol_code sym) const {
  stats statstable(_self, sym.raw());
  const auto &st = statstable.get(sym.raw());
  return st.supply;
}

asset phoenixtoken::get_balance(name owner, symbol sym) const {
  symbol_code sym_code = sym.code();
  accounts_t accountstable(_self, owner.value);
  const auto &ac = accountstable.find(sym_code.raw());
  return ac == accountstable.end() ? asset(0, sym) : ac->balance;
}

void phoenixtoken::check_user(const name &name) {
  print("phoenix_account", phoenix_account.to_string(), "\n");
  print("name", name.to_string(), "\n");
  phoenix::users_table _users(phoenix_account, phoenix_account.value, 1024, 64,
                              false, false, 0);
  _users.get(name.value,
             std::string("user does not exist" + name.to_string()).c_str());
}

void phoenixtoken::check_running() {
  globals g = get_globals();
  check(!g.paused, "contract is currently under maintenance. Check back soon");
}

std::vector<std::string> parse_memo(const std::string &memo) {
  std::vector<std::string> results;
  auto end = memo.cend();
  auto start = memo.cbegin();

  for (auto it = memo.cbegin(); it != end; ++it) {
    if (*it == ' ') {
      results.emplace_back(start, it);
      start = it + 1;
    }
  }
  if (start != end)
    results.emplace_back(start, end);

  return results;
}

void phoenixtoken::on_transfer(const eosio::name &from, const eosio::name &to,
                               const eosio::asset &quantity,
                               const std::string &memo) {
  check_running();
  // sending EOS, do nothing
  if (from == get_self() || from == name("eosio.stake")) {
    return;
  }

  // only care about WEOSDT transfers
  if (quantity.symbol != WEOSDT_EXT_SYMBOL.get_symbol())
    return;

  check(get_first_receiver() == WEOSDT_EXT_SYMBOL.get_contract(),
        "WEOSDT contract mismatch");

  check(to == get_self(), "contract is not involved in this transfer");
  check(quantity.symbol.is_valid(), "invalid quantity");
  check(quantity.symbol == WEOSDT_EXT_SYMBOL.get_symbol(),
        "invalid token symbol");

  vector<string> parsed_memo = parse_memo(memo);
  check(parsed_memo.size() == 2, "invalid memo");
  check(parsed_memo[0] == "deposit", "invalid memo 1");

  const name depositor = name(parsed_memo[1]);
  check_user(depositor);

  // issue virtual WEOSDT
  _transfer(PHOENIX_VACCOUNT, depositor, quantity);
}


#ifdef __TEST__
void phoenixtoken::test(name vaccount) {
  print("phoenix_account", phoenix_account.to_string(), "\n");
  print("name", vaccount.to_string(), "\n");
  phoenix::users_table _users(phoenix_account, phoenix_account.value, 1024, 64,
                              false, false, 0);
  auto user = _users.find(vaccount.value);
  check(user != _users.end(), "user does not exist");
  check(false, (std::string("user exists for some reason: ") + user->username.to_string()).c_str());
}
#endif

// EOSIO_DISPATCH_SVC_TRX(CONTRACT_NAME(), (login)(renewpledge))
extern "C" {
void apply(uint64_t receiver, uint64_t code, uint64_t action) {
  if (code != receiver && action == "transfer"_n.value) {
    eosio::execute_action(eosio::name(receiver), eosio::name(code),
                          &CONTRACT_NAME()::on_transfer);
  } else if (receiver == code) {
    switch (action) {
      EOSIO_DISPATCH_HELPER(CONTRACT_NAME(), DAPPSERVICE_ACTIONS_COMMANDS())
      EOSIO_DISPATCH_HELPER(CONTRACT_NAME(),
                            (create)(issue)(transfer)(open)(createacc))
#ifdef __TEST__
      EOSIO_DISPATCH_HELPER(CONTRACT_NAME(), (test))
#endif
      EOSIO_DISPATCH_HELPER(CONTRACT_NAME(), (xdcommit)(xvinit))
      EOSIO_DISPATCH_HELPER(CONTRACT_NAME(), (xsignal))
    default:
      check(false,
            "unrecognized internal token action: " + name(action).to_string());
    }
  }
  eosio_exit(0);
}
}