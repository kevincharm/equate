import test from 'ava'
import * as fs from 'fs'
import * as path from 'path'

const { isMatch } = require('../build/Release/module') // tslint:disable-line

test('simple buffers correctly calculate mismatch', t => {
    const firstImage = Buffer.from([69, 69, 69, 69])
    const secondImage = Buffer.from([42, 42, 42, 42])
    const result = isMatch(firstImage, secondImage)
    t.is(result, false)
})

test('different jpegs correctly calculate mismatch', t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_b.jpg'))
    const result = isMatch(firstImage, secondImage)
    t.is(result, false)
})

test('same jpegs correctly calculate match', t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))
    const result = isMatch(firstImage, secondImage)
    t.is(result, true)
})
