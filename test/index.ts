import test from 'ava'
import * as fs from 'fs'
import * as path from 'path'
import { isMatch } from '../src/module'

test('simple buffers correctly calculate mismatch', async t => {
    const firstImage = Buffer.from([69, 69, 69, 69])
    const secondImage = Buffer.from([42, 42, 42, 42])

    const result = await isMatch(firstImage, secondImage, { tolerancePercent: 0 })
    t.is(result.didMatch, false)
})

test('different images return mismatch', async t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_b.jpg'))

    const result = await isMatch(firstImage, secondImage, { tolerancePercent: 0 })
    t.is(result.didMatch, false)
})

test('same images return match', async t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))

    const result = await isMatch(firstImage, secondImage, { tolerancePercent: 0 })
    t.is(result.didMatch, true)
})

test('different images return match if under threshold', async t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_c.png'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_d.png'))

    const result = await isMatch(firstImage, secondImage, { tolerancePercent: 10 })
    t.is(result.didMatch, true)
})

test('different images return mismatch if over threshold', async t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_c.png'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_d.png'))

    const result = await isMatch(firstImage, secondImage, { tolerancePercent: 9 })
    t.is(result.didMatch, false)
})

test('different images return buffer', async t => {
    const firstImage = fs.readFileSync(path.join(__dirname, 'image_a.jpg'))
    const secondImage = fs.readFileSync(path.join(__dirname, 'image_b.jpg'))
    const diffImage = fs.readFileSync(path.join(__dirname, 'image_diff.png'))

    const result = await isMatch(firstImage, secondImage, { tolerancePercent: 0, diffOutputFormat: 'png' })
    const pngBuffer = result.imageDiffData
    t.deepEqual(pngBuffer, diffImage)
})
