const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const ROOT = accounts[0];

const STAKE_ACC = `phoenixaaa`;

const services = [
  {
    provider: `heliosselene`,
    package: `liquidaccts2`,
    service: `accountless1`
  },
  {
    provider: `heliosselene`,
    package: `ipfs1`,
    service: `ipfsservice1`
  },
  {
    provider: `heliosselene`,
    package: `oracleservic`,
    service: `oracleservic`
  },
  {
    provider: `heliosselene`,
    package: `cronservices`,
    service: `cronservices`
  },
  {
    provider: `uuddlrlrbass`,
    package: `liquidaccts1`,
    service: `accountless1`
  },
  {
    provider: `uuddlrlrbass`,
    package: `ipfs2`,
    service: `ipfsservice1`
  },
  {
    provider: `uuddlrlrbass`,
    package: `oracleservi2`,
    service: `oracleservic`
  },
  {
    provider: `uuddlrlrbass`,
    package: `cronservice2`,
    service: `cronservices`
  },
  {
    provider: `airdropsdacs`,
    package: `standard`,
    service: `accountless1`
  },
  {
    provider: `airdropsdacs`,
    package: `standard`,
    service: `ipfsservice1`
  },
  {
    provider: `airdropsdacs`,
    package: `standard`,
    service: `oracleservic`
  },
  {
    provider: `airdropsdacs`,
    package: `standard`,
    service: `cronservices`
  }
];

// https://dsphq.io/accounts/phoenixfeeds/packages?network=kylin
async function action() {
  try {
    const filteredServices = services.filter(
      s =>
        s.provider == `airdropsdacs` &&
        (s.service === `oracleservic` || s.service === `cronservices`)
    );

    // await sendTransaction(
    //   filteredServices.map(service => ({
    //     account: `dappservices`,
    //     name: `preselectpkg`,
    //     authorization: [
    //       {
    //         actor: STAKE_ACC,
    //         permission: `active`
    //       }
    //     ],
    //     data: {
    //       owner: STAKE_ACC,
    //       ...service,
    //       depth: 5,
    //     }
    //   }))
    // );

    // await sendTransaction(
    //   filteredServices.map(service => ({
    //     account: `dappservices`,
    //     name: `selectpkg`,
    //     authorization: [
    //       {
    //         actor: STAKE_ACC,
    //         permission: `active`
    //       }
    //     ],
    //     data: {
    //       owner: STAKE_ACC,
    //       ...service
    //     }
    //   }))
    // );

    // await sendTransaction(
    //   filteredServices.map(service => ({
    //     account: `dappservices`,
    //     name: `unstaketo`,
    //     authorization: [
    //       {
    //         actor: ROOT,
    //         permission: `active`
    //       }
    //     ],
    //     data: {
    //       from: ROOT,
    //       to: STAKE_ACC,
    //       provider: service.provider,
    //       service: service.service,
    //       quantity: `10.0000 DAPP`,
    //     }
    //   }))
    // );

    // await sendTransaction(
    //   filteredServices.map(service => ({
    //     account: `dappservices`,
    //     name: `refundto`,
    //     authorization: [
    //       {
    //         actor: STAKE_ACC,
    //         permission: `active`
    //       }
    //     ],
    //     data: {
    //       from: ROOT,
    //       to: STAKE_ACC,
    //       provider: service.provider,
    //       service: service.service,
    //       symcode: `DAPP`,
    //     }
    //   }))
    // );
    await sendTransaction(
      filteredServices.map(service => ({
        account: `dappservices`,
        name: `closeprv`,
        authorization: [
          {
            actor: STAKE_ACC,
            permission: `active`
          }
        ],
        data: {
          ...service,
          owner: STAKE_ACC,
        }
      }))
    );

    process.exit(0);
  } catch (error) {
    // ignore
    process.exit(1);
  }
}

action();
