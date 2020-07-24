#include "phoenix.hpp"

// #include "eosio_token.hpp"
#include "constants.hpp"
#include "helpers.hpp"
#include "phoenixtoken-interface.hpp"

auto phoenix::check_user(const name &name) {
  const auto user = _users.find(name.value);
  check(user != _users.end(), "user does not exist: " + name.to_string());
  return user;
}

void phoenix::check_running() {
  globals g = get_globals();
  check(!g.paused, "contract is currently under maintenance. Check back soon");
}

void phoenix::signup(const name &vaccount, const eosio::public_key &pubkey) {
  check_running();
  require_auth(get_self());

  vkeys_t vkeys_table(get_self(), get_self().value, 1024, 64,
                      VACCOUNTS_SHARD_PINNING, false,
                      VACCOUNTS_DELAYED_CLEANUP);
  auto existing = vkeys_table.find(vaccount.value);
  eosio::check(existing == vkeys_table.end(), "vaccount already exists");
  vkeys_table.emplace(get_self(), [&](auto &new_key) {
    new_key.pubkey = pubkey;
    new_key.vaccount = vaccount;
  });

  uint32_t day_identifier = get_day_identifier();
  limits limit = _limits.get_or_default();
  if (limit.day_identifier == day_identifier) {
    limit.vaccounts_created_today++;
  } else {
    limit.day_identifier = day_identifier;
    limit.vaccounts_created_today = 1;
  }
  check(limit.vaccounts_created_today <= limit.max_vaccount_creations_per_day,
        "daily vaccount registrations exceeded");
  _limits.set(limit, get_self());

  // Create a record in the table if the player doesn't exist in our app yet
  auto user = _users.find(vaccount.value);
  check(user == _users.end(), "user already exists");
  check(vaccount.to_string().length() >= 4 &&
            vaccount.to_string().length() <= 12,
        "username must be between 4 and 12 characters");
  _users.emplace(get_self(),
                 [&](auto &new_user) { new_user.username = vaccount; });

  // open token balances
  phoenixtoken::open_action vopen(token_account, {get_self(), "active"_n});
  vopen.send(vaccount, WEOSDT_EXT_SYMBOL.get_symbol());
}

void phoenix::login(const name &vaccount, const eosio::public_key &pubkey) {
  check_running();
  require_auth(get_self());

  vkeys_t vkeys_table(get_self(), get_self().value, 1024, 64,
                      VACCOUNTS_SHARD_PINNING, false,
                      VACCOUNTS_DELAYED_CLEANUP);
  auto itr = vkeys_table.find(vaccount.value);
  eosio::check(itr != vkeys_table.end(), "vaccount does not exist");
  vkeys_table.modify(itr, get_self(),
                     [&](auto &new_key) { new_key.pubkey = pubkey; });
}

// void phoenix::regaccount_hook(const regaccount_action &action) {
//   check_running();

//   uint32_t day_identifier = get_day_identifier();
//   limits limit = _limits.get_or_default();
//   if (limit.day_identifier == day_identifier) {
//     limit.vaccounts_created_today++;
//   } else {
//     limit.day_identifier = day_identifier;
//     limit.vaccounts_created_today = 1;
//   }
//   check(limit.vaccounts_created_today <=
//   limit.max_vaccount_creations_per_day,
//         "daily vaccount registrations exceeded");
//   _limits.set(limit, get_self());

//   // create a user upon registering, to avoid broken states
//   const name &username = action.vaccount;

//   // Create a record in the table if the player doesn't exist in our app yet
//   auto user = _users.find(username.value);
//   check(user == _users.end(), "user already exists");
//   check(
//       username.to_string().length() >= 4 && username.to_string().length() <=
//       12, "username must be between 4 and 12 characters");
//   _users.emplace(get_self(),
//                  [&](auto &new_user) { new_user.username = username; });

//   // open token balances
//   open(username, WEOSDT_EXT_SYMBOL.get_symbol());
// }

void phoenix::setlimits(const uint32_t &max_vaccount_creations_per_day) {
  require_auth(get_self());

  limits limit = _limits.get_or_default();
  limit.max_vaccount_creations_per_day = max_vaccount_creations_per_day;
  _limits.set(limit, get_self());
}

void phoenix::setfeatured(std::vector<name> featured_authors,
                          std::vector<uint64_t> featured_posts) {
  require_auth(get_self());

  globals g = get_globals();
  if (featured_authors.size()) {
    g.featured_authors = featured_authors;
  }
  if (featured_posts.size()) {
    g.featured_posts = featured_posts;
  }
  _globals.set(g, get_self());
}

