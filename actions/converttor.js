const initEnvironment = require(`eosiac`);
const { runTrx, readVRAMData, vAccounts } = require(`./_helpers`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const CONTRACT_ACCOUNT = accounts[accounts.length - 1];
const PHOENIXSWAP_ACCOUNT = accounts[2];
const vAccount = vAccounts[1]
const linkedAccount = accounts[0]

async function action() {
  try {
    await sendTransaction({
      account: PHOENIXSWAP_ACCOUNT,
      name: `open`,
      authorization: [
        {
          actor: CONTRACT_ACCOUNT,
          permission: `active`
        }
      ],
      data: {
        owner: linkedAccount,
        symbol: `4,PHOENIX`,
        ram_payer: CONTRACT_ACCOUNT,
      }
    });

    await runTrx({
      contract_code: CONTRACT_ACCOUNT,
      wif: vAccount.wif,
      payload: {
        name: "converttor",
        data: {
          payload: {
            vaccount: vAccount.name,
            eos_account: linkedAccount,
            quantity: `0.1000 PHOENIX`,
          }
        }
      }
    });

    console.log(`${vAccount.name} balance:`, await readVRAMData({
      contract: CONTRACT_ACCOUNT,
      key: `PHOENIX`,
      table: `accounts`,
      scope: vAccount.name
    }));

    process.exit(0);
  } catch (error) {
    console.error(error)
    // ignore
    process.exit(1);
  }
}

action();
