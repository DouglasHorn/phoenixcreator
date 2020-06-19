const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const CONTRACT_ACCOUNT = accounts[accounts.length - 1];
console.log(CONTRACT_ACCOUNT)
async function action() {
  try {
    await sendTransaction({
      account: CONTRACT_ACCOUNT,
      name: `init`,
      authorization: [
        {
          actor: CONTRACT_ACCOUNT,
          permission: `active`
        }
      ],
      data: {
        phoenix_vaccount_pubkey: "EOS1111111111111111111111111111111114T1Anm",
      }
    });
    process.exit(0);
  } catch (error) {
    // ignore
    process.exit(1);
  }
}

action();