void phoenix::pause(bool pause) {
  require_auth(get_self());

  globals g = get_globals();
  g.paused = pause;
  _globals.set(g, get_self());
}

void phoenix::init(eosio::public_key phoenix_vaccount_pubkey) {
  require_auth(get_self());
  // need to add the actual entry so frontend can read it
  globals g = get_globals();
  _globals.set(g, get_self());

  // set up core PHOENIX vaccount that distributes funds
  setKey(PHOENIX_VACCOUNT, phoenix_vaccount_pubkey);
  _users.emplace(get_self(),
                 [&](auto &new_user) { new_user.username = PHOENIX_VACCOUNT; });
  setKey(PHOENIX_FEES_VACCOUNT, phoenix_vaccount_pubkey);
  _users.emplace(get_self(), [&](auto &new_user) {
    new_user.username = PHOENIX_FEES_VACCOUNT;
  });

  phoenixtoken::open_action vopen(token_account, {get_self(), "active"_n});
  vopen.send(PHOENIX_VACCOUNT, WEOSDT_EXT_SYMBOL.get_symbol());
  vopen.send(PHOENIX_FEES_VACCOUNT, WEOSDT_EXT_SYMBOL.get_symbol());

  // not needed anymore
  // create VWEOSDT on token contract & issue all to PHOENIX vaccount
  // this is done externally because it requires token permission
  // const auto max_weosdt_supply =
  //     asset(170'000'000 * 1'000'000'000,
  //           WEOSDT_EXT_SYMBOL.get_symbol()); // 10 billion, same as EOS
  // phoenixtoken::create_action create(token_account, {get_self(),
  // "active"_n}); create.send(get_self(), max_weosdt_supply);
  // phoenixtoken::issue_action issue(token_account, {get_self(), "active"_n});
  // issue.send(PHOENIX_VACCOUNT, max_weosdt_supply, "init");
}

void phoenix::updateuser(const updateuser_payload &payload) {
  check_running();
  require_vaccount(payload.vaccount);
  auto user = check_user(payload.vaccount);

  _users.modify(user, get_self(),
                [&](auto &u) { u.profile_info = payload.new_profile_info; });
}

void phoenix::updatetiers(const updatetiers_payload &payload) {
  check_running();
  require_vaccount(payload.vaccount);
  auto user = check_user(payload.vaccount);
  check(payload.new_tiers.size() <= 5, "cannot have more than 5 tiers");

  _users.modify(user, get_self(),
                [&](auto &u) { u.tiers = payload.new_tiers; });
}

void phoenix::update_latest_posts(const uint64_t &post_id) {
  globals g = get_globals();
  g.latest_post_indexes.emplace(g.latest_post_indexes.begin(), post_id);
  if (g.latest_post_indexes.size() > 20) {
    // remove last element
    g.latest_post_indexes.erase(g.latest_post_indexes.end() - 1);
  }
  _globals.set(g, get_self());
}

void phoenix::remove_from_latest_post(const uint64_t &post_id) {
  globals g = get_globals();
  const auto &itr =
      std::find_if(g.latest_post_indexes.begin(), g.latest_post_indexes.end(),
                   [&](const uint64_t &id) { return id == post_id; });
  if (itr != g.latest_post_indexes.end()) {
    g.latest_post_indexes.erase(itr);
  }
  _globals.set(g, get_self());
}

void phoenix::createpost(createpost_payload payload) {
  check_running();
  require_vaccount(payload.vaccount);
  auto user = check_user(payload.vaccount);

  globals g = get_globals();
  check(payload.expected_id == g.next_post_id, "post id mismatch");
  uint64_t post_id = g.next_post_id++;
  _globals.set(g, get_self());

  _posts.emplace(get_self(), [&](auto &p) {
    p.id = post_id;
    p.author = payload.vaccount;
    p.title = payload.title;
    p.content = payload.content;
    p.featured_image_url = payload.featured_image_url;
    p.meta = payload.meta;
    p.encrypted = payload.encrypted;
    p.decrypt_for_usd = payload.decrypt_for_usd;
    p.created = (eosio::time_point_sec)eosio::current_time_point();
    p.updated = (eosio::time_point_sec)eosio::current_time_point();
  });

  _users.modify(user, get_self(),
                [&](auto &u) { u.post_indexes.emplace_back(post_id); });

  update_latest_posts(post_id);

  if (payload.encrypted) {
    check(payload.decrypt_for_usd >= 0.01,
          "encrypted posts require a decryption USD value of at least 0.01");
    check(payload.post_key.size() > 0,
          "encrypted posts require an encrypted post_key parameter");
    post_key_enc_table _posts_keys(get_self(), dsp_name.value, 1024, 64, false,
                                   false, VACCOUNTS_DELAYED_CLEANUP);
    _posts_keys.emplace(get_self(), [&](auto &x) {
      x.post_id = post_id;
      x.post_key = payload.post_key;
    });
  }
}

