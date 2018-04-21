#include <node_api.h>
#include <stdint.h>
#include <stdio.h>
#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

static napi_value img_is_match(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value out;

    size_t argc = 2;
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

    napi_get_boolean(env, false, &out);

    int imga_width, imga_height, imga_nchannels, imgb_width, imgb_height, imgb_nchannels;
    uint16_t *imga_pixels = stbi_load_16_from_memory(imga, imga_len,
        &imga_width, &imga_height, &imga_nchannels, 4);
    uint16_t *imgb_pixels = stbi_load_16_from_memory(imgb, imgb_len,
        &imgb_width, &imgb_height, &imgb_nchannels, 4);

    DEBUG_INFO("imga { len: %d, w: %d, h: %d, n: %d }\n",
        (int)imga_len, imga_width, imga_height, imga_nchannels);
    DEBUG_INFO("imgb { len: %d, w: %d, h: %d, n: %d }\n",
        (int)imgb_len, imgb_width, imgb_height, imgb_nchannels);

    if (imga_width != imgb_width ||
        imga_height != imgb_height ||
        imga_nchannels != imgb_nchannels) {
        goto done;
    }

    size_t npixels = imga_width * imga_height;
    for (size_t i=0; i<npixels; i++) {
        if (imga_pixels[i] != imgb_pixels[i]) goto done;
    }

    napi_get_boolean(env, true, &out);

done:
    stbi_image_free(imga_pixels);
    stbi_image_free(imgb_pixels);

    return out;
}

napi_value init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value fn;

    status = napi_create_function(env, NULL, 0, img_is_match, NULL, &fn);
    OK_OR_THROW(status, "Unable to wrap native function")

    status = napi_set_named_property(env, exports, "isMatch", fn);
    OK_OR_THROW(status, "Unable to populate exports")

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
