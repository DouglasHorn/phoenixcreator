const initEnvironment = require(`eosiac`);
const { runTrx, readVRAMData, vAccounts } = require(`./_helpers`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const CONTRACT_ACCOUNT = accounts[accounts.length - 1];
const vAccount = vAccounts[1]
const linkedAccount = accounts[0]

async function action() {
  try {
    await sendTransaction({
      account: CONTRACT_ACCOUNT,
      name: "converttov",
      authorization: [
        {
          actor: linkedAccount,
          permission: `active`
        }
      ],
      data: {
        vaccount: vAccount.name,
        eos_account: linkedAccount,
        quantity: `0.1000 PHOENIX`,
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
