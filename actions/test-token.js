const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const CONTRACT_ACCOUNT = accounts[accounts.length - 2];
const TOKEN_ACCOUNT = accounts[accounts.length - 1];

async function action() {
  try {
    await sendTransaction({
      account: TOKEN_ACCOUNT,
      name: `test`,
      authorization: [
        {
          actor: TOKEN_ACCOUNT,
          permission: `active`
        }
      ],
      data: {
        vaccount: `phoenix`
      }
    });
    process.exit(0);
  } catch (error) {
    // ignore
    process.exit(1);
  }
}

action();