void phoenix::updatepost(updatepost_payload payload) {
  check_running();
  require_vaccount(payload.vaccount);
  auto user = check_user(payload.vaccount);

  const auto post = _posts.find(payload.id);
  check(post != _posts.end(), "post does not exist for this author");
  check(post->author == payload.vaccount,
        "you are not the author of this post");

  if (payload.delete_post) {
    _users.modify(user, get_self(), [&](auto &u) {
      const auto &itr = std::find_if(
          u.post_indexes.begin(), u.post_indexes.end(),
          [&](const uint64_t &post_id) { return post_id == payload.id; });
      check(itr != u.post_indexes.end(), "post already deleted");
      u.post_indexes.erase(itr);
    });

    post_key_enc_table _posts_keys(get_self(),
                                   eosio::name("airdropsdac1").value, 1024, 64,
                                   false, false, VACCOUNTS_DELAYED_CLEANUP);
    auto existing_post_key = _posts_keys.find(payload.id);
    if (existing_post_key != _posts_keys.end()) {
      _posts_keys.erase(existing_post_key);
    }

    remove_from_latest_post(payload.id);
  } else {
    _posts.modify(post, get_self(), [&](auto &p) {
      p.title = payload.title;
      p.content = payload.content;
      p.featured_image_url = payload.featured_image_url;
      p.meta = payload.meta;
      p.encrypted = payload.encrypted;
      p.decrypt_for_usd = payload.decrypt_for_usd;
      p.updated = (eosio::time_point_sec)eosio::current_time_point();
    });

    if (payload.encrypted) {
      check(payload.decrypt_for_usd >= 0.01,
            "encrypted posts require a decryption USD value of at least 0.01");
      check(payload.post_key.size() > 0,
            "encrypted posts require an encrypted post_key parameter");
      post_key_enc_table _posts_keys(
          get_self(), eosio::name("airdropsdac1").value, 1024, 64, false, false,
          VACCOUNTS_DELAYED_CLEANUP);
      auto existing_post_key = _posts_keys.find(payload.id);
      if (existing_post_key == _posts_keys.end()) {
        _posts_keys.emplace(get_self(), [&](auto &x) {
          x.post_id = payload.id;
          x.post_key = payload.post_key;
        });
      } else {
        _posts_keys.modify(existing_post_key, get_self(),
                           [&](auto &x) { x.post_key = payload.post_key; });
      }
    }
  }
}

void phoenix::follow(follow_payload payload) {
  check_running();
  require_vaccount(payload.vaccount);
  check_user(payload.vaccount);

  // update _follows_from
  const auto from_follows = _follows_from.find(payload.vaccount.value);
  if (from_follows == _follows_from.end()) {
    _follows_from.emplace(get_self(), [&](auto &s) {
      s.from = payload.vaccount;
      s.tos = std::vector<name>{};
      diff_vectors(s.tos, payload.follows, payload.unfollows);
    });
  } else {
    _follows_from.modify(from_follows, get_self(), [&](auto &s) {
      diff_vectors(s.tos, payload.follows, payload.unfollows);
    });
  }

  // update _follows_to
  for (const name follower : payload.follows) {
    check_user(follower);

    const auto to_follows = _follows_to.find(follower.value);
    if (to_follows == _follows_to.end()) {
      _follows_to.emplace(get_self(), [&](auto &s) {
        s.to = follower;
        diff_vectors(s.froms, std::vector<name>{payload.vaccount},
                     std::vector<name>{});
      });
    } else {
      _follows_to.modify(to_follows, get_self(), [&](auto &s) {
        diff_vectors(s.froms, std::vector<name>{payload.vaccount},
                     std::vector<name>{});
      });
    }
  }

  for (const name follower : payload.unfollows) {
    check_user(follower);

    const auto to_follows = _follows_to.find(follower.value);
    if (to_follows == _follows_to.end()) {
      // nothing to do
    } else {
      _follows_to.modify(to_follows, get_self(), [&](auto &s) {
        diff_vectors(s.froms, std::vector<name>{},
                     std::vector<name>{payload.vaccount});
      });
    }
  }
}

