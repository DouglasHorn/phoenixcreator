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
ACTION transfer(name from, name to, asset quantity, string memo);
ACTION transferv(transferv_payload payload);
ACTION open(const name& owner, const symbol& symbol);

inline asset get_supply(symbol_code sym) const;
inline asset get_balance(name owner, symbol sym) const;

using create_action = eosio::action_wrapper<"create"_n, &phoenix::create>;
using issue_action = eosio::action_wrapper<"issue"_n, &phoenix::issue>;
using transfer_action = eosio::action_wrapper<"transfer"_n, &phoenix::transfer>;
using open_action = eosio::action_wrapper<"open"_n, &phoenix::open>;

private:
TABLE account {
  asset balance;
  uint64_t primary_key() const { return balance.symbol.code().raw(); }
};

TABLE currency_stats {
  asset supply;
  asset max_supply;
  name issuer;

  uint64_t primary_key() const { return supply.symbol.code().raw(); }
};

typedef dapp::multi_index<"accounts"_n, account> accounts_t;
typedef eosio::multi_index<".accounts"_n, account> accounts_t_v_abi;
typedef eosio::multi_index<"accounts"_n, shardbucket> accounts_t_abi;

typedef eosio::multi_index<"stat"_n, currency_stats> stats;

void sub_balance(name owner, asset value);
void add_balance(name owner, asset value);
void _transfer(const name& from, const name& to, const asset& quantity);
