#include "phoenix.hpp"

using namespace eosio;

void phoenix::create(name issuer, asset maximum_supply) {
  require_auth(get_self());

  auto sym = maximum_supply.symbol;
  eosio::check(sym.is_valid(), "invalid symbol name");
  eosio::check(maximum_supply.is_valid(), "invalid supply");
  eosio::check(maximum_supply.amount > 0, "max-supply must be positive");

  stats statstable(_self, sym.code().raw());
  auto existing = statstable.find(sym.code().raw());
  eosio::check(existing == statstable.end(),
               "token with symbol already exists");

  statstable.emplace(_self, [&](auto& s) {
    s.supply.symbol = maximum_supply.symbol;
    s.max_supply = maximum_supply;
    s.issuer = issuer;
  });
}

void phoenix::issue(name to, asset quantity, string memo) {
  check_running();
  auto sym = quantity.symbol;
  eosio::check(sym.is_valid(), "invalid symbol name");
  eosio::check(memo.size() <= 256, "memo has more than 256 bytes");

  auto sym_name = sym.code().raw();
  stats statstable(_self, sym_name);
  auto existing = statstable.find(sym_name);
  eosio::check(existing != statstable.end(),
               "token with symbol does not exist, create token before issue");
  const auto& st = *existing;

  require_auth(st.issuer);
  eosio::check(quantity.is_valid(), "invalid quantity");
  eosio::check(quantity.amount > 0, "must issue positive quantity");

  eosio::check(quantity.symbol == st.supply.symbol,
               "symbol precision mismatch");
  eosio::check(quantity.amount <= st.max_supply.amount - st.supply.amount,
               "quantity exceeds available supply");

  statstable.modify(st, eosio::same_payer,
                    [&](auto& s) { s.supply += quantity; });

  // special check here because the EOS account => vAccount mapping
  // makes it impossible to know for any other issuer
  eosio::check(st.issuer == get_self(), "only phoenix account may issue");
  eosio::check(to == PHOENIX_VACCOUNT,
               "only phoenix vaccount may receive new supply");
  // check if phoenix vaccount exists
  phoenix::check_user(PHOENIX_VACCOUNT);
  add_balance(to, quantity);

  //   if (to != st.issuer) {
  //     SEND_INLINE_ACTION(*this, transfer, {st.issuer, "active"_n},
  //                        {st.issuer, to, quantity, memo});
  //   }
}

void phoenix::transfer(name from, name to, asset quantity, string memo) {
  check_running();
  // invoked through transferv as an inline action from vaccounts
  require_auth(get_self());
  eosio::check(from != to, "cannot transfer to self");
  phoenix::check_user(from);
  phoenix::check_user(to);

  eosio::check(memo.size() <= 256, "memo has more than 256 bytes");

  _transfer(from, to, quantity);
}

void phoenix::open(const name& owner, const symbol& symbol) {
  check_running();
  // we want to open it in signup action and do not have vaccount auth
  if(!has_auth(get_self())) {
    require_vaccount(owner);
  }

  phoenix::check_user(owner);

  auto sym_code_raw = symbol.code().raw();
  stats statstable(get_self(), sym_code_raw);
  const auto& st = statstable.get(sym_code_raw, "symbol does not exist");
  check(st.supply.symbol == symbol, "symbol precision mismatch");

  accounts_t acnts(
      get_self(),   // contract
      owner.value,  // scope
      1024,         // optional: shards per table
      64,           // optional: buckets per shard
      false,  // optional: pin shards in RAM - (buckets per shard) X (shards
              // per table) X 32B - 2MB in this example
      false,  // optional: pin buckets in RAM - keeps most of the data in RAM.
              // should be evicted manually after the process
      VACCOUNTS_DELAYED_CLEANUP);

  auto it = acnts.find(sym_code_raw);
  if (it == acnts.end()) {
    acnts.emplace(get_self(), [&](auto& a) { a.balance = asset{0, symbol}; });
  }
}

void phoenix::_transfer(const name& from, const name& to,
                        const asset& quantity) {
  auto sym = quantity.symbol.code().raw();
  stats statstable(_self, sym);
  const auto& st = statstable.get(sym);

  // require_recipient( from );
  // require_recipient( to );

  eosio::check(quantity.is_valid(), "invalid quantity");
  eosio::check(quantity.amount > 0, "must transfer positive quantity");
  eosio::check(quantity.symbol == st.supply.symbol,
               "symbol precision mismatch");
  sub_balance(from, quantity);
  add_balance(to, quantity);
}

void phoenix::transferv(transferv_payload payload) {
  check_running();
  require_vaccount(payload.from);

  phoenix::transfer_action transfer(get_self(), {get_self(), "active"_n});
  transfer.send(payload.from, payload.to, payload.quantity, payload.memo);
}

void phoenix::add_balance(name owner, asset value) {
  accounts_t to_acnts(
      get_self(),   // contract
      owner.value,  // scope
      1024,         // optional: shards per table
      64,           // optional: buckets per shard
      false,  // optional: pin shards in RAM - (buckets per shard) X (shards
              // per table) X 32B - 2MB in this example
      false,  // optional: pin buckets in RAM - keeps most of the data in RAM.
              // should be evicted manually after the process
      VACCOUNTS_DELAYED_CLEANUP);
  const auto& to = to_acnts.find(value.symbol.code().raw());
  if (to == to_acnts.end()) {
    to_acnts.emplace(get_self(), [&](auto& a) { a.balance = value; });
  } else {
    to_acnts.modify(to, get_self(), [&](auto& a) { a.balance += value; });
  }
}

void phoenix::sub_balance(name owner, asset value) {
  accounts_t from_acnts(get_self(), owner.value, 1024, 64, false, false,
                        VACCOUNTS_DELAYED_CLEANUP);
  const auto& from =
      from_acnts.get(value.symbol.code().raw(), "no balance object found");
  eosio::check(from.balance.amount >= value.amount, "overdrawn balance");
  // CHANGED: never close balance on coldtoken
  // if (from.balance.amount == value.amount) {
  //   from_acnts.erase(from);
  // } else {
    from_acnts.modify(from, get_self(), [&](auto& a) { a.balance -= value; });
  // }
}

asset phoenix::get_supply(symbol_code sym) const {
  stats statstable(_self, sym.raw());
  const auto& st = statstable.get(sym.raw());
  return st.supply;
}

asset phoenix::get_balance(name owner, symbol sym) const {
  symbol_code sym_code = sym.code();
  accounts_t accountstable(_self, owner.value);
  const auto& ac = accountstable.find(sym_code.raw());
  return ac == accountstable.end() ? asset(0, sym) : ac->balance;
}
