#include "constants.hpp"
auto phoenix::check_user(const name &name) {
  const auto user = _users.find(name.value);
  check(user != _users.end(), "user does not exist: " + name.to_string());
  return user;
}

void phoenix::check_running() {
  globals g = get_globals();
  check(!g.paused, "contract is currently under maintenance. Check back soon");
}

void phoenix::signup(const name &vaccount, const eosio::public_key &pubkey,
                     const uint64_t &checksum) {
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
  _users.emplace(get_self(), [&](auto &new_user) {
    new_user.username = vaccount;
    new_user.checksum = checksum;
  });

  // open token balances
  phoenixtoken::open_action vopen(token_account, {get_self(), "active"_n});
  vopen.send(vaccount, WEOSDT_EXT_SYMBOL.get_symbol());
}

void phoenix::login(const name &vaccount, const eosio::public_key &pubkey,
                    const uint64_t &checksum) {
  check_running();
  require_auth(get_self());

  vkeys_t vkeys_table(get_self(), get_self().value, 1024, 64,
                      VACCOUNTS_SHARD_PINNING, false,
                      VACCOUNTS_DELAYED_CLEANUP);
  auto itr = vkeys_table.find(vaccount.value);
  eosio::check(itr != vkeys_table.end(), "vaccount does not exist");
  vkeys_table.modify(itr, get_self(),
                     [&](auto &new_key) { new_key.pubkey = pubkey; });

  auto user = check_user(vaccount);
  check(user->checksum == checksum, "checksum mismatch");
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
  check(payload.new_profile_info.description.size() < 400,
        "description must be less than 400 chars");
  check(payload.new_profile_info.displayName.size() < 100,
        "name must be less than 100 chars");
  check(payload.new_profile_info.headerSrc.size() < 255,
        "header must be less than 255 chars");
  check(payload.new_profile_info.website.size() < 255,
        "website must be less than 255 chars");

  const set<name> valid_socials = set<name>{
      // EOSIO system accounts
      "facebook"_n, "instagram"_n, "twitch"_n, "twitter"_n, "youtube"_n,
  };

  for (auto social_entry : payload.new_profile_info.social) {
    check(valid_socials.find(social_entry.first) != valid_socials.end(),
          "invalid social brand");
    check(social_entry.second.size() < 50,
          "social names must be less than 50 chars");
  }

  _users.modify(user, get_self(),
                [&](auto &u) { u.profile_info = payload.new_profile_info; });

  // update url?
  if (payload.url == name(""))
    return;

  check(payload.url.suffix() != name("phnx"),
        "phnx suffix may not be used in a custom url");
  check(payload.url.to_string().size() >= 3,
        "url must be at least 3 characters long");

  customurl_table _customurl(get_self(), get_self().value, 1024, 64, false,
                             false, VACCOUNTS_DELAYED_CLEANUP);
  auto itr = _customurl.find(payload.vaccount.value);
  if (itr == _customurl.end()) {
    _customurl.emplace(get_self(), [&](auto &row) {
      row.username = payload.vaccount;
      row.url = payload.url;
    });
  } else {
    _customurl.modify(itr, get_self(), [&](auto &u) { u.url = payload.url; });
  }

  // do reverse link
  _customurl = customurl_table(get_self(), payload.url.value, 1024, 64, false,
                               false, VACCOUNTS_DELAYED_CLEANUP);
  itr = _customurl.find(0);
  if (itr == _customurl.end()) {
    _customurl.emplace(get_self(), [&](auto &row) {
      // primary key = 0
      row.username = name(0);
      row.url = payload.vaccount;
    });
  } else {
    check(itr->url == payload.vaccount,
          "custom link claimed by another vaccount");
    _customurl.modify(itr, get_self(),
                      [&](auto &u) { u.url = payload.vaccount; });
  }
}

void phoenix::updatetiers(const updatetiers_payload &payload) {
  check_running();
  require_vaccount(payload.vaccount);
  auto user = check_user(payload.vaccount);

  check(payload.new_tiers.size() <= 5, "cannot have more than 5 tiers");
  for (auto tier : payload.new_tiers) {
    check(tier.title.size() < 150, "title must be less than 150 chars");
    check(tier.description.size() <= 144,
          "description must be less than 144 chars");
    check(tier.benefits.size() <= 5, "a maximum of 5 benefits is allowed");
    for (auto benefit : tier.benefits) {
      check(benefit.size() <= 100, "benefits cannot be longer than 100 chars");
    }
  }
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
                                   dsp_name.value, 1024, 64,
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
           get_self(), dsp_name.value, 1024, 64, false, false,
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

  follows_table _follows_from(get_self(), name("from").value, 1024, 64, false,
                              false, VACCOUNTS_DELAYED_CLEANUP);
  const auto from_follows = _follows_from.find(payload.vaccount.value);
  if (from_follows == _follows_from.end()) {
    _follows_from.emplace(get_self(), [&](auto &s) {
      s.user = payload.vaccount;
      s.users = std::vector<name>{};
      diff_vectors(s.users, payload.follows, payload.unfollows);
    });
  } else {
    _follows_from.modify(from_follows, get_self(), [&](auto &s) {
      diff_vectors(s.users, payload.follows, payload.unfollows);
    });
  }

  // update _follows_to
  follows_table _follows_to(get_self(), name("to").value, 1024, 64, false,
                            false, VACCOUNTS_DELAYED_CLEANUP);
  for (const name follower : payload.follows) {
    check_user(follower);

    const auto to_follows = _follows_to.find(follower.value);
    if (to_follows == _follows_to.end()) {
      _follows_to.emplace(get_self(), [&](auto &s) {
        s.user = follower;
        diff_vectors(s.users, std::vector<name>{payload.vaccount},
                     std::vector<name>{});
      });
    } else {
      _follows_to.modify(to_follows, get_self(), [&](auto &s) {
        diff_vectors(s.users, std::vector<name>{payload.vaccount},
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
        diff_vectors(s.users, std::vector<name>{},
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
    check(is_account(payload.account), "linked chain account does not exist");
  }

  _users.modify(user, get_self(),
                [&](auto &u) { u.linked_name = payload.account; });
}

void phoenix::logcreateacc(name vaccount, name created_account,
                           eosio::public_key pubkey) {
  check_running();
  require_auth(token_account);

  const auto user = check_user(vaccount);
  check(user->created_account == name(""),
        "you already created a free account");
  _users.modify(user, get_self(),
                [&](auto &u) { u.created_account = created_account; });
}

void phoenix::on_transfer(eosio::name from, eosio::name to,
                          eosio::asset quantity, std::string memo) {
  check_running();

  // accept everything, deny WEOSDT
  check(get_first_receiver() != WEOSDT_EXT_SYMBOL.get_contract(),
        "WEOSDT must be deposited to the token contract");
}

void phoenix::internal_vtransfer(const eosio::name &from, const eosio::name &to,
                                 const eosio::asset &quantity,
                                 const std::string &memo) {
  phoenixtoken::transfer_action vtransfer(token_account,
                                          {get_self(), "active"_n});
  vtransfer.send(from, to, quantity, memo);
}