void phoenix::linkaccount(linkaccount_payload payload) {
  check_running();
  require_vaccount(payload.vaccount);

  const auto user = check_user(payload.vaccount);

  if (payload.account != ""_n) {
    check(is_account(payload.account), "linked eos account does not exist");
  }

  _users.modify(user, get_self(),
                [&](auto &u) { u.linked_name = payload.account; });
}

void phoenix::setcustomurl(setcustomurl_payload payload) {
  check_running();
  require_vaccount(payload.vaccount);
  check(payload.url.to_string().size() >= 3, "url must be at least 3 characters long");
  const auto user = check_user(payload.vaccount);

  customurl_table _customurl(get_self(), get_self().value, 1024, 64, false, false,
                             VACCOUNTS_DELAYED_CLEANUP);
  auto itr = _customurl.find(payload.vaccount.value);
  if (itr == _customurl.end()) {
    _customurl.emplace(get_self(), [&](auto &row) {
      row.username = payload.vaccount;
      row.url = payload.url;
    });
  } else {
    _customurl.modify(itr, get_self(),
                      [&](auto &u) { u.url = payload.url; });
  }

  // do reverse link
  _customurl = customurl_table(get_self(), payload.url.value, 1024, 64, false, false,
                               VACCOUNTS_DELAYED_CLEANUP);
  itr = _customurl.find(0);
  if (itr == _customurl.end()) {
    _customurl.emplace(get_self(), [&](auto &row) {
      // primary key = 0
      row.username = name(0);
      row.url = payload.vaccount;
    });
  } else {
    check(itr->url == payload.vaccount, "custom link claimed by another vaccount");
    _customurl.modify(itr, get_self(),
                      [&](auto &u) { u.url = payload.vaccount; });
  }
}

void phoenix::createacc(createacc_payload payload) {
  check_running();
  require_vaccount(payload.vaccount);

  const auto user = check_user(payload.vaccount);
  check(user->created_account == name(""),
        "you already created a free account");
  _users.modify(user, get_self(),
                [&](auto &u) { u.created_account = payload.account_name; });

  phoenixtoken::createacc_action vcreateacc(token_account,
                                            {get_self(), "active"_n});
  vcreateacc.send(payload.account_name, payload.pubkey);
}

void phoenix::on_transfer(eosio::name from, eosio::name to,
                          eosio::asset quantity, std::string memo) {
  check_running();

  // accept everything, deny WEOSDT
  check(get_first_receiver() != WEOSDT_EXT_SYMBOL.get_contract(),
        "WEOSDT must be deposited to the token contract");
}

void phoenix::pledge(pledge_payload payload) {
  check_running();
  require_vaccount(payload.vaccount);
  // no need to check users here,
  // will be checked when pledge is paid and balances are adjusted
  // check_user(payload.vaccount);
  // check_user(payload.to);
  check(payload.weosdt_quantity.symbol.is_valid(), "invalid weosdt_quantity");
  check(payload.weosdt_quantity.symbol == WEOSDT_EXT_SYMBOL.get_symbol(),
        "invalid token symbol for weosdt_quantity");
  check(payload.weosdt_quantity.amount > 0, "weosdt_quantity must be > 0");

  const auto pledges_from = _pledges_from.find(payload.vaccount.value);
  if (pledges_from == _pledges_from.end()) {
    const uint64_t pledge_id = create_pledge(payload);
    upsert_pledge_relations(payload, pledge_id);
  } else {
    const auto &tos = pledges_from->tos;
    const auto &pair_itr =
        std::find_if(tos.begin(), tos.end(), [&](const name_pledge_pair &p) {
          return p.name == payload.to;
        });
    if (pair_itr == tos.end()) {
      const uint64_t pledge_id = create_pledge(payload);
      upsert_pledge_relations(payload, pledge_id);
    } else {
      update_pledge(payload, pair_itr->pledge_id);
    }
  }
}

