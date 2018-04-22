import test from 'ava'
import * as fs from 'fs'
import * as path from 'path'
import { isMatch } from '../src/module'

test('simple buffers correctly calculate mismatch', t => {
    const firstImage = Buffer.from([69, 69, 69, 69])
    const secondImage = Buffer.from([42, 42, 42, 42])
    const result = isMatch(firstImage, secondImage, 0)
    t.is(result, false)
})

test('different images return mismatch', t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_b.jpg'))
    const result = isMatch(firstImage, secondImage, 0)
    t.is(result, false)
})

test('same images return match', t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))
    const result = isMatch(firstImage, secondImage, 0)
    t.is(result, true)
})

test('different images return match if under threshold', t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_c.png'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_d.png'))
    const result = isMatch(firstImage, secondImage, 10)
    t.is(result, true)
})

test('different images return mismatch if over threshold', t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_c.png'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_d.png'))
    const result = isMatch(firstImage, secondImage, 9)
    t.is(result, false)
})
