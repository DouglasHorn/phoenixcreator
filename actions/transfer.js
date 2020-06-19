const { runTrx, readVRAMData, vAccounts } = require(`./_helpers`);
const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const ROOT_ACCOUNT = accounts[0];
const CONTRACT_ACCOUNT = accounts[accounts.length - 1];

// exclude phoenix
const allVAccounts = [
  ...vAccounts,
//  { name: `frontendfron` },
].filter(({ name }) => name !== `phoenix`)

async function action() {
  for (const vAccount of allVAccounts) {
    try {
      await sendTransaction({
        account: CONTRACT_ACCOUNT,
        name: `transfer`,
        authorization: [
          {
            actor: CONTRACT_ACCOUNT,
            permission: `active`
          }
        ],
        data: {
          from: vAccounts[0].name,
          to: vAccount.name,
          quantity: `1000.0000 PHOENIX`,
          memo: "some phoenix to get started"
        }
      });
      await sendTransaction({
        account: `eosio.token`,
        name: `transfer`,
        authorization: [
          {
            actor: ROOT_ACCOUNT,
            permission: `active`
          }
        ],
        data: {
          from: ROOT_ACCOUNT,
          to: CONTRACT_ACCOUNT,
          quantity: `10.0000 EOS`,
          memo: `deposit ${vAccount.name}`,
        }
      });
      console.log(`${vAccount.name} balance:`, await readVRAMData({
        contract: CONTRACT_ACCOUNT,
        key: `PHOENIX`,
        table: `accounts`,
        scope: vAccount.name
      }));
      console.log(`${vAccount.name} balance:`, await readVRAMData({
        contract: CONTRACT_ACCOUNT,
        key: `EOS`,
        table: `accounts`,
        scope: vAccount.name
      }));
    } catch (error) {
      console.error(error);
    }
  }
  process.exit(0);
}

action();
