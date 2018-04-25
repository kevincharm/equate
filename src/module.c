#include <node_api.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <math.h>
#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

enum diff_out_fmt {
    DIFF_OUTPUT_JPEG,
    DIFF_OUTPUT_PNG
};

static inline enum diff_out_fmt get_output_fmt(napi_env env, napi_value options)
{
    napi_status status;
    enum diff_out_fmt output_fmt = DIFF_OUTPUT_PNG;

    napi_value nv_diff_out_fmt;
    status = napi_get_named_property(env, options, "diffOutputFormat", &nv_diff_out_fmt);
    if (status != napi_ok) {
        goto done;
    }

    bool is_jpeg = false;
    napi_value nv_str_jpeg;
    status = napi_create_string_utf8(env, "jpeg", 4, &nv_str_jpeg);
    OK_OR_THROW(status, "Unable to create string 'jpg'!")
    status = napi_strict_equals(env, nv_diff_out_fmt, nv_str_jpeg, &is_jpeg);
    OK_OR_THROW(status, "Unable to compare diffOutputFormat!")
    if (is_jpeg) {
        output_fmt = DIFF_OUTPUT_JPEG;
    }

done:
    return output_fmt;
}

static inline double get_tolerance_pct(napi_env env, napi_value options)
{
    napi_status status;
    double tolerance_pct = 0;

    napi_value nv_tolerance_pct;
    status = napi_get_named_property(env, options, "tolerancePercent", &nv_tolerance_pct);
    if (status != napi_ok) {
        goto done;
    }

    status = napi_get_value_double(env, nv_tolerance_pct, &tolerance_pct);
    OK_OR_THROW(status, "Tolerance percentage supplied is invalid!")

done:
    return tolerance_pct;
}

static inline void assert_is_function(napi_env env, napi_value fn)
{
    napi_status status;
    napi_valuetype fn_type;
    status = napi_typeof(env, fn, &fn_type);
    OK_OR_THROW(status, "Unable to get typeof callback!")
    if (fn_type != napi_function) {
        THROW("Callback (4th argument) must be a function!")
    }
}

static napi_value img_is_match(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value result;
    status = napi_create_object(env, &result);
    OK_OR_THROW(status, "Failed to create result object!")

    napi_value global;
    status = napi_get_global(env, &global);
    OK_OR_THROW(status, "Failed to get global object!")

    napi_value did_match;
    napi_get_boolean(env, false, &did_match);

    // Get arguments
    size_t argc = 4;
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

    // Options (3rd argument)
    napi_value options = argv[2];
    double tolerance_pct = get_tolerance_pct(env, options);

    enum diff_out_fmt output_fmt = get_output_fmt(env, options);
    (void)output_fmt; // TODO: unused

    // Callback (4th argument)
    napi_value arg_cb = argv[3];
    assert_is_function(env, arg_cb);

    // Parse image buffers into iterable channels per pixel
    int imga_width, imga_height, imga_nchannels, imgb_width, imgb_height, imgb_nchannels;
    uint16_t *imgdiff_pixels = NULL;
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

    size_t u16_px_len = imga_width * imga_height * imga_nchannels;
    imgdiff_pixels = malloc(u16_px_len * sizeof(uint16_t));
    double tolerance_thresh = (imga_width * imga_height * tolerance_pct) / 100.;
    int pixel_diff_count = 0;
    for (size_t i=0; i<u16_px_len; i++) {
        if (imga_pixels[i] != imgb_pixels[i]) {
            pixel_diff_count++;
            size_t rgba_n = i % imga_nchannels;
            if (rgba_n == 0) {
                // red
                imgdiff_pixels[i] = 0xffff;
            } else if (rgba_n == 2) {
                // blue
                imgdiff_pixels[i] = 0xffff;
            } else {
                imgdiff_pixels[i] = imga_pixels[i];
            }
        }
    }

    DEBUG_INFO("pixel_diff_count: %d, tolerance_thresh: %.2f\n",
        (int)pixel_diff_count, tolerance_thresh);
    if (pixel_diff_count > tolerance_thresh == false) {
        napi_get_boolean(env, true, &did_match);
    }

    // TODO(ktjiam): Encode the damn buffer before you return it you doofus.
    napi_value image_diff_data;
    status = napi_create_buffer(env, u16_px_len * sizeof(uint16_t), (void **)imgdiff_pixels, &image_diff_data);
    OK_OR_THROW(status, "Failed to create Buffer for highlighted image diff data!")
    status = napi_set_named_property(env, result, "imageDiffData", image_diff_data);
    OK_OR_THROW(status, "Failed to set property imageDiffData in result object!")

done:
    // Populate the results object
    status = napi_set_named_property(env, result, "didMatch", did_match);
    OK_OR_THROW(status, "Failed to set property didMatch in result object!")

    // Invoke user-provided callback with result object
    napi_value unused_cb_ret;
    status = napi_call_function(env, global, arg_cb, 1, &result, &unused_cb_ret);
    OK_OR_THROW(status, "Failed to invoke callback!")

    // Cleanup
    stbi_image_free(imga_pixels);
    stbi_image_free(imgb_pixels);
    free(imgdiff_pixels);

    napi_value undefined;
    status = napi_get_undefined(env, &undefined);
    OK_OR_THROW(status, "Failed to create undefined!")
    return undefined;
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
