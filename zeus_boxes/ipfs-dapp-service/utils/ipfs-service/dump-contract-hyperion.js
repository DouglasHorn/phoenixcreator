const { requireBox } = require("@liquidapps/box-utils");
const fetch = require("node-fetch");
const fs = require("fs");
const { Serialize } = require("eosjs");
const querystring = require("querystring");
const multihash = require('multihashes');
const sha256 = require('js-sha256').sha256;

const contractAccount = process.env.CONTRACT || "";

const hyperionEndpoint =
  process.env.HYPERION_ENDPOINT || "https://api-wax.maltablock.org";
if (!contractAccount) throw new Error(`must set CONTRACT`);

const hashData256 = (data) => {
    var hash = sha256.create();
    hash.update(data);
    return hash.hex();
};
// zeus_boxes/storage-dapp-service/services/storage-dapp-service-node/common.js
const dataToIpfsHash = (dataBuffer) => {
    const hash = hashData256(dataBuffer);
    const paddedHash = "01551220" + hash;
    const bytes = Buffer.from(paddedHash, 'hex');
    const address = multihash.toB58String(bytes);
    return 'ipfs://z' + address;
}

async function run() {
  let skip = 0;
  const LIMIT = 100;
  const allCommits = [];
  while (true) {
    // limit=2&skip=3&code=phoenixgroup&scope=phoenixgroup&table=ipfsentry
    const qs = querystring.encode({
      code: contractAccount,
      scope: contractAccount,
      table: `ipfsentry`,
      skip,
      limit: LIMIT,
    });
    const url = `${hyperionEndpoint}/v2/history/get_deltas?${qs}`;
    console.log(`fetching `, url);
    const response = await fetch(url).then((resp) => resp.json());
    const commits = response.deltas;
    skip += commits.length;
    allCommits.push(
      ...commits.map((c) => ({
          timestamp: c.timestamp,
          data: Buffer.from(c.data.data, `hex`).toString(`base64`),
          hash: dataToIpfsHash(Buffer.from(c.data.data, `hex`))
        }))
    );

    if (commits.length === 0) break;

    console.log(`${skip} commits: ${commits[commits.length - 1].timestamp}`);
  }
  console.log(`Done. Read ${skip} commits`);
  fs.writeFileSync(
    `${contractAccount}_ipfsentry_${Date.now()}.json`,
    JSON.stringify(allCommits)
  );
}

run().catch((error) => {
  console.error(error.stack);
});
