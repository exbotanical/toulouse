#!/usr/bin/env node

const fs = require('node:fs').promises

function die(msg, rc = 1) {
  console.error(msg)
  process.exit(1)
}

;(async () => {
  if (process.argv.length < 3) {
    die('Usage: node sign.js <filepath>')
  }

  const filepath = process.argv[2]
  try {
    let buf = fs.readFileSync(filepath)
    const len = buf.length
    if (len > 510) {
      die(`boot block too large (${len} bytes (max 510))`)
    }

    console.info(`boot block is ${len} bytes`)

    const padding = Buffer.alloc(510 - len, 0)
    buf = buf.concat([buf, padding])

    const signature = Buffer.from([0x55, 0xaa])
    buf = buf.concat([buf, signature])
    fs.writeFileSync(filepath, buf)
  } catch (e) {
    die(`an exception occurred: ${e}`)
  }
})()
