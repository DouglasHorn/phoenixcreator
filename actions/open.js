const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const TOKEN_ACCOUNT = accounts[accounts.length - 1];
const CONTRACT_ACCOUNT = accounts[accounts.length - 2];

async function action() {
  try {
    await sendTransaction([
      {
        account: TOKEN_ACCOUNT,
        name: `open`,
        authorization: [
          {
            actor: CONTRACT_ACCOUNT,
            permission: `active`,
          },
        ],
        data: {
          owner: `tesese`,
          symbol: `9,WEOSDT`,
        },
      },
    ]);
    process.exit(0);
  } catch (error) {
    // ignore
    process.exit(1);
  }
}

action();
