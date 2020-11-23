/**
 *  Pledges are split into two actions because we exceed deadlines
 * 1. pledges are created or updated using `pledge` action
 * 2. pledges are paid using `renewpledge` action
 * 
 * Once a pledge has been established it's enough to pay with renewpledge
 */
void phoenix::pledge(pledge_payload payload) {
  check_running();
  require_vaccount(payload.vaccount);
  // no need to check users here,
  // will be checked when pledge is paid and balances are adjusted
  check_user(payload.vaccount);
  check_user(payload.to);
  check(payload.weosdt_quantity.symbol.is_valid(), "invalid weosdt_quantity");
  check(payload.weosdt_quantity.symbol == WEOSDT_EXT_SYMBOL.get_symbol(),
        "invalid token symbol for weosdt_quantity");
  check(payload.weosdt_quantity.amount > 0, "weosdt_quantity must be > 0");


  pledges_rel_table _pledges_from(get_self(), name("from").value, 1024, 64, false, false,
               VACCOUNTS_DELAYED_CLEANUP);
  const auto pledges_from = _pledges_from.find(payload.vaccount.value);
  if (pledges_from == _pledges_from.end()) {
    const uint64_t pledge_id = create_pledge(payload);
    upsert_pledge_relations(payload, pledge_id);
  } else {
    const auto &tos = pledges_from->users;
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
    p.cycle_us = days(30);
    p.cycle_start = eosio::current_time_point();

    p.next_cycle_us = p.cycle_us;
    p.next_usd_value = p.usd_value;
    p.next_weosdt_quantity = p.weosdt_quantity;
    p.next_delete = false;
    p.paid = false;
  });

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
}

void phoenix::upsert_pledge_relations(const pledge_payload &payload,
                                      uint64_t pledge_id) {
  pledges_rel_table _pledges_from(get_self(), name("from").value, 1024, 64, false, false,
               VACCOUNTS_DELAYED_CLEANUP);
  const auto pledges_from = _pledges_from.find(payload.vaccount.value);
  if (pledges_from == _pledges_from.end()) {
    _pledges_from.emplace(get_self(), [&](auto &p) {
      p.user = payload.vaccount;
      p.users = std::vector<name_pledge_pair>{
          name_pledge_pair{payload.to, pledge_id}};
    });
  } else {
    const auto &tos = pledges_from->users;
    const auto &pair_itr =
        std::find_if(tos.begin(), tos.end(), [&](const name_pledge_pair &p) {
          return p.name == payload.to;
        });
    if (pair_itr == tos.end()) {
      _pledges_from.modify(pledges_from, get_self(), [&](auto &p) {
        p.users.emplace_back(name_pledge_pair{payload.to, pledge_id});
      });
    };
  }

  pledges_rel_table _pledges_to(get_self(), name("to").value, 1024, 64, false, false,
               VACCOUNTS_DELAYED_CLEANUP);
  const auto pledges_to = _pledges_to.find(payload.to.value);
  if (pledges_to == _pledges_to.end()) {
    _pledges_to.emplace(get_self(), [&](auto &p) {
      p.user = payload.to;
      p.users = std::vector<name_pledge_pair>{
          name_pledge_pair{payload.vaccount, pledge_id}};
    });
  } else {
    const auto &froms = pledges_to->users;
    const auto &pair_itr = std::find_if(
        froms.begin(), froms.end(),
        [&](const name_pledge_pair &p) { return p.name == payload.vaccount; });
    if (pair_itr == froms.end()) {
      _pledges_to.modify(pledges_to, get_self(), [&](auto &p) {
        p.users.emplace_back(name_pledge_pair{payload.vaccount, pledge_id});
      });
    };
  }
}

void phoenix::renewpledge(renewpledge_payload payload) {
  check_running();

  // just a sanity check
  // pledges_rel_table _pledges_to(get_self(), name("to").value, 1024, 64, false, false,
  //             VACCOUNTS_DELAYED_CLEANUP);
  // const auto pledges_to = _pledges_to.find(payload.to.value);
  // check(pledges_to != _pledges_to.end(), "user does not have any pledges");
  // const auto &froms = pledges_to->users;
  // const auto &pair_itr =
  //     std::find_if(froms.begin(), froms.end(), [&](const name_pledge_pair &p) {
  //       return p.pledge_id == payload.pledge_id;
  //     });
  // check(pair_itr != froms.end(), "pledge does not exist for receiver");
  // check(pair_itr->name == pledge->from,
  //       "pledgers differ in pledge and relations");

  const auto &pledge = _pledges.find(payload.pledge_id);
  check(pledge != _pledges.end(), "pledge does not exist");
  check(pledge->to == payload.to, "wrong subscriber account");
  check(pledge->from == payload.vaccount, "you are not the sender of this pledge");

  // if autorenew anyone can call renew and let pledge creator pay
  // otherwise require payers auth
  if (pledge->autorenew && payload.vaccount == payload.payer) {
    // don't require auth only in this case
  } else {
    require_vaccount(payload.payer);
  }

  const auto cycle_end = pledge->cycle_start + pledge->cycle_us;
  // if not paid yet skip the check
  if(pledge->paid) {
    check(eosio::current_time_point() > cycle_end,
          "current pledge is still running");
  }

  // update pledge info from new cycle
  _pledges.modify(pledge, get_self(), [&](auto &p) {
    p.cycle_start = eosio::current_time_point();
    p.cycle_us = p.next_cycle_us;
    // update prices here
    p.usd_value = p.next_usd_value;
    p.weosdt_quantity = p.next_weosdt_quantity;
    p.paid = true;

    if (payload.expected_usd_value > 0) {
      // if someone else pays, they might want to check that they are not front-run by creator updating their pledge
      check(fabs(payload.expected_usd_value - p.usd_value) < 0.01, "pledge value mismatch - was " + std::to_string(p.usd_value));
    }
  });
  pay_pledge(payload.payer, payload.pledge_id);
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

void phoenix::schedule_renewpledge(const pledge_info &pledge) {
  auto seconds =
      ((pledge.cycle_start + pledge.cycle_us) - eosio::current_time_point())
          .to_seconds() +
      1;

  renewpledge_payload inner_payload =
      renewpledge_payload{.vaccount = pledge.from, .to = pledge.to, .pledge_id = pledge.id, .payer= pledge.from, .expected_usd_value = -1.0};
  std::vector<char> packed_inner_payload = eosio::pack(inner_payload);

  timer_payload payload =
      timer_payload{.type = "renewpledge"_n, .payload = packed_inner_payload};
  std::vector<char> packed_payload = eosio::pack(payload);

  schedule_timer(name(pledge.id), packed_payload, seconds);
}
