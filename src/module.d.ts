/// <reference types="node" />
/**
 * Compares two images. Returns `true` if the two images are sufficiently similar.
 * @param imageA A node Buffer object containing raw data of the first image to compare
 * @param imageB A node Buffer object containing raw data of the second image to compare
 * @param tolerancePercentage Percentage of pixels allowed to be different for two images to be considered the same
 */
export declare const isMatch: (imageA: Buffer, imageB: Buffer, tolerancePercentage: number) => boolean;
