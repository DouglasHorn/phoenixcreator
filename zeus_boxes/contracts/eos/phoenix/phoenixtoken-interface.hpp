#pragma once

class phoenixtoken {
  public:
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

  inline asset get_supply(symbol_code sym) const;
  inline asset get_balance(name owner, symbol sym) const;

  using create_action =
      eosio::action_wrapper<"create"_n, &phoenixtoken::create>;
  using issue_action = eosio::action_wrapper<"issue"_n, &phoenixtoken::issue>;
  using transfer_action =
      eosio::action_wrapper<"transfer"_n, &phoenixtoken::transfer>;
  using open_action = eosio::action_wrapper<"open"_n, &phoenixtoken::open>;
};
