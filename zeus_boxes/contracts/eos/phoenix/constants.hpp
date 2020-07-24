#pragma once

#ifdef __TEST__
static constexpr auto PHOENIX_VACCOUNT = "phoenix"_n;
static constexpr auto PHOENIX_FEES_VACCOUNT = "fees.phoenix"_n;
static constexpr name phoenix_account = name("phoenixv2c11");
static constexpr name token_account = name("phoenixv2t11");
static constexpr name dsp_name = eosio::name("airdropsdac1");

static const extended_symbol WEOSDT_EXT_SYMBOL{symbol("WEOSDT", 9),
                                               name("ibc1eos1tok3")};


static constexpr symbol CHAIN_SYMBOL = eosio::symbol("EOS", 4);

static constexpr double FEES_PERCENTAGE = 0.02;
static constexpr name FEES_ACCOUNT = name("phoenixashes");
#else
// static constexpr symbol CHAIN_SYMBOL = eosio::symbol("WAX", 8)
static const extended_symbol WEOSDT_EXT_SYMBOL{symbol("WEOSDT", 9),
                                               name("weosdttokens")};
#endif