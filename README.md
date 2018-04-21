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

    const firstImage = fs.readFileSync('foo.jpg')
    const secondImage = fs.readFileSync('foo.jpg')

    const result = isMatch(firstImage, secondImage)
    assert(result, true)
```

TypeScript (includes type definitions):
```ts
    import { isMatch } from 'equate'

    const firstImage = fs.readFileSync('foo.jpg')
    const secondImage = fs.readFileSync('bar.jpg')

    const result = isMatch(firstImage, secondImage)
    assert(result, false)
```
