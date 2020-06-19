const { runTrx, readVRAMData, vAccounts, getNextTableKey } = require(`./_helpers`)
const initEnvironment = require(`eosiac`);

const { env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const CONTRACT_ACCOUNT = accounts[0];


async function action() {
  try {
    const vAccount = vAccounts[2]
    // for some reason the .vconfig is not updated immediately only with a delay
    const nextKey = await getNextTableKey({ contract: CONTRACT_ACCOUNT, table: `posts`})
    console.log(`.vconfig nextKey`, nextKey)

    const resp = await runTrx({
      contract_code: CONTRACT_ACCOUNT,
      wif: vAccount.wif,
      payload: {
        name: "createpost",
        data: {
          payload: {
            author: vAccount.name,
            title: Buffer.from(`Title`, "utf8"),
            content: Buffer.from(`## Markdown header\n> content`, "utf8"),
            featured_image_url: Buffer.from(
              `https://blabla.test/14.jpg`,
              "utf8"
            ),
            encrypted: false,
            decrypt_for_usd: 0,
          }
        }
      }
    });

    const user = (await readVRAMData({
      contract: CONTRACT_ACCOUNT,
      key: vAccount.name,
      table: `users`,
      scope: CONTRACT_ACCOUNT
    })).row

    console.log(`post_indexes`, user.post_indexes)

    console.log((await readVRAMData({
      contract: CONTRACT_ACCOUNT,
      key: Number.parseInt(user.post_indexes.slice(-1)[0]),
      table: `posts`,
      scope: CONTRACT_ACCOUNT,
    })).row)
    
    process.exit(0);
  } catch (error) {
    // ignore
    console.error(error)
    process.exit(1);
  }
}

action();
