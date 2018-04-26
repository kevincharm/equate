import test from 'ava'
import * as fs from 'fs'
import * as path from 'path'
import { isMatch } from '../src/module'

test('simple buffers correctly calculate mismatch', t => {
    const firstImage = Buffer.from([69, 69, 69, 69])
    const secondImage = Buffer.from([42, 42, 42, 42])
    isMatch(firstImage, secondImage, { tolerancePercent: 0 }, result => {
        t.is(result.didMatch, false)
    })
})

test('different images return mismatch', t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_b.jpg'))
    isMatch(firstImage, secondImage, { tolerancePercent: 0 }, result => {
        t.is(result.didMatch, false)
    })
})

test('same images return match', t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))
    isMatch(firstImage, secondImage, { tolerancePercent: 0 }, result => {
        t.is(result.didMatch, true)
    })
})

test('different images return match if under threshold', t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_c.png'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_d.png'))
    isMatch(firstImage, secondImage, { tolerancePercent: 10 }, result => {
        t.is(result.didMatch, true)
    })
})

test('different images return mismatch if over threshold', t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_c.png'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_d.png'))
    isMatch(firstImage, secondImage, { tolerancePercent: 9 }, result => {
        t.is(result.didMatch, false)
    })
})

/*
test('different images return buffer', t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_b.jpg'))
    isMatch(firstImage, secondImage, { tolerancePercent: 0, diffOutputFormat: 'jpeg' }, result => {
        const jpegBuffer = result.imageDiffData!
        t.is(true, true)
    })
})
*/
