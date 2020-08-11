# Phoenix 🐦🔥

## Development

Bootstrapped with [zeus-sdk](https://docs.liquidapps.io/en/stable/developers/zeus-getting-started.html).

### Setup

```
npm install -g @liquidapps/zeus-cmd
zeus install?????? # does not exist yet
```

Only way to do it consistently right now:

```bash
# clone this repo somewhere
git clone phoenix phoenix_

# create a new LiquidApps app
mkdir phoenix
cd phoenix
zeus unbox all-dapp-services --no-create-dir

# manually merge this repo to the liquidApps dapp
# Add phoenix project to contracts/eos/CMakeLists.txt

# Patch contracts/eosio.bios
# Patch extensions/commands/start-localenv 02 and 05, and 21

# Optionally copy the eosiac.yml
```

Deploy everything to an account, stake DAPP. (You can use the `actions/_stakedapp.js` script.)
**Make sure to also run the `xvinit` action that sets the chainId requires for vAccounts 2.0**.

### Local env tests

```
zeus compile phoenix --all=false
# zeus migrate # not needed
zeus test test/phoenix.spec.js --compile-all=0
```

### Deploying on Kylin
curl http://faucet.cryptokylin.io/create/phoenixashet
curl http://faucet.cryptokylin.io/get_token/phoenixashet

Stake DAPP services: `accountless1` and `ipfsservic`.

```bash
# phoenix init
node actions/_stakedapp.js # with changes to account name
node actions/_xvinit.js # with changes to account name

# phoenixtoken init
node actions/create-issue
node actions/init


# create some vaccounts
node actions/login.js
node actions/transfer.js # distributes some virtual phoenix
```

#### Notes

`zeus create contract <name>` is only available if one does `zeus unbox dapp --no-create-dir` (`templates-emptycontract-eos-cpp`, `templates-extensions`)
