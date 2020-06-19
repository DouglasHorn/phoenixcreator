const initEnvironment = require(`eosiac`);

const network = process.env.EOSIAC_ENV || `kylin`
const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

console.log(network)
const accounts = Object.keys(env.accounts);

const CONTRACT_ACCOUNT = network === `mainnet` ? `globaloracle` : accounts[1];

async function action() {
  try {
    const apiEndpoint = `min-api.cryptocompare.com/data/price?fsym=EOS&tsyms=USD&api_key=5c321a996c46d05d7b64cad200fee4718fbe2dac603daa4c0acedcffe6a1e465`;
    const uri = `https+json://USD/${apiEndpoint}`;

    await sendTransaction([{
      account: CONTRACT_ACCOUNT,
      name: `settokencfg`,
      authorization: [
        {
          actor: CONTRACT_ACCOUNT,
          permission: `active`
        }
      ],
      data: {
        symbolCode: `EOS`,
        uri: Buffer.from(uri, "utf8"),
        average_over: 288,
        fetch_interval_seconds: 5 * 60,
        active: true,
        initialPrice: 0,
        strategy: 1,
      }
    },
    // {
    //   account: CONTRACT_ACCOUNT,
    //   name: `settokencfg`,
    //   authorization: [
    //     {
    //       actor: CONTRACT_ACCOUNT,
    //       permission: `active`
    //     }
    //   ],
    //   data: {
    //     symbolCode: `PHOENIX`,
    //     uri: Buffer.from(``, "utf8"),
    //     average_over: 1,
    //     fetch_interval_seconds: 5 * 60,
    //     active: false,
    //     initialPrice: 0.02,
    //     strategy: 0,
    //   }
    // },
    ]);
    process.exit(0);
  } catch (error) {
    // ignore
    process.exit(1);
  }
}

action();
