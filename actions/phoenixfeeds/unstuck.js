const initEnvironment = require(`eosiac`);

const network = process.env.EOSIAC_ENV || `kylin`
const { sendTransaction, env } = initEnvironment(
  network,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const CONTRACT_ACCOUNT = network === `mainnet` ? `globaloracle` : accounts[1];

async function action() {
  try {
    const apiEndpoint = `min-api.cryptocompare.com/data/price?fsym=EOS&tsyms=USD&api_key=5c321a996c46d05d7b64cad200fee4718fbe2dac603daa4c0acedcffe6a1e465`;
    const uri = `https+json://USD/${apiEndpoint}`;

    await sendTransaction([
      {
        account: CONTRACT_ACCOUNT,
        name: `unstuck`,
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
