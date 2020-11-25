const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const ROOT_ACCOUNT = accounts[0];
const PHOENIX_CONTRACT = accounts[accounts.length - 2];
const PHOENIXTOKEN_CONTRACT = accounts[accounts.length - 1];

// exclude phoenix
const allVAccounts = [`payments`]

async function action() {
  for (const vAccount of allVAccounts) {
    try {
      await sendTransaction({
        account: `weosdttokens`,
        name: `transfer`,
        authorization: [
          {
            actor: `cmichelonwax`,
            permission: `ops`
          }
        ],
        data: {
          from: `cmichelonwax`,
          to: PHOENIXTOKEN_CONTRACT,
          quantity: `9.000000000 WEOSDT`,
          memo: `deposit ${vAccount}`,
        }
      });
      // console.log(`${vAccount} balance:`, await readVRAMData({
      //   contract: PHOENIX_CONTRACT,
      //   key: `WEOSDT`,
      //   table: `accounts`,
      //   scope: vAccount
      // }));
    } catch (error) {
      console.error(error);
    }
  }
  process.exit(0);
}

action();
