require('mocha');

const { requireBox } = require('@liquidapps/box-utils');
const { assert } = require("chai"); // Using Assert style
const { getCreateKeys } = requireBox('eos-keystore/helpers/key-utils');
const {
  getNetwork,
  getCreateAccount,
  getEos,
  getLocalDSPEos,
  getTestContract,
} = requireBox('seed-eos/tools/eos/utils');
let Eos = require("eosjs");
const getDefaultArgs = requireBox('seed-zeus-support/getDefaultArgs');
let { PrivateKey, PublicKey } = require("eosjs-ecc");

const artifacts = requireBox('seed-eos/tools/eos/artifacts');
const deployer = requireBox('seed-eos/tools/eos/deployer');
const {
  genAllocateDAPPTokens,
  readVRAMData
} = requireBox('dapp-services/tools/eos/dapp-services');
const { loadModels } = requireBox('seed-models/tools/models');
const fetch = require('node-fetch');
const { createClient } = requireBox("client-lib-base/client/dist/src/dapp-client-lib");
global.fetch = fetch;

const initHelpers = require("./phoenix.helpers");

const delay = ms => new Promise(res => setTimeout(res, ms));

let phoenixContractCodeName = "phoenix";
const phoenixCode = "phoenixacc";
const vAccount1 = `vaccount2`;
const vAccount2 = `vaccount3`;
const vAccount3 = `vaccount4`;
const vAccountPhoenix = `phoenix`;
let phoenixArtifact = artifacts.require(`./${phoenixContractCodeName}/`)

const endpoint = "http://localhost:13015";

const { runTrx, rpc } = initHelpers({ endpoint });

