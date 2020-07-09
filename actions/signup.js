const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const ROOT_ACCOUNT = accounts[0];
const CONTRACT_ACCOUNT = accounts[accounts.length - 2];

async function action() {
  try {
    await sendTransaction({
      account: CONTRACT_ACCOUNT,
      name: `signup`,
      authorization: [
        {
          actor: CONTRACT_ACCOUNT,
          permission: `active`,
        },
      ],
      data: {
        vaccount: `tester1.phnx`,
        pubkey: `EOS8Rf2CuK26Lj5Xfmc5x7xSupoidfJLt1HjDwuTJEWUWNj6Y9Qsb`,
      },
    });
  } catch (error) {
    console.error(error);
  }
  process.exit(0);
}

action();
