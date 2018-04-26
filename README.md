![travis-status](https://travis-ci.org/kevincharm/equate.svg?branch=master)

# equate

Node native module for image diffing/comparison written in C. Requires `node@>=8.6.0` for N-API.

## Installation

Get it via npm:
```sh
npm install --save equate
```
or
```sh
yarn add equate
```

## Usage

Plain JavaScript:
```js
const { isMatch } = require('equate')

compare()

async function compare() {
    const firstImage = fs.readFileSync('foo.jpg')
    const secondImage = fs.readFileSync('foo.jpg')

    const result = await isMatch(firstImage, secondImage, {
        tolerancePercent: 0,
        diffOutputFormat: 'png'
    })

    assert(result.didMatch, true)
}
```

TypeScript (includes type definitions):
```ts
import { isMatch } from 'equate'

compare()

async function compare() {
    const firstImage = fs.readFileSync('foo.jpg')
    const secondImage = fs.readFileSync('bar.jpg')

    const result = await isMatch(firstImage, secondImage, {
        tolerancePercent: 0,
        diffOutputFormat: 'png'
    })

    const pngBuffer = result.imageDiffData
    assert(pngBuffer.readUInt8(0), 0x89)
    assert(result.didMatch, false)
}
```