describe(`vAccounts Service Test Contract`, () => {
  let selectedNetwork = getNetwork(getDefaultArgs());
  let eosPhoenixContract;
  let eosTokenContract;
  let privateWif;
  let testAccountNames = [`testacc1`, `testacc2`];
  let testAccountKeys = [];

  const regVAccount = name =>
    runTrx({
      contract_code: phoenixCode,
      wif: privateWif,
      payload: {
        name: "regaccount",
        data: {
          payload: {
            vaccount: name
          }
        }
      }
    });

  before(done => {
    (async () => {
      try {
        privateWif = (await PrivateKey.randomKey());
        const publicKeyVAccount = privateWif.toPublic().toString()
        privateWif = privateWif.toWif()

        // deploy and generate + stake DAPP tokens
        const services = [`vaccounts`, `ipfs`, `cron`]; // await loadModels("dapp-services");
        console.log(`deploying`, phoenixCode, phoenixArtifact);
        let deployedContract = await deployer.deploy(
          phoenixArtifact,
          phoenixCode,
          // {
          //   ...getDefaultArgs(),
          //   // up stake to circumvent RAM issues
          //   stake: `100000.0000`
          // }
        );
        console.log(`deploy done`, phoenixCode);
        for (const service of services) {
          console.log(`allocating service "${service}"`);
          await genAllocateDAPPTokens(deployedContract, service); // service.name
        }
        console.log(`Allocating done`);

        // initialize vaccount service with chainId
        eosPhoenixContract = await getTestContract(phoenixCode)
        const chainId = (await rpc.get_info()).chain_id;

        await eosPhoenixContract.xvinit({
            chainid: chainId
        }, {
            authorization: `${phoenixCode}@active`,
        });
        let a = await eosPhoenixContract.init({
          phoenix_vaccount_pubkey: `EOS1111111111111111111111111111111114T1Anm`,
        }, {
            authorization: `${phoenixCode}@active`,
        });
        console.log(a)
        return;

        console.log(`registering vAccounts`);

        const dappClient = await createClient({ httpEndpoint: endpoint, fetch });
        const vaccClient = await dappClient.service(
          "vaccounts",
          phoenixCode
      );
        try {
          const res = await vaccClient.push_liquid_account_transaction(
              phoenixCode,
              privateWif,
              "regaccount",
              {
                  vaccount: vAccount1
              }
          );
          console.log(res)
          // await regVAccount(vAccount1);
          // await regVAccount(vAccount2);
          // await regVAccount(vAccount3);
        } catch (_err) {
          // ignore vaccount already exists error
          console.log(_err.message);
        }

        console.log(`tryin to read vram data`)
        let tableRes = await readVRAMData({
          contract: phoenixCode,
          key: vAccount1,
          table: `users`,
          scope: phoenixCode,
          keytype: `name`,
          keysize: 64,
        });
        console.log(tableRes.row);

        const eosToken = await getLocalDSPEos("eosio.token", getDefaultArgs());
        eosTokenContract = await eosToken.contract("eosio.token");
        await eosTokenContract.create("eosio.token", `10000000000.0000 EOS`, {
          authorization: [`eosio.token@active`]
        });
        await eosTokenContract.issue(
          {
            to: `eosio.token`,
            quantity: "10000000000.0000 EOS",
            memo: ``
          },
          {
            authorization: [`eosio.token@active`]
          }
        );

        // distribute EOS tokens to test accounts
        testAccountKeys = await Promise.all(
          testAccountNames.map(name => getCreateAccount(name))
        );
        await Promise.all(
          testAccountNames.map(async name => {
            return eosTokenContract.transfer(
              {
                from: `eosio.token`,
                to: name,
                quantity: "10.0000 EOS",
                memo: ``
              },
              {
                authorization: `eosio.token@active`
              }
            );
          })
        );

        done();
      } catch (e) {
        done(e);
      }
    })();
  });

  it("regs account and can login", done => {
    (async () => {
      try {
        const vAccounts = [vAccount1, vAccount2, vAccount3];
        // accounts were created before already
        for (const name of vAccounts) {
          let tableRes = await readVRAMData({
            contract: phoenixCode,
            key: name,
            table: `users`,
            scope: phoenixCode,
            keytype: `name`,
            keysize: 64,
          });
          assert.equal(tableRes.row.username, name, "wrong user name");
          console.log(`user ${name} exists`);
        }

        done();
      } catch (e) {
        console.error(e)
        done(e);
      }
    })();
  });

  it("updates user info", done => {
    (async () => {
      try {
        const profileInfo = {
          displayName: `hello`,
          logoSrc: `logo`,
          headerSrc: `header`,
          description: `description`,
          website: `https://phoenix.url`
        };
        await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "updateuser",
            data: {
              payload: {
                username: vAccount1,
                new_profile_info: profileInfo
              }
            }
          }
        });

        let tableRes = await readVRAMData({
          contract: phoenixCode,
          key: vAccount1,
          table: `users`,
          scope: phoenixCode
        });
        assert.deepEqual(
          tableRes.row.profile_info,
          profileInfo,
          "wrong user profile Info"
        );

        done();
      } catch (e) {
        done(e);
      }
    })();
  });

  it("can create / edit / remove subscription tiers", done => {
    (async () => {
      try {
        const firstTier = {
          title: `Supporter`,
          description: `Thanks for your support`,
          usd_value: 5.0,
        };
        const secondTier = {
          title: `Fan`,
          description: `Thanks for idolizing`,
          usd_value: 10.0,
        };
        await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "updatetiers",
            data: {
              payload: {
                username: vAccount1,
                new_tiers: [firstTier],
              }
            }
          }
        });
        await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "updatetiers",
            data: {
              payload: {
                username: vAccount1,
                new_tiers: [firstTier, secondTier],
              }
            }
          }
        });

        let tableRes = await readVRAMData({
          contract: phoenixCode,
          key: vAccount1,
          table: `users`,
          scope: phoenixCode
        });
        assert.deepEqual(
          tableRes.row.tiers,
          [firstTier, secondTier],
          "wrong initial first and second tier"
        );

        // update first tier
        const updatedFirstTier = {
          ...firstTier,
          title: `Backer`,
          usd_value: 1.0
        }
        await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "updatetiers",
            data: {
              payload: {
                username: vAccount1,
                new_tiers: [updatedFirstTier, secondTier],
              }
            }
          }
        });
        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: vAccount1,
          table: `users`,
          scope: phoenixCode
        });
        assert.deepEqual(
          tableRes.row.tiers,
          [updatedFirstTier, secondTier],
          "wrong updated first tier"
        );

        // delete first tier
        await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "updatetiers",
            data: {
              payload: {
                username: vAccount1,
                new_tiers: [secondTier],
              }
            }
          }
        });
        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: vAccount1,
          table: `users`,
          scope: phoenixCode
        });
        assert.deepEqual(
          tableRes.row.tiers,
          [secondTier],
          "wrong tiers after removing first tier"
        );

        done();
      } catch (e) {
        done(e);
      }
    })();
  });

  it("can transfer PHOENIX to vaccounts", done => {
    (async () => {
      try {
        await eosPhoenixContract.transfer(
          {
            from: vAccountPhoenix,
            to: vAccount1,
            quantity: "20.0000 PHOENIX",
            memo: ``
          },
          {
            authorization: [`${phoenixCode}@active`]
          }
        );

        let tableRes = await readVRAMData({
          contract: phoenixCode,
          key: `PHOENIX`,
          table: `accounts`,
          scope: vAccount1
        });
        console.log(`accounts row`, tableRes.row);
        assert(tableRes.row.balance === `20.0000 PHOENIX`, "wrong balance");

        await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "transferv",
            data: {
              payload: {
                from: vAccount1,
                to: vAccount2,
                quantity: "10.0000 PHOENIX",
                memo: ``
              }
            }
          }
        });

        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: `PHOENIX`,
          table: `accounts`,
          scope: vAccount2
        });
        console.log(`accounts row`, tableRes.row);
        assert(tableRes.row.balance === `10.0000 PHOENIX`, "wrong balance");

        done();
      } catch (e) {
        done(e);
      }
    })();
  });

  it("can create posts", done => {
    (async () => {
      try {
        let res = await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "createpost",
            data: {
              payload: {
                author: vAccount1,
                title: Buffer.from(`Title`, "utf8"),
                content: Buffer.from(postText, "utf8"),
                featured_image_url: Buffer.from(
                  `https://blabla.test/14.jpg`,
                  "utf8"
                ),
                meta: ``,
                encrypted: false,
                decrypt_for_usd: 0,
                post_key: [],
                expected_id: 0,
              }
            }
          }
        });

        let outputLines = res.processed.action_traces[0].console;
        // console.log(outputLines);

        let user1 = (await readVRAMData({
          contract: phoenixCode,
          key: vAccount1,
          table: `users`,
          scope: phoenixCode
        })).row;
        console.log(user1)
        assert.equal(user1.post_indexes.length, 1, `wrong post indexes length user 1`)
        assert.equal(user1.post_indexes[0], 0, `wrong post indexes for user 1`)

        let tableRes = await readVRAMData({
          contract: phoenixCode,
          key: Number.parseInt(user1.post_indexes[0]),
          table: `posts`,
          scope: phoenixCode
        });
        console.log(tableRes)
        assert(
          Buffer.from(tableRes.row.title).toString(`utf8`) === `Title`,
          "wrong title for post 1"
        );

        // post 2
        res = await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "createpost",
            data: {
              payload: {
                author: vAccount2,
                title: Buffer.from(`Vaccount 2`, "utf8"),
                content: Buffer.from(`## Vaccount 2\n> content`, "utf8"),
                featured_image_url: Buffer.from(
                  `https://blabla.test/14.jpg`,
                  "utf8"
                ),
                meta: ``,
                encrypted: false,
                decrypt_for_usd: 0,
                post_key: [],
                expected_id: 1,
              }
            }
          }
        });

        outputLines = res.processed.action_traces[0].console;
        // console.log(outputLines);

        let user2 = (await readVRAMData({
          contract: phoenixCode,
          key: vAccount2,
          table: `users`,
          scope: phoenixCode
        })).row;
        assert.equal(user2.post_indexes.length, 1, `wrong post indexes length user 1`)
        assert.equal(user2.post_indexes[0], 1, `wrong post indexes for user 1`)

        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: Number.parseInt(user2.post_indexes[0]),
          table: `posts`,
          scope: phoenixCode
        });
        console.log(tableRes)
        assert(
          Buffer.from(tableRes.row.title).toString(`utf8`) === `Vaccount 2`,
          "wrong title for post 2"
        );

        done();
      } catch (e) {
        done(e);
      }
    })();
  });

  it("can update the post", done => {
    (async () => {
      try {
        const res = await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "updatepost",
            data: {
              payload: {
                author: vAccount1,
                id: 0,
                title: Buffer.from(`Updated Title`, "utf8"),
                content: Buffer.from(`## Markdown header\n> content`, "utf8"),
                featured_image_url: Buffer.from(
                  `https://blabla.test/14.jpg`,
                  "utf8"
                ),
                meta: ``,
                encrypted: false,
                decrypt_for_usd: 0,
                post_key: [],
                delete_post: false,
              }
            }
          }
        });

        let outputLines = res.processed.action_traces[0].console;
        // console.log(`output`, outputLines);

        const tableRes = await readVRAMData({
          contract: phoenixCode,
          key: 0,
          table: `posts`,
          scope: phoenixCode
        });
        assert(
          Buffer.from(tableRes.row.title).toString(`utf8`) === `Updated Title`,
          "wrong title"
        );

        done();
      } catch (e) {
        done(e);
      }
    })();
  });

  it("can follow users", done => {
    (async () => {
      try {
        let res = await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "follow",
            data: {
              payload: {
                from: vAccount1,
                follows: [vAccount2, vAccount3],
                unfollows: []
              }
            }
          }
        });

        let outputLines = res.processed.action_traces[0].console;
        // console.log(`output`, outputLines);

        let tableRes = await readVRAMData({
          contract: phoenixCode,
          key: vAccount1,
          table: `followsfrom`,
          scope: phoenixCode
        });
        assert.deepEqual(
          tableRes.row.tos,
          [vAccount2, vAccount3],
          "wrong follows from vAccount1"
        );
        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: vAccount2,
          table: `followsto`,
          scope: phoenixCode
        });
        assert.deepEqual(
          tableRes.row.froms,
          [vAccount1],
          "wrong follows to vAccount2"
        );
        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: vAccount3,
          table: `followsto`,
          scope: phoenixCode
        });
        assert.deepEqual(
          tableRes.row.froms,
          [vAccount1],
          "wrong follows to vAccount3"
        );

        /**
         * Unfollow
         */
        res = await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "follow",
            data: {
              payload: {
                from: vAccount1,
                follows: [],
                unfollows: [vAccount2]
              }
            }
          }
        });
        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: vAccount1,
          table: `followsfrom`,
          scope: phoenixCode
        });
        assert.deepEqual(
          tableRes.row.tos,
          [vAccount3],
          "wrong follows from vAccount1 after unfollow"
        );
        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: vAccount2,
          table: `followsto`,
          scope: phoenixCode
        });
        assert.deepEqual(
          tableRes.row.froms,
          [],
          "wrong follows to vAccount2 after unfollow"
        );

        done();
      } catch (e) {
        done(e);
      }
    })();
  });

  it("can link an eos account", done => {
    (async () => {
      const linkedAccount = testAccountNames[0];
      try {
        let res = await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "linkaccount",
            data: {
              payload: {
                from: vAccount1,
                account: linkedAccount
              }
            }
          }
        });

        let outputLines = res.processed.action_traces[0].console;
        // console.log(`output`, outputLines);

        let tableRes = await readVRAMData({
          contract: phoenixCode,
          key: vAccount1,
          table: `users`,
          scope: phoenixCode
        });
        assert.deepEqual(
          tableRes.row.linked_name,
          linkedAccount,
          "wrong linked account"
        );

        done();
      } catch (e) {
        done(e);
      }
    })();
  });

  it("can deposit and withdraw EOS", done => {
    (async () => {
      const linkedAccount = testAccountNames[0];
      try {
        await eosTokenContract.transfer(
          linkedAccount,
          phoenixCode,
          `5.0000 EOS`,
          `deposit ${vAccount1}`,
          {
            authorization: `${linkedAccount}@active`,
            keyProvider: [testAccountKeys[0].active.privateKey]
          }
        );

        let tableRes = await readVRAMData({
          contract: phoenixCode,
          key: `EOS`,
          table: `accounts`,
          scope: vAccount1,
        });
        assert.equal(tableRes.row.balance, `5.0000 EOS`, "wrong EOS balance after deposit");

        let res = await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "withdraw",
            data: {
              payload: {
                from: vAccount1,
                to_eos_account: linkedAccount,
                quantity: `2.0000 EOS`
              }
            }
          }
        });

        let outputLines = res.processed.action_traces[0].console;
        // console.log(`output`, outputLines);

        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: `EOS`,
          table: `accounts`,
          scope: vAccount1,
        });
        assert.equal(tableRes.row.balance, `3.0000 EOS`, "wrong EOS balance after withdraw");


        // overdraw balance should not be possible
        let failed = false;
        try {
          res = await runTrx({
            contract_code: phoenixCode,
            wif: privateWif,
            payload: {
              name: "withdraw",
              data: {
                payload: {
                  from: vAccount1,
                  to_eos_account: linkedAccount,
                  quantity: `3.0001 EOS`
                }
              }
            }
          });
          failed = Boolean(res.error);
        } catch (err) {
          failed = true;
        }

        assert.equal(failed, true, "should not be able to overdraw balance");

        done();
      } catch (e) {
        done(e);
      }
    })();
  });

  it("can create a pledge to someone", done => {
    (async () => {
      try {
        const pledge = {
          from: vAccount1,
          to: vAccount2,
          usd_value: 7.0,
          eos_quantity: `1.0000 EOS`,
          phoenix_quantity: `1.0000 PHOENIX`,
          autorenew: true,
          next_delete: false
        };
        let res = await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "pledge",
            data: {
              payload: pledge
            }
          }
        });

        let tableRes = await readVRAMData({
          contract: phoenixCode,
          key: 0,
          table: `pledges`,
          scope: phoenixCode
        });
        assert.deepInclude(tableRes.row, pledge, "wrong pledge info");

        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: vAccount1,
          table: `pledgesfrom`,
          scope: phoenixCode
        });
        assert.deepEqual(
          tableRes.row,
          { from: vAccount1, tos: [{ name: vAccount2, pledge_id: "0" }] },
          "wrong pledgesfrom info"
        );

        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: vAccount2,
          table: `pledgesto`,
          scope: phoenixCode
        });
        assert.deepEqual(
          tableRes.row,
          { to: vAccount2, froms: [{ name: vAccount1, pledge_id: "0" }] },
          "wrong pledgesto info"
        );

        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: `EOS`,
          table: `accounts`,
          scope: vAccount1,
        });
        assert.equal(
          tableRes.row.balance,
          `2.0000 EOS`,
          "wrong EOS balance for pledger after pledge"
        );
        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: `PHOENIX`,
          table: `accounts`,
          scope: vAccount1
        });
        assert.equal(
          tableRes.row.balance,
          `9.0000 PHOENIX`,
          "wrong PHOENIX balance for pledger after pledge"
        );

        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: `EOS`,
          table: `accounts`,
          scope: vAccount2,
        });
        assert.equal(
          tableRes.row.balance,
          `1.0000 EOS`,
          "wrong EOS balance for receiver after pledge"
        );
        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: `PHOENIX`,
          table: `accounts`,
          scope: vAccount2
        });
        assert.equal(
          tableRes.row.balance,
          `11.0000 PHOENIX`,
          "wrong PHOENIX balance for receiver after pledge"
        );

        done();
      } catch (e) {
        done(e);
      }
    })();
  });

  it("can update a pledge", done => {
    (async () => {
      try {
        const pledge = {
          from: vAccount1,
          to: vAccount2,
          usd_value: 5.9361,
          eos_quantity: `1.1337 EOS`,
          phoenix_quantity: `0.1338 PHOENIX`,
          autorenew: false,
          next_delete: true
        };
        let res = await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "pledge",
            data: {
              payload: pledge
            }
          }
        });

        let tableRes = await readVRAMData({
          contract: phoenixCode,
          key: 0,
          table: `pledges`,
          scope: phoenixCode
        });
        assert.deepInclude(
          tableRes.row,
          {
            from: vAccount1,
            to: vAccount2,
            // values from previous create pledge test
            usd_value: 7.0,
            eos_quantity: `1.0000 EOS`,
            phoenix_quantity: `1.0000 PHOENIX`,
            // updates values from here
            next_eos_quantity: `1.1337 EOS`,
            next_phoenix_quantity: `0.1338 PHOENIX`,
            autorenew: false,
            next_delete: true
          },
          "wrong pledge info"
        );
        assert.closeTo(
          tableRes.row.next_usd_value,
          5.9361,
          0.00001,
          "wrong next_usd_value pledge info"
        );

        done();
      } catch (e) {
        done(e);
      }
    })();
  });

  it("can renew a pledge", done => {
    (async () => {
      try {
        const pledge = {
          from: vAccount1,
          to: vAccount3,
          usd_value: 7.0,
          eos_quantity: `1.0000 EOS`,
          phoenix_quantity: `1.0000 PHOENIX`,
          autorenew: true,
          next_delete: false
        };
        let res = await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "pledge",
            data: {
              payload: pledge
            }
          }
        });
        // update pledge
        res = await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "pledge",
            data: {
              payload: {
                ...pledge,
                usd_value: 1.4,
                eos_quantity: `0.2000 EOS`,
                phoenix_quantity: `0.2000 PHOENIX`
              }
            }
          }
        });

        // pledge should now be be at the end of its cycle
        await delay(6000);

        res = await runTrx({
          contract_code: phoenixCode,
          wif: privateWif,
          payload: {
            name: "renewpledge",
            data: {
              payload: {
                to: vAccount3,
                pledge_id: 1
              }
            }
          }
        });

        let outputLines = res.processed.action_traces[0].console;
        // console.log(`output`, outputLines);

        let tableRes = await readVRAMData({
          contract: phoenixCode,
          key: `EOS`,
          table: `accounts`,
          scope: vAccount3,
        });
        assert.closeTo(
          Number.parseFloat(tableRes.row.balance.split(` `)[0]),
          1.2,
          0.0001,
          "wrong EOS balance after renew pledge"
        );
        tableRes = await readVRAMData({
          contract: phoenixCode,
          key: `PHOENIX`,
          table: `accounts`,
          scope: vAccount3
        });
        assert.closeTo(
          Number.parseFloat(tableRes.row.balance.split(` `)[0]),
          1.2,
          0.0001,
          "wrong PHOENIX balance for receiver after pledge"
        );

        done();
      } catch (e) {
        done(e);
      }
    })();
  });
});