bool phoenix::timer_callback(name timer, std::vector<char> payload,
                             uint32_t seconds) {
  // TODO: enable auth check ? or are SVC commands secured automatically
  // Tal: every service commands trigger a usage inline action.
  // if you don't have a QUOTA (or any staking) for that DSP, the action should
  // fail. timer should only be called as deferred transaction with our auth
  // require_auth(DSP_PROVIDER); // invoked by DSP provider

  // deserialize timer_payload
  timer_payload unpacked_timer_payload = eosio::unpack<timer_payload>(payload);

  // deserialize renewpledge_payload
  if (unpacked_timer_payload.type == "renewpledge"_n) {
    renewpledge_payload unpacked_inner_payload =
        eosio::unpack<renewpledge_payload>(unpacked_timer_payload.payload);
    renewpledge(unpacked_inner_payload);
  } else {
    check(false, "unknown timer_payload type: " +
                     unpacked_timer_payload.type.to_string());
  }

  return false;
}

void phoenix::renewpledge(renewpledge_payload payload) {
  check_running();
  check_user(payload.to);
  // just a sanity check
  const auto pledges_to = _pledges_to.find(payload.to.value);
  check(pledges_to != _pledges_to.end(), "user does not have any pledges");
  const auto &froms = pledges_to->froms;
  const auto &pair_itr =
      std::find_if(froms.begin(), froms.end(), [&](const name_pledge_pair &p) {
        return p.pledge_id == payload.pledge_id;
      });
  check(pair_itr != froms.end(), "pledge does not exist for receiver");

  const auto &pledge = _pledges.find(payload.pledge_id);
  check(pledge != _pledges.end(), "pledge does not exist");
  check(pair_itr->name == pledge->from,
        "pledgers differ in pledge and relations");

  // if autorenew anyone can renew, otherwise require pledgers auth
  if (!pledge->autorenew) {
    require_vaccount(pledge->from);
  }

  const auto cycle_end = pledge->cycle_start + pledge->cycle_us;
  check(eosio::current_time_point() > cycle_end,
        "current pledge is still running");

  // update pledge info from new cycle
  _pledges.modify(pledge, get_self(), [&](auto &p) {
    p.cycle_start = eosio::current_time_point();
    p.usd_value = p.next_usd_value;
    p.weosdt_quantity = p.next_weosdt_quantity;
    p.cycle_us = p.next_cycle_us;
  });
  pay_pledge(pledge->from, payload.pledge_id);
}

uint64_t phoenix::create_pledge(const pledge_payload &payload) {
  // create pledge
  const auto id = _pledges.available_primary_key();
  const auto &new_pledge = _pledges.emplace(get_self(), [&](auto &p) {
    p.id = id;
    p.from = payload.vaccount;
    p.to = payload.to;
    p.usd_value = asset_to_decimal(payload.weosdt_quantity);
    p.weosdt_quantity = payload.weosdt_quantity;
    p.autorenew = payload.autorenew;
#ifdef __TEST__
    p.cycle_us = minutes(5);
#else
    p.cycle_us = days(30);
#endif
    p.cycle_start = eosio::current_time_point();

    p.next_cycle_us = p.cycle_us;
    p.next_usd_value = p.usd_value;
    p.next_weosdt_quantity = p.weosdt_quantity;
    p.next_delete = false;
  });

  pay_pledge(payload.vaccount, id);

  return id;
}

void phoenix::update_pledge(const pledge_payload &payload,
                            const uint64_t &pledge_id) {
  const auto &pledge = _pledges.find(pledge_id);
  check(pledge != _pledges.end(), "pledge does not exist");

  _pledges.modify(pledge, get_self(), [&](auto &p) {
    p.next_usd_value = payload.usd_value;
    p.next_weosdt_quantity = payload.weosdt_quantity;
    p.autorenew = payload.autorenew;
    p.next_delete = payload.next_delete;
  });

  const auto cycle_end = pledge->cycle_start + pledge->cycle_us;
  if (eosio::current_time_point() > cycle_end) {
    renewpledge(renewpledge_payload{.to = pledge->to, .pledge_id = pledge->id});
  }
}

