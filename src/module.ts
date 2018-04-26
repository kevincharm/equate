const { _isMatch } = require('../build/Release/module.node') // tslint:disable-line

export interface MatchOptions {
    /**
     * Percentage of pixels allowed to be different for two images to be considered the same
     */
    tolerancePercent?: number
    /**
     * Format of the image buffer passed back as the image diff
     */
    diffOutputFormat?: 'png' | 'jpeg'
}

export interface MatchResult {
    /**
     * `true` if the two images are sufficiently similar
     */
    didMatch: boolean
    /**
     * Buffer containing image data highlighting the diff
     */
    imageDiffData?: Buffer
}

/**
 * Compares two images. Returns `true` if the two images are sufficiently similar.
 * @param imageA A node Buffer object containing raw data of the first image to compare
 * @param imageB A node Buffer object containing raw data of the second image to compare
 * @param options Specify matching options
 */
export function isMatch(imageA: Buffer, imageB: Buffer, options: MatchOptions): Promise<MatchResult> {
    return new Promise((resolve, reject) => {
        try {
            _isMatch(imageA, imageB, options, (result: MatchResult) => {
                resolve(result)
            })
        } catch (err) {
            reject(err)
        }
    })
}