const postText = `<!-- start slipsum code -->

My money's in that office, right? If she start giving me some bullshit about it ain't there, and we got to go someplace else and get it, I'm gonna shoot you in the head then and there. Then I'm gonna shoot that bitch in the kneecaps, find out where my goddamn money is. She gonna tell me too. Hey, look at me when I'm talking to you, motherfucker. You listen: we go in there, and that nigga Winston or anybody else is in there, you the first motherfucker to get shot. You understand?

You think water moves fast? You should see ice. It moves like it has a mind. Like it knows it killed the world once and got a taste for murder. After the avalanche, it took us a week to climb out. Now, I don't know exactly when we turned on each other, but I know that seven of us survived the slide... and only five made it out. Now we took an oath, that I'm breaking now. We said we'd say it was the snow that killed the other two, but it wasn't. Nature is lethal but it doesn't hold a candle to man.

You think water moves fast? You should see ice. It moves like it has a mind. Like it knows it killed the world once and got a taste for murder. After the avalanche, it took us a week to climb out. Now, I don't know exactly when we turned on each other, but I know that seven of us survived the slide... and only five made it out. Now we took an oath, that I'm breaking now. We said we'd say it was the snow that killed the other two, but it wasn't. Nature is lethal but it doesn't hold a candle to man.

Now that there is the Tec-9, a crappy spray gun from South Miami. This gun is advertised as the most popular gun in American crime. Do you believe that shit? It actually says that in the little book that comes with it: the most popular gun in American crime. Like they're actually proud of that shit. 

Now that there is the Tec-9, a crappy spray gun from South Miami. This gun is advertised as the most popular gun in American crime. Do you believe that shit? It actually says that in the little book that comes with it: the most popular gun in American crime. Like they're actually proud of that shit. 

Normally, both your asses would be dead as fucking fried chicken, but you happen to pull this shit while I'm in a transitional period so I don't wanna kill you, I wanna help you. But I can't give you this case, it don't belong to me. Besides, I've already been through too much shit this morning over this case to hand it over to your dumb ass.

Normally, both your asses would be dead as fucking fried chicken, but you happen to pull this shit while I'm in a transitional period so I don't wanna kill you, I wanna help you. But I can't give you this case, it don't belong to me. Besides, I've already been through too much shit this morning over this case to hand it over to your dumb ass.

You think water moves fast? You should see ice. It moves like it has a mind. Like it knows it killed the world once and got a taste for murder. After the avalanche, it took us a week to climb out. Now, I don't know exactly when we turned on each other, but I know that seven of us survived the slide... and only five made it out. Now we took an oath, that I'm breaking now. We said we'd say it was the snow that killed the other two, but it wasn't. Nature is lethal but it doesn't hold a candle to man.

You think water moves fast? You should see ice. It moves like it has a mind. Like it knows it killed the world once and got a taste for murder. After the avalanche, it took us a week to climb out. Now, I don't know exactly when we turned on each other, but I know that seven of us survived the slide... and only five made it out. Now we took an oath, that I'm breaking now. We said we'd say it was the snow that killed the other two, but it wasn't. Nature is lethal but it doesn't hold a candle to man.

Now that there is the Tec-9, a crappy spray gun from South Miami. This gun is advertised as the most popular gun in American crime. Do you believe that shit? It actually says that in the little book that comes with it: the most popular gun in American crime. Like they're actually proud of that shit. 

<!-- end slipsum code -->`