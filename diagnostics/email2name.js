const crypto = require("crypto");

const deriveNameFromEmail = async (email) => {
  const salt = `T4WlHfBK71K0kZGsvS4RRA`;
  // subset of eos name alphabet (without . and 1-5)
  const alphabet = `abcdefghijklmnopqrstuvwxyz`.split(``);
  const suffix = `.phnx`;

  const hash = crypto.createHash("sha256");
  hash.update(email.trim() + salt);

  const randomnessBuffer = hash.digest();
  const buffer = new Uint8Array(randomnessBuffer.slice(0, 12 - suffix.length));
  const name = Array.from(buffer)
    .map((byte) => alphabet[byte % alphabet.length])
    .join(``);
  const accountName = `${name}${suffix}`;

  // last 64 bits of sha256 hash
  const checksum = randomnessBuffer.readBigUInt64BE(8).toString();

  return {
    accountName,
    checksum,
  };
};

async function start() {
  const email = process.argv[process.argv.length - 1]
  const name = await deriveNameFromEmail(email);
  console.log(`${email} => ${name.accountName} (${name.checksum})`)
}

start().catch(console.error)
