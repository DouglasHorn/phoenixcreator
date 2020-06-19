const { runTrx, readVRAMData, vAccounts } = require(`./_helpers`);
const initEnvironment = require(`eosiac`);

const { env } = initEnvironment(process.env.EOSIAC_ENV || `kylin`, {
  verbose: true
});

const accounts = Object.keys(env.accounts);

const CONTRACT_ACCOUNT = accounts[accounts.length - 1];

async function action() {
  const userVAccounts = vAccounts.filter(({ name }) => name !== `phoenix`)
  for (const vAccount of userVAccounts.slice(1)) {
    console.log(vAccount.name);

    try {
      await runTrx({
        contract_code: CONTRACT_ACCOUNT,
        wif: vAccount.wif,
        payload: {
          name: "regaccount",
          data: {
            payload: {
              vaccount: vAccount.name
            }
          }
        }
      });

      // await runTrx({
      //   contract_code: CONTRACT_ACCOUNT,
      //   wif: vAccount.wif,
      //   payload: {
      //     name: "login",
      //     data: {
      //       payload: {
      //         username: vAccount.name
      //       }
      //     }
      //   }
      // });

      // try {
      //   console.log(
      //     `finished creating ${vAccount.name}`,
      //     await readVRAMData({
      //       contract: CONTRACT_ACCOUNT,
      //       key: vAccount.name,
      //       table: `users`,
      //       scope: CONTRACT_ACCOUNT
      //     })
      //   );
      // } catch {}
      
    } catch (error) {
      console.error(error);
    }
  }
  process.exit(0);
}

action();
