const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const CONTRACT_ACCOUNT = accounts[accounts.length - 1];

// DEPRECATED: use init action instead to issue tokens
async function action() {
  try {
    await sendTransaction({
      account: CONTRACT_ACCOUNT,
      name: `issue`,
      authorization: [
        {
          actor: CONTRACT_ACCOUNT,
          permission: `active`
        }
      ],
      data: {
        to: "phoenix",
        quantity: "1000000000.0000 PHOENIX",
        memo: "issue 1/10 total supply of PHOENIX"
      }
    });
    process.exit(0);
  } catch (error) {
    // ignore
    process.exit(1);
  }
}

action();
