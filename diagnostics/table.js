// @ts-check
const fs = require(`fs`);
const { requireBox } = require("@liquidapps/box-utils");
const { fstat } = require("fs");
const { parseTable } = requireBox(
  "ipfs-dapp-service/utils/ipfs-service/get-table"
);

const ENDPOINT = `https://waxdsp.maltablock.org:443`;
const CONTRACT_NAME = process.argv[process.argv.length - 2] || `phoenixgroup`;
const TABLE_NAME = process.argv[process.argv.length - 1] || `users`;

async function start() {
  const data = await parseTable(CONTRACT_NAME, TABLE_NAME, ENDPOINT, 1024);
  console.log(`Read ${data.length} users`);

  fs.writeFileSync(
    __dirname + `/${TABLE_NAME}.json`,
    JSON.stringify(data, null, 2)
  );
}

start().catch(console.error);
