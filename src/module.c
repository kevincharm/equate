#include <node_api.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <math.h>
#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

static napi_value img_is_match(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result;

    status = napi_create_object(env, &result);
    OK_OR_THROW(status, "Failed to create result object!")

    napi_value did_match;
    napi_get_boolean(env, false, &did_match);

    // Get arguments
    size_t argc = 3;
    napi_value argv[argc];
    status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    OK_OR_THROW(status, "Unable to parse arguments!")

    size_t imga_len = 0;
    uint8_t *imga;
    status = napi_get_buffer_info(env, argv[0], (void **)&imga, &imga_len);
    OK_OR_THROW(status, "First buffer supplied is invalid!")

    size_t imgb_len = 0;
    uint8_t *imgb;
    status = napi_get_buffer_info(env, argv[1], (void **)&imgb, &imgb_len);
    OK_OR_THROW(status, "Second buffer supplied is invalid!")

    double tolerance_pct = 0;
    status = napi_get_value_double(env, argv[2], &tolerance_pct);
    OK_OR_THROW(status, "Tolerance percentage supplied is invalid!")

    // Parse image buffers into iterable channels per pixel
    int imga_width, imga_height, imga_nchannels, imgb_width, imgb_height, imgb_nchannels;
    uint16_t *imga_pixels = stbi_load_16_from_memory(imga, imga_len,
        &imga_width, &imga_height, &imga_nchannels, 4);
    uint16_t *imgb_pixels = stbi_load_16_from_memory(imgb, imgb_len,
        &imgb_width, &imgb_height, &imgb_nchannels, 4);

    DEBUG_INFO("imga { len: %d, w: %d, h: %d, n: %d }\n",
        (int)imga_len, imga_width, imga_height, imga_nchannels);
    DEBUG_INFO("imgb { len: %d, w: %d, h: %d, n: %d }\n",
        (int)imgb_len, imgb_width, imgb_height, imgb_nchannels);

    // Exit early if dimensions are different
    if (imga_width != imgb_width ||
        imga_height != imgb_height ||
        imga_nchannels != imgb_nchannels) {
        goto done;
    }

    size_t npixels = imga_width * imga_height;
    double tolerance_thresh = (npixels * tolerance_pct) / 100.;
    int pixel_diff_count = 0;
    for (size_t i=0; i<npixels; i++) {
        if (imga_pixels[i] != imgb_pixels[i]) {
            pixel_diff_count++;
        }

        if (pixel_diff_count > tolerance_thresh) {
            goto done;
        }
    }

    // At this point, the two images are considered equivalent
    napi_get_boolean(env, true, &did_match);

done:
    stbi_image_free(imga_pixels);
    stbi_image_free(imgb_pixels);

    // Populate the results object
    status = napi_set_named_property(env, result, "didMatch", did_match);
    OK_OR_THROW(status, "Failed to set property didMatch in result object!")

    return result;
}

napi_value init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value fn;

    status = napi_create_function(env, NULL, 0, img_is_match, NULL, &fn);
    OK_OR_THROW(status, "Unable to wrap native function")

    status = napi_set_named_property(env, exports, "_isMatch", fn);
    OK_OR_THROW(status, "Unable to populate exports")

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
