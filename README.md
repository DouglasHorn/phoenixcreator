# PhoenixCreator

Phoenix is a membership platform that **connects content creators with their fans**. Phoenix enables creators to develop a membership-based business.
With Phoenix, fans support their preferred creators via recurring payments of a stable coin - _WEOSDT_. 
In return, content creators publish exclusive content available only to their fans. Creators can also give special rewards such as rare NFTs or other exclusive rewards to their subscribers. 

### Pricing:
Fans are charged at the time of their initial pledge and on the anniversary of their pledge each month thereafter. 
Phoenix charges a **2% fee on the monthly income** earned by creators on the platform. 
**There are no other fees.**
Fans make recurring payments in WEOSDT and creators receive their income in WEOSDT.
Phoenix provides a hosted creator page and dedicated coaching and support for creators. We will help you to build a successful and thriving membership business. 


### Community Guidelines

Phoenix is a platform for content creators to share their creations exclusively with their fans.
Please don’t post anything that is obviously illegal.
A good rule of thumb is, if you are uncomfortable sharing your content publicly, then you should probably not post it on Phoenix.
As a creator, you may not post creations that infringe on another's intellectual property rights.
Phoenix is a community that is inclusive and diverse. But we do ask that you respect the rule of law. Please flag your creations as 18+ if they may be considered inappropriate for people under 18 years of age. 
We will not allow funds to be collected for anything that facilitates harmful or illegal activities.
Please use your common sense. If you feel that something may be illegal, please don’t post such content on Phoenix.


## Features

