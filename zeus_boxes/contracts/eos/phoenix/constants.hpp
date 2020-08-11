#pragma once

static constexpr double FEES_PERCENTAGE = 0.02;

#ifdef __KYLIN__
// Kylin
static constexpr auto PHOENIX_VACCOUNT = "phoenix"_n;
static constexpr auto PHOENIX_FEES_VACCOUNT = "fees.phoenix"_n;
static constexpr name phoenix_account = name("phoenixv2c11");
static constexpr name token_account = name("phoenixv2t11");
static constexpr name dsp_name = eosio::name("airdropsdac1");

#ifdef __TEST__
static constexpr extended_symbol WEOSDT_EXT_SYMBOL{symbol("WEOSDT", 9),
                                               name("eosio.token")};
#else
static constexpr extended_symbol WEOSDT_EXT_SYMBOL{symbol("WEOSDT", 9),
                                               name("weosdttoken2")};
#endif
static constexpr symbol CHAIN_SYMBOL = eosio::symbol("EOS", 4);
static constexpr name FEES_ACCOUNT = name("phoenixashes");

#else
// WAX MAINNET
static constexpr auto PHOENIX_VACCOUNT = "phoenix"_n;
static constexpr auto PHOENIX_FEES_VACCOUNT = "fees.phoenix"_n;
static constexpr name phoenix_account = name("phoenixgroup");
static constexpr name token_account = name("phoenixcreat");
static constexpr name dsp_name = eosio::name("airdropsdac1");
static constexpr symbol CHAIN_SYMBOL = eosio::symbol("WAX", 8);
static constexpr extended_symbol WEOSDT_EXT_SYMBOL{symbol("WEOSDT", 9),
                                               name("weosdttokens")};
static constexpr name FEES_ACCOUNT = name("waxmeetup111");
#endif