const initEnvironment = require(`eosiac`);

const network = process.env.EOSIAC_ENV || `kylin`
const { sendTransaction, env } = initEnvironment(
  network,
  { verbose: true }
);

console.log(network)
const accounts = Object.keys(env.accounts);

const CONTRACT_ACCOUNT = network === `mainnet` ? `globaloracle` : accounts[1];

async function action() {
  try {
    await sendTransaction([
      {
        account: CONTRACT_ACCOUNT,
        name: `triggercron`,
        authorization: [
          {
            actor: CONTRACT_ACCOUNT,
            permission: `active`
          }
        ],
        data: {
          symbolCode: `EOS`
        }
      }
    ]);
    process.exit(0);
  } catch (error) {
    // ignore
    process.exit(1);
  }
}

action();