void phoenix::upsert_pledge_relations(const pledge_payload &payload,
                                      uint64_t pledge_id) {
  const auto pledges_from = _pledges_from.find(payload.vaccount.value);
  if (pledges_from == _pledges_from.end()) {
    _pledges_from.emplace(get_self(), [&](auto &p) {
      p.from = payload.vaccount;
      p.tos = std::vector<name_pledge_pair>{
          name_pledge_pair{payload.to, pledge_id}};
    });
  } else {
    const auto &tos = pledges_from->tos;
    const auto &pair_itr =
        std::find_if(tos.begin(), tos.end(), [&](const name_pledge_pair &p) {
          return p.name == payload.to;
        });
    if (pair_itr == tos.end()) {
      _pledges_from.modify(pledges_from, get_self(), [&](auto &p) {
        p.tos.emplace_back(name_pledge_pair{payload.to, pledge_id});
      });
    };
  }

  const auto pledges_to = _pledges_to.find(payload.to.value);
  if (pledges_to == _pledges_to.end()) {
    _pledges_to.emplace(get_self(), [&](auto &p) {
      p.to = payload.to;
      p.froms = std::vector<name_pledge_pair>{
          name_pledge_pair{payload.vaccount, pledge_id}};
    });
  } else {
    const auto &froms = pledges_to->froms;
    const auto &pair_itr = std::find_if(
        froms.begin(), froms.end(),
        [&](const name_pledge_pair &p) { return p.name == payload.vaccount; });
    if (pair_itr == froms.end()) {
      _pledges_to.modify(pledges_to, get_self(), [&](auto &p) {
        p.froms.emplace_back(name_pledge_pair{payload.vaccount, pledge_id});
      });
    };
  }
}

void phoenix::pay_pledge(const name &payer, const uint64_t &pledge_id) {
  const auto &pledge = _pledges.find(pledge_id);
  check(pledge != _pledges.end(), "pledge does not exist (pay_pledge)");

  auto quantity = pledge->weosdt_quantity;
  auto fees = asset(FEES_PERCENTAGE * quantity.amount, quantity.symbol);
  // update user WEOSDT balance
  if ((quantity - fees).amount > 0) {
    internal_vtransfer(payer, pledge->to, quantity - fees, "subscription");
  }
  if (fees.amount > 0) {
    // transfer fees to vfees account
    internal_vtransfer(payer, PHOENIX_FEES_VACCOUNT, fees, "subscription fees");
  }

  if (pledge->autorenew) {
    schedule_renewpledge(*pledge);
  }
}

void phoenix::schedule_renewpledge(const pledge_info &pledge) {
  auto seconds =
      ((pledge.cycle_start + pledge.cycle_us) - eosio::current_time_point())
          .to_seconds() +
      1;

  renewpledge_payload inner_payload =
      renewpledge_payload{.to = pledge.to, .pledge_id = pledge.id};
  std::vector<char> packed_inner_payload = eosio::pack(inner_payload);

  timer_payload payload =
      timer_payload{.type = "renewpledge"_n, .payload = packed_inner_payload};
  std::vector<char> packed_payload = eosio::pack(payload);

  schedule_timer(name(pledge.id), packed_payload, seconds);
}

void phoenix::internal_vtransfer(const eosio::name &from, const eosio::name &to,
                                 const eosio::asset &quantity,
                                 const std::string &memo) {
  phoenixtoken::transfer_action vtransfer(token_account,
                                          {get_self(), "active"_n});
  vtransfer.send(from, to, quantity, memo);
}

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
  print("testreset");

  {
    auto raw_table = users_table_abi(get_self(), get_self().value);
    clear_table(raw_table);
    // _users.clear();
  }

  {
    auto raw_table = posts_table_abi(get_self(), get_self().value);
    clear_table(raw_table);
    // _posts.clear();
  }

  {
    auto raw_table = post_key_enc_table_abi(get_self(), get_self().value);
    clear_table(raw_table);
    post_key_enc_table _posts_keys(get_self(), dsp_name.value, 1024, 64, false,
                                   false, VACCOUNTS_DELAYED_CLEANUP);
    // _posts_keys.clear();
  }

  {
    auto raw_table = follows_from_table_abi(get_self(), get_self().value);
    clear_table(raw_table);
    // _follows_from.clear();
  }

  {
    auto raw_table = follows_to_table_abi(get_self(), get_self().value);
    clear_table(raw_table);
    // _follows_to.clear();
  }

  {
    auto raw_table = pledges_table_abi(get_self(), get_self().value);
    clear_table(raw_table);
    // _pledges.clear();
  }

  {
    auto raw_table = pledges_from_table_abi(get_self(), get_self().value);
    clear_table(raw_table);
    // _pledges_from.clear();
  }

  {
    auto raw_table = pledges_to_table_abi(get_self(), get_self().value);
    clear_table(raw_table);
    // _pledges_to.clear();
  }

  {
    auto raw_table = ipfsentries_t(get_self(), get_self().value);
    clear_table(raw_table);
  }

  {
    auto raw_table = vkeys_t_abi(get_self(), get_self().value);
    clear_table(raw_table);
  }
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
      EOSIO_DISPATCH_HELPER(CONTRACT_NAME(), (init)(setfeatured)(pause)(
                                                 renewpledge)(signup)(login))
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