const { runTrx, readVRAMData, vAccounts, getNextTableKey, delay } = require(`./_helpers`)
const initEnvironment = require(`eosiac`);

const { env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const CONTRACT_ACCOUNT = accounts[accounts.length - 1]


async function action() {
  try {
    const vAccount1 = vAccounts[1]
    const vAccount2 = vAccounts[2]
    // for some reason the .vconfig is not updated immediately only with a delay
    const nextKey = 0
    try {
      nextKey = await getNextTableKey({ contract: CONTRACT_ACCOUNT, table: `pledges`})
    } catch {}

    console.log(nextKey)
    const pledge = {
      from: vAccount1.name,
      to: vAccount2.name,
      eos_quantity: `1.0000 EOS`,
      phoenix_quantity: `1.0000 PHOENIX`,
      usd_value: 3.306333,
      autorenew: false,
      next_delete: false
    };
    // const resp = await runTrx({
    //   contract_code: CONTRACT_ACCOUNT,
    //   wif: vAccount1.wif,
    //   payload: {
    //     name: "pledge",
    //     data: {
    //       payload: pledge,
    //     }
    //   }
    // });
    // console.log(resp)

    // takes some time until DSP has indexed it?
    await delay(10000)
    const pledgesFrom = (await readVRAMData({
      contract: CONTRACT_ACCOUNT,
      key: vAccount1.name,
      table: `pledgesfrom`,
      scope: CONTRACT_ACCOUNT
    })).row
    console.log(await readVRAMData({
      contract: CONTRACT_ACCOUNT,
      key: Number.parseInt(pledgesFrom.tos[0].pledge_id),
      table: `pledges`,
      scope: CONTRACT_ACCOUNT
    }))
    console.log(`${vAccount1.name} balance:`, await readVRAMData({
      contract: CONTRACT_ACCOUNT,
      key: `PHOENIX`,
      table: `accounts`,
      scope: vAccount1.name
    }));
    console.log(`${vAccount2.name} balance:`, await readVRAMData({
      contract: CONTRACT_ACCOUNT,
      key: `PHOENIX`,
      table: `accounts`,
      scope: vAccount2.name
    }));
    console.log(`${vAccount1.name} balance:`, await readVRAMData({
      contract: CONTRACT_ACCOUNT,
      key: `EOS`,
      table: `accounts`,
      scope: vAccount1.name
    }));
    console.log(`${vAccount2.name} balance:`, await readVRAMData({
      contract: CONTRACT_ACCOUNT,
      key: `EOS`,
      table: `accounts`,
      scope: vAccount2.name
    }));
    
    process.exit(0);
  } catch (error) {
    // ignore
    console.error(error)
    process.exit(1);
  }
}

action();
