const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `kylin`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const ROOT = accounts[0];

const STAKE_ACC = `phoenixv2t12` // accounts[accounts.length - 1]

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
    provider: `heliosselene`,
    package: `storagelarge`,
    service: `liquidstorag`
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

const getAmountOfDapp = service => {
  return `10.0000 DAPP`;
  // if (
  //   service.provider === `airdropsdacs` &&
  //   service.service === `ipfsservice1` &&
  //   service.package === `standard`
  // )
  //   return `1000.0000 DAPP`;
  // else return `10.0000 DAPP`;
};

const PROVIDERS = [`airdropsdacs`, `heliosselene`]
async function action() {
  try {
    const filteredServices = services.filter(
      s => s.provider === PROVIDERS[1]
    );

    if (filteredServices.length === 0) throw new Error(`No services`);

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
    //       depth: 1,
    //     }
    //   }))
    // );

    await sendTransaction(
      filteredServices.map(service => ({
        account: `dappservices`,
        name: `selectpkg`,
        authorization: [
          {
            actor: STAKE_ACC,
            permission: `active`
          }
        ],
        data: {
          owner: STAKE_ACC,
          ...service
        }
      }))
    );

    await sendTransaction(
      filteredServices.map(service => ({
        account: `dappservices`,
        name: `staketo`,
        authorization: [
          {
            actor: ROOT,
            permission: `active`
          }
        ],
        data: {
          from: ROOT,
          to: STAKE_ACC,
          provider: service.provider,
          service: service.service,
          quantity: getAmountOfDapp(service),
          transfer: false
        }
      }))
    );

    process.exit(0);
  } catch (error) {
    console.error(error.message);
    process.exit(1);
  }
}

action();
