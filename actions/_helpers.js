const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const { TextDecoder, TextEncoder } = require("text-encoding");
const { JsonRpc, Api, Serialize } = require("eosjs");
const { JsSignatureProvider } = require("eosjs/dist/eosjs-jssig");
const { getCreateKeys } = require("../extensions/helpers/key-utils");
const { getNetwork } = require("../extensions/tools/eos/utils");
const { encodeName } = require("../services/dapp-services-node/common");
const getDefaultArgs = require("../extensions/helpers/getDefaultArgs");
const fetch = require("node-fetch");
const ecc = require("eosjs-ecc");
let {
  PrivateKey,
  PublicKey,
  Signature,
  Aes,
  key_utils,
  config
} = require("eosjs-ecc");
const { getUrl } = require("../extensions/tools/eos/utils");
const { BigNumber } = require("bignumber.js");

var url = env.node_endpoint;
const rpc = new JsonRpc(url, { fetch });

const postData = (url = ``, data = {}) => {
  // Default options are marked with *
  return fetch(url, {
    method: "POST", // *GET, POST, PUT, DELETE, etc.
    mode: "cors", // no-cors, cors, *same-origin
    cache: "no-cache", // *default, no-cache, reload, force-cache, only-if-cached
    credentials: "same-origin", // include, *same-origin, omit
    headers: {
      // "Content-Type": "application/json",
      // "Content-Type": "application/x-www-form-urlencoded",
    },
    redirect: "follow", // manual, *follow, error
    referrer: "no-referrer", // no-referrer, *client
    body: JSON.stringify(data) // body data type must match "Content-Type" header
  }).then(response => response.json()); // parses response to JSON
};

const postVirtualTx = ({ contract_code, payload, wif }, signature) => {
  if (!signature) signature = ecc.sign(Buffer.from(payload, "hex"), wif);
  const public_key = PrivateKey.fromString(wif)
    .toPublic()
    .toString();

  return postData(`${url}/v1/dsp/accountless1/push_action`, {
    contract_code,
    public_key,
    payload,
    signature
  });
};

const toBound = (numStr, bytes) =>
  `${(new Array(bytes * 2 + 1).join("0") + numStr)
    .substring(numStr.length)
    .toUpperCase()}`;

const runTrx = async ({ contract_code, payload, wif }) => {
  let dataPayload = payload.data.payload;
  const vAccountUsed =
    dataPayload.vaccount ||
    dataPayload.author ||
    dataPayload.username ||
    dataPayload.from;
  if (!vAccountUsed)
    console.warn(
      `Could not determine the vAccount used to sign the transaction "${payload.name}" to get the nonce.`
    );

  const chainId = (await rpc.get_info()).chain_id;

  const signatureProvider = new JsSignatureProvider([]);
  const api = new Api({
    rpc,
    signatureProvider,
    // chainId:"",
    textDecoder: new TextDecoder(),
    textEncoder: new TextEncoder()
  });


  let buffer = new Serialize.SerialBuffer({
    textDecoder: new TextDecoder(),
    textEncoder: new TextEncoder()
  });

  const expiry = Math.floor(Date.now() / 1000) + 120; //two minute expiry
  buffer.pushNumberAsUint64(expiry);

  if (vAccountUsed) {
    try {
      const tableRes = await readVRAMData({
        contract: contract_code,
        key: vAccountUsed,
        table: "vkey",
        scope: contract_code
      });
      nonce = tableRes.row.nonce;
    } catch (e) {
      nonce = 0;
    }
  }
  // console.log(`nonce`, nonce)

  buffer.pushNumberAsUint64(nonce);

  let buf1 = buffer.getUint8Array(8);
  let buf2 = buffer.getUint8Array(8);
  let header =
    Serialize.arrayToHex(buf1) + Serialize.arrayToHex(buf2) + chainId;

  const response = await api.serializeActions([
    {
      account: contract_code,
      name: payload.name,
      authorization: [],
      data: payload.data
    }
  ]);
  const toName = name => {
    let res = new BigNumber(encodeName(name, true));
    res = toBound(res.toString(16), 8);
    return res;
  };

  buffer.pushVaruint32(response[0].data.length / 2);
  let varuintBytes = [];
  while (buffer.haveReadData()) varuintBytes.push(buffer.get());
  const serializedDataWithLength =
    Serialize.arrayToHex(varuintBytes) + response[0].data;

  // payloadSerialized corresponds to the actual vAccount action (like regaccount) https://github.com/liquidapps-io/zeus-sdk/blob/a3041e9177ffe4375fd8b944f4a10f74a447e406/boxes/groups/services/vaccounts-dapp-service/contracts/eos/dappservices/_vaccounts_impl.hpp#L50-L60
  // and is used as xvexec's payload vector<char>: https://github.com/liquidapps-io/zeus-sdk/blob/4e79122e42eeab50cf633097342b9c1fa00960c6/boxes/groups/services/vaccounts-dapp-service/services/vaccounts-dapp-service-node/index.js#L30
  // eosio::action fields to serialize https://github.com/EOSIO/eosio.cdt/blob/master/libraries/eosiolib/action.hpp#L194-L221
  const actionSerialized =
    "0000000000000000" + // account_name
    toName(payload.name) + // action_name
    // std::vector<permission_level> authorization https://github.com/EOSIO/eosio.cdt/blob/master/libraries/eosiolib/action.hpp#L107-L155
    "00" +
    // std::vector<char> data;
    serializedDataWithLength;

  const payloadSerialized = header + actionSerialized;

  const trxResult = await postVirtualTx({
    contract_code: contract_code,
    wif,
    payload: payloadSerialized
  });

  if (trxResult.error)
    throw new Error(
      `Error in vaccount transaction (${payload.name}): ${JSON.stringify(
        trxResult.error.details[0]
      )}`
    );

  return trxResult.result;
};

const readVRAMData = async ({ contract, key, table, scope }) => {
  const service = "ipfsservice1";
  const result = await postData(`${url}/v1/dsp/${service}/get_table_row`, {
    contract,
    scope,
    table,
    key
  });
  console.log(`${url}/v1/dsp/${service}/get_table_row`, {
    contract,
    scope,
    table,
    key
  })
  if (result.error) {
    console.error(`readVRAMData Error`, result.error);
    throw new Error(result.error);
  }
  return result;
};

const getNextTableKey = async ({ contract, table }) => {
  const res = await rpc.get_table_rows({
    json: true,
    code: contract,
    scope: table,
    table: ".vconfig",
    limit: 1,
  })

  // unitialized defaults to 0
  if(res.rows.length === 0) return 0
  return Number.parseInt(res.rows[0].next_available_key)
};

const delay = ms => new Promise(resolve => setTimeout(resolve, ms))

const vAccounts = [
  { name: `phoenix`, wif: PrivateKey.fromSeed(`phoenix`).toWif() },
  { name: `vacc1`, wif: PrivateKey.fromSeed(`vacc1`).toWif() },
  { name: `vacc2`, wif: PrivateKey.fromSeed(`vacc2`).toWif() },
];

module.exports = {
  runTrx,
  readVRAMData,
  getNextTableKey,
  vAccounts,
  delay
};
