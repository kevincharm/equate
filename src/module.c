#include <node_api.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <math.h>
#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "vendor/stb_image_write.h"

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

static napi_value is_match(napi_env env, napi_callback_info info)
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

    // Get image buffer (1st & 2nd arguments)
    size_t imga_len = 0;
    uint8_t *imga;
    status = napi_get_buffer_info(env, argv[0], (void **)&imga, &imga_len);
    OK_OR_THROW(status, "First buffer supplied is invalid!")
    size_t imgb_len = 0;
    uint8_t *imgb;
    status = napi_get_buffer_info(env, argv[1], (void **)&imgb, &imgb_len);
    OK_OR_THROW(status, "Second buffer supplied is invalid!")

    // Parse options (3rd argument)
    napi_value options = argv[2];
    double tolerance_pct = get_tolerance_pct(env, options);

    enum diff_out_fmt output_fmt = get_output_fmt(env, options);
    (void)output_fmt; // TODO: unused

    // Assert that callback is a function (4th argument)
    napi_value arg_cb = argv[3];
    assert_is_function(env, arg_cb);

    // Parse image buffers into iterable channels per pixel
    int imga_width = 0;
    int imga_height = 0;
    int imga_nchannels = 0;
    int imgb_width = 0;
    int imgb_height = 0;
    int imgb_nchannels = 0;
    const int nchannels = 4;
    uint8_t *imgdiff_png_buffer = NULL;
    uint8_t *imgdiff_pixels = NULL;
    // TODO: check for NULL allocation and throw
    uint8_t *imga_pixels = stbi_load_from_memory(imga, imga_len,
        &imga_width, &imga_height, &imga_nchannels, nchannels);
    uint8_t *imgb_pixels = stbi_load_from_memory(imgb, imgb_len,
        &imgb_width, &imgb_height, &imgb_nchannels, nchannels);
    if (imga_width == 0 || imga_height == 0 ||
        imgb_width == 0 || imgb_height == 0) {
        // TODO: Handle files with different # of channels
        goto done;
    }

    int max_width = imga_width > imgb_width ? imga_width : imgb_width;
    int max_height = imga_height > imgb_height ? imga_height : imgb_height;
    int u8_img_len = max_width * max_height * nchannels;
    imgdiff_pixels = malloc(u8_img_len);
    int pixel_diff_count = 0;
    for (int i = 0; i < max_width * nchannels; i++) {
        bool widthMismatch = false;
        if (i > (imga_width * nchannels) ||
            i > (imgb_width * nchannels)) {
            widthMismatch = true;
        }

        for (int j = 0; j < max_height; j++) {
            bool heightMismatch = false;
            if (!widthMismatch &&
                (j > imga_height ||
                j > imgb_height)) {
                heightMismatch = true;
            }

            bool pixelMismatch = false;
            int r = i + (j * max_width * nchannels);
            if (!widthMismatch &&
                !heightMismatch &&
                (imga_pixels[r] != imgb_pixels[r])) {
                pixelMismatch = true;
            }

            if (!widthMismatch && !heightMismatch && !pixelMismatch) {
                imgdiff_pixels[r] = imga_pixels[r];
                continue;
            }

            // If a mismatch, colour the diff pixel with magenta
            pixel_diff_count++;
            int rgba_n = r % nchannels;
            if (rgba_n == 0) {
                imgdiff_pixels[r] = 0xff; // red
            } else if (rgba_n == 1) {
                imgdiff_pixels[r] = 0; // green
            } else if (rgba_n == 2) {
                imgdiff_pixels[r] = 0xff; // blue
            }
        }
    }

    double tolerance_thresh = (max_width * max_height * tolerance_pct) / 100.;
    if (pixel_diff_count <= tolerance_thresh) {
        napi_get_boolean(env, true, &did_match);
    }

    if (output_fmt != DIFF_OUTPUT_PNG) {
        THROW("Only PNG diff output is currently supported!")
    }
    int stride_bytes = nchannels * max_width;
    int imgdiff_png_buffer_len = 0;
    imgdiff_png_buffer = stbi_write_png_to_mem(imgdiff_pixels, stride_bytes,
        max_width, max_height, nchannels, &imgdiff_png_buffer_len);

    napi_value image_diff_data;
    status = napi_create_buffer_copy(env, imgdiff_png_buffer_len,
        (const void *)imgdiff_png_buffer, NULL, &image_diff_data);
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
    STBIW_FREE(imgdiff_png_buffer);
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

    status = napi_create_function(env, NULL, 0, is_match, NULL, &fn);
    OK_OR_THROW(status, "Unable to wrap native function")

    status = napi_set_named_property(env, exports, "_isMatch", fn);
    OK_OR_THROW(status, "Unable to populate exports")

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