- **Accountless signup & login using only email**: No pre-existing blockchain account or wallet is required to sign up as a user and use the platform to discover content. This is made possible by using the email to derive a LiquidAccount (called Phoenix account) that is managed through 2FA enabled by the email address. This allows for a **smooth user onboarding without any prior blockchain experience** and going through the hassle of getting a WAX account, setting up a wallet and managing CPU/NET/RAM account resources, while still giving full custody to the user. The contract uses LiquidAccounts to store the user data in vRAM (IPFS) to enable scaling beyond the chain's RAM limitations.
- **Optional Free WAX account creation**: We offer a free [`.phoenix`](https://wax.bloks.io/account/phoenix) WAX account to all users to interact with third-party services that are not yet compatible with LiquidAccounts like decentralized exchanges.
- **Creating content**: Users create and edit posts using a rich-text editor and store the post contents on-chain so they can never be censored. vRAM is used for storing the content, such that each post it leaves a trail on the blockchain and other DSPs can easily reconstruct the posts from chain information.
- **Following/Subscribing** to users: Users can follow other users and are presented with a customized feed of posts. A user owning stablecoin on a blockchain account can deposit it to the contract and their Phoenix account is credited with the funds. 
  They can then subscribe to other uses by pledging a recurring amount each month denominated in USD. The recurring payments are implemented using LiquidScheduler in case the payments are set to auto-renew.
- **Subscriber-only content**: Phoenix allows content creators to create posts only for subscribers who pledged a certain amount a month. Achieving privacy on a public blockchain requires some sophisticated mechanisms and we use the DSPs as trusted decryption providers.
- **Profile info upload**: Users can edit their profile and upload avatars. The images are stored on IPFS (not in vRAM) using LiquidStorage and only an IPFS hash is stored on-chain instead. This allows fetching the user avatars from a DSP's IPFS cluster.
- **Deposit / Transfer / Withdraw funds**: Users can transfer deposited funds to other Phoenix users or withdraw their funds to a blockchain account they control, taking them out of the Phoenix ecosystem.


# Smart contracts

> The smart contracts are open-source and can be found [here](./zeus_boxes/contracts/eos)

The smart contracts for Phoenix are live on the [WAX blockchain](https://github.com/worldwide-asset-exchange/wax-blockchain) and are split into:

1. [`phoenixgroup`](https://wax.bloks.io/account/phoenixgroup): The core contract handling signups, logins, post creation, etc.
2. [`phoenixcreat`](https://wax.bloks.io/account/phoenixcreat): The token contract handling account balances and subscription payments

Phoenix makes use of various [**LiquidApps services**](https://liquidapps.io/) to scale the platform and onboard new users as easy as possible.

## Logic contract

Due to using LiquidAccounts and vRAM all actions initiated by Phoenix users are LiquidAccount actions (except when depositing funds), and most tables are vRAM tables. Therefore, interaction with the contract requires using the Phoenix frontend, or a custom block explorer that can encode/decode LiquidAccount and vRAM data.

<details>
<summary>Phoenixgroup Table documentation</summary>
  
#### User Info

vRAM Table: `users`

```cpp
struct [[eosio::table]] user_info {
  name username;
  name linked_name = ""_n; // account of EOS name
  user_profile_info profile_info;
  std::vector<pledge_tier> tiers;
  // as there's no support for secondary indexes on vRAM, we need to keep the
  // "foreign key" relationships in the user
  std::vector<uint64_t> post_indexes = std::vector<uint64_t>{};

  auto primary_key() const { return username.value; }
};
```


#### Posts

vRAM Table: `posts`

```cpp
struct [[eosio::table]] post_info {
  uint64_t id;
  name author;
  std::vector<uint8_t> title;
  std::vector<uint8_t> content;
  eosio::time_point_sec created;
  eosio::time_point_sec updated;
  std::vector<uint8_t> featured_image_url;
  std::string meta;
  bool encrypted = false;
  float decrypt_for_usd = 0;

  auto primary_key() const { return id; }
};
```

#### Follows

Followers are split into two tables to easily query followers of an account, but also accounts a user follows.
vRAM Table: `follows` - Scopes: `from` / `to`

```cpp
/**
 * Follows
 * would just be a from,to table with index on both
 * but DAPP network does not support secondary indexes
 * so split it into two tables and duplicate data
 * scope="from", scope="to"
 */
struct [[eosio::table]] follows_info {
  name user;
  std::vector<name> users;

  auto primary_key() const { return user.value; }
};
```

#### Subscriptions

The same split into two tables is done for subscribers and subscribees with a reference to the subscription data.
vRAM Table: `pledgesrel` - Scopes: `from` / `to`

```cpp
struct name_pledge_pair {
  name name;
  uint64_t pledge_id;
};
struct [[eosio::table]] pledges_rel_info {
  name user;
  std::vector<name_pledge_pair> users;

  auto primary_key() const { return user.value; }
};
```

vRAM Table: `pledges`

```cpp
struct [[eosio::table]] pledge_info {
  uint64_t id;
  name from;
  name to;
  eosio::microseconds cycle_us;
  // taken from subscription tier
  float usd_value;
  // value of (eos_quantity + phoenix_quantity) at time of pledge must be >=
  // usd_value
  eosio::asset weosdt_quantity = asset(0, WEOSDT_EXT_SYMBOL.get_symbol());
  eosio::time_point cycle_start;
  /* if pledge should auto renew next cycle */
  bool autorenew = false;
  /* arguments to update the pledge to in the next cycle */
  eosio::microseconds next_cycle_us = microseconds(0);
  // taken from subscription tier
  float next_usd_value;
  eosio::asset next_weosdt_quantity = asset(0, WEOSDT_EXT_SYMBOL.get_symbol());
  /* remove the pledge after next cycle */
  bool next_delete = false;
  bool paid = false;

  auto primary_key() const { return id; }
};
```

#### Encryptions

Phoenix allows content creators to create posts only for subscribers who pledged a certain amount a month. Achieving privacy on a public blockchain requires some sophisticated mechanisms and we use the DSPs as trusted decryption providers.
- The creator decides to create a subscriber-only post and derives a private symmetric post encryption key.
- They encrypt the post using the post encryption key and encrypt the encryption key itself using available DPS's public keys. The encrypted post and the DSP-encrypted post encryption key is sent to the blockchain and stored in vRAM. So far no user can decrypt the post.
- A user wants to view the subscriber-only post and asks a DSP to decrypt it for them. They send an authenticated message from their vaccount to the DSP.
- The DSP checks the message's authenticity, and checks if the user subscribed to the post creator. If the user is eligible to view the post, the DSP retrieves the encrypted post encryption key from the blockchain and can decrypt it because it was encrypted under their public key. The decrypted post encryption key is sent as a reponse to the user.
- The user can now fetch the encrypted post from vRAM and decrypt it using the post encryption key.

This description service could easily be generalized and become a general DSP decryption service.

vRAM Table: `postkeyenc` - Scopes: DSP account names that provide decryption services

```cpp
// Scope: DSP account
struct [[eosio::table("postkeyenc")]] post_key_encryption {
  uint64_t post_id;
  std::vector<uint8_t> post_key;
  auto primary_key() const { return post_id; }
};
```


### Config Tables

The config tables are the only tables stored in RAM instead of vRAM as they are singletons and of a fixed size, therefore not limiting scale.

#### Globals

This configuration singleton manages global data like the next post id, if the contract is paused, or the latest posts to display.

Table Name: `globals`
```cpp
TABLE globals {
  std::vector<uint64_t> latest_post_indexes = std::vector<uint64_t>{};
  uint64_t next_post_id = 0;
  bool paused = false;
};
```

#### Limits

The limits table tracks daily account limits to prevent spam.

Table Name: `limits`

```cpp
TABLE limits {
  uint32_t day_identifier = 0;
  uint32_t vaccounts_created_today = 0;
  uint32_t max_vaccount_creations_per_day = 50;
};
```
</details>

## Coldtoken contract

Phoenix also contains a [**coldtoken contract**](https://docs.liquidapps.io/en/v2.0/developers/boxes/coldtoken.html) managing the vaccounts' token balances in vRAM tables and using vaccount actions to transfer funds.

<details>
<summary>Phoenixcreat documentation</summary>

The coldtoken contract is an extension of the [LiquidApps' coldtoken contract](https://github.com/liquidapps-io/zeus-sdk/tree/e6122731a1771d6d19342657028ba9307bd9d1b7/boxes/groups/sample/coldtoken/contracts/eos/coldtoken):

- It has been extended to manage subscription fees paid out with a `payoutfees` action.
- It also contains a `createacc` action to create a free [`.phoenix` WAX account](https://wax.bloks.io/account/phoenix).

</details>

## Development Instructions

Bootstrapped with [zeus-sdk](https://docs.liquidapps.io/en/stable/developers/zeus-getting-started.html).

<details>
<summary><b>Setup</b></summary>

```bash
npm install -g @liquidapps/zeus-cmd
# zeus install # does not exist yet
```

Only way to do it consistently right now:

```bash
# clone this repo somewhere
git clone phoenixcreator phoenixcreator_
```

Create a new [LiquidApps contract](https://docs.liquidapps.io/en/v2.0/developers/zeus-getting-started.html#create-your-own-contract)

```bash
mkdir phoenixcreator; cd phoenixcreator
zeus box create
zeus unbox all-dapp-services
zeus unbox liquidx
#zeus create contract phoenix

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

```bash
curl http://faucet.cryptokylin.io/create/phoenixashet
curl http://faucet.cryptokylin.io/get_token/phoenixashet
```

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
</details>

# Resources

- [phoenixcreator.com](https://phoenixcreator.com)
- [Telegram](https://t.me/phoenixcreator)
- [Twitter](https://twitter.com/phoenixcreator1)

# License

[MIT licensed](./LICENSE).
