const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const ROOT = accounts[0];

const CONTRACT = accounts[accounts.length - 2];
const TOKEN = accounts[accounts.length - 1];

async function action() {
  try {
    await sendTransaction([
      {
        account: CONTRACT,
        name: `xvinit`,
        authorization: [
          {
            actor: CONTRACT,
            permission: `active`
          }
        ],
        data: {
          chainid: env.chain_id
        }
      },
      {
        account: TOKEN,
        name: `xvinit`,
        authorization: [
          {
            actor: TOKEN,
            permission: `active`
          }
        ],
        data: {
          host: CONTRACT
        }
      }
    ]
    );

    process.exit(0);
  } catch (error) {
    // ignore
    process.exit(1);
  }
}

action();
