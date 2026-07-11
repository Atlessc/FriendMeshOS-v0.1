#include "graphics/view/TFT/TFTScreenshot.h"

#include <Arduino.h>
#include <fcntl.h>

#if LV_USE_SNAPSHOT
#include <src/others/snapshot/lv_snapshot.h>
#endif

namespace
{

constexpr uint32_t BMP_FILE_HEADER_SIZE = 14;
constexpr uint32_t BMP_INFO_HEADER_SIZE = 40;
constexpr uint32_t BMP_PIXEL_DATA_OFFSET =
    BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE;

uint32_t getBmpRowSize(uint32_t width)
{
    const uint32_t rawRowSize = width * 3U;

    // BMP rows must end on a 4-byte boundary.
    return (rawRowSize + 3U) & ~3U;
}

} // namespace

bool TFTScreenshot::captureScreen(ScreenshotFileSystem &filesystem, const char *path)
{
    lv_obj_t *screen = lv_screen_active();

    if (screen == nullptr) {
        Serial.println("[Screenshot] No active LVGL screen found");
        return false;
    }

    return captureObject(filesystem, path, screen);
}

bool TFTScreenshot::captureObject(
    ScreenshotFileSystem &filesystem,
    const char *path,
    lv_obj_t *object
)
{
#if !LV_USE_SNAPSHOT
    Serial.println("[Screenshot] LV_USE_SNAPSHOT is disabled");
    return false;
#else
    if (path == nullptr || path[0] == '\0') {
        Serial.println("[Screenshot] Invalid screenshot path");
        return false;
    }

    if (object == nullptr) {
        Serial.println("[Screenshot] Invalid LVGL object");
        return false;
    }

    /*
     * Make sure LVGL has finished calculating the current object layout
     * before rendering it into the snapshot buffer.
     */
    lv_obj_update_layout(object);

    /*
     * XRGB8888 gives us predictable 8-bit red, green and blue channels.
     *
     * A full 320x240 snapshot requires a little over 300 KB of temporary
     * memory, so the T-Deck's PSRAM is important here.
     */
    lv_draw_buf_t *snapshot =
        lv_snapshot_take(object, LV_COLOR_FORMAT_RGB565);

    if (snapshot == nullptr) {
        Serial.println(
            "[Screenshot] LVGL failed to create the snapshot buffer"
        );
        return false;
    }

    const bool success = writeBmp(filesystem, path, snapshot);

    lv_draw_buf_destroy(snapshot);

    if (success) {
        Serial.printf("[Screenshot] Saved: %s\n", path);
    } else {
        Serial.printf("[Screenshot] Failed to save: %s\n", path);
    }

    return success;
#endif
}

bool TFTScreenshot::writeBmp(
    ScreenshotFileSystem &filesystem,
    const char *path,
    const lv_draw_buf_t *snapshot
)
{
    if (snapshot == nullptr || snapshot->data == nullptr) {
        Serial.println("[Screenshot] Snapshot buffer is empty");
        return false;
    }

    if (snapshot->header.cf != LV_COLOR_FORMAT_RGB565) {
        Serial.println("[Screenshot] Unsupported snapshot color format");
        return false;
    }

    const uint32_t width = snapshot->header.w;
    const uint32_t height = snapshot->header.h;
    const uint32_t sourceStride = snapshot->header.stride;

    if (width == 0 || height == 0 || sourceStride == 0) {
        Serial.println("[Screenshot] Snapshot dimensions are invalid");
        return false;
    }

    const uint32_t bmpRowSize = getBmpRowSize(width);
    const uint32_t pixelDataSize = bmpRowSize * height;
    const uint32_t fileSize = BMP_PIXEL_DATA_OFFSET + pixelDataSize;

    #if defined(HAS_SDCARD) && !defined(HAS_SD_MMC) && !defined(ARCH_PORTDUINO)
    ScreenshotFile file;

    if (!file.open(path, O_WRONLY | O_CREAT | O_TRUNC)) {
        Serial.printf("[Screenshot] Could not open file: %s\n", path);
        return false;
    }
#else
    ScreenshotFile file = filesystem.open(path, FILE_WRITE);

    if (!file) {
        Serial.printf("[Screenshot] Could not open file: %s\n", path);
        return false;
    }
#endif

    /*
     * BITMAPFILEHEADER
     */
    file.write(static_cast<uint8_t>('B'));
    file.write(static_cast<uint8_t>('M'));

    if (!writeUint32(file, fileSize) ||
        !writeUint16(file, 0) ||
        !writeUint16(file, 0) ||
        !writeUint32(file, BMP_PIXEL_DATA_OFFSET)) {
        Serial.println("[Screenshot] Failed writing BMP file header");
        file.close();
        return false;
    }

    /*
     * BITMAPINFOHEADER
     */
    if (!writeUint32(file, BMP_INFO_HEADER_SIZE) ||
        !writeUint32(file, width) ||
        !writeUint32(file, height) ||
        !writeUint16(file, 1) ||
        !writeUint16(file, 24) ||
        !writeUint32(file, 0) ||
        !writeUint32(file, pixelDataSize) ||
        !writeUint32(file, 2835) ||
        !writeUint32(file, 2835) ||
        !writeUint32(file, 0) ||
        !writeUint32(file, 0)) {
        Serial.println("[Screenshot] Failed writing BMP info header");
        file.close();
        return false;
    }

    const uint32_t rawBmpRowSize = width * 3U;
    const uint32_t paddingSize = bmpRowSize - rawBmpRowSize;
    const uint8_t padding[3] = {0, 0, 0};

    /*
     * BMP files store image rows from bottom to top.
     */
    for (int32_t y = static_cast<int32_t>(height) - 1; y >= 0; --y) {
        const uint8_t *sourceRow =
            snapshot->data +
            (static_cast<uint32_t>(y) * sourceStride);

        for (uint32_t x = 0; x < width; ++x) {
            /*
             * LVGL XRGB8888 is stored in memory as:
             *
             * byte 0: blue
             * byte 1: green
             * byte 2: red
             * byte 3: unused/alpha
             *
             * A 24-bit BMP also expects blue, green, red byte ordering,
             * so we can write the first three bytes directly.
             */
            const uint16_t pixel565 =
              reinterpret_cast<const uint16_t *>(sourceRow)[x];

            const uint8_t red =
              static_cast<uint8_t>(((pixel565 >> 11U) & 0x1FU) * 255U / 31U);

            const uint8_t green =
              static_cast<uint8_t>(((pixel565 >> 5U) & 0x3FU) * 255U / 63U);

            const uint8_t blue =
              static_cast<uint8_t>((pixel565 & 0x1FU) * 255U / 31U);

            const uint8_t bmpPixel[3] = {
              blue,
              green,
              red
            };

            if (file.write(bmpPixel, sizeof(bmpPixel)) != sizeof(bmpPixel)) {
                Serial.println(
                    "[Screenshot] Failed writing BMP pixel data"
                );
                file.close();
                return false;
            }
        }

        if (paddingSize > 0 &&
            file.write(padding, paddingSize) != paddingSize) {
            Serial.println("[Screenshot] Failed writing BMP row padding");
            file.close();
            return false;
        }
    }

    file.flush();
    file.close();

    return true;
}

bool TFTScreenshot::writeUint16(ScreenshotFile &file, uint16_t value)
{
    const uint8_t bytes[2] = {
        static_cast<uint8_t>(value & 0xFFU),
        static_cast<uint8_t>((value >> 8U) & 0xFFU)
    };

    return file.write(bytes, sizeof(bytes)) == sizeof(bytes);
}

bool TFTScreenshot::writeUint32(ScreenshotFile &file, uint32_t value)
{
    const uint8_t bytes[4] = {
        static_cast<uint8_t>(value & 0xFFU),
        static_cast<uint8_t>((value >> 8U) & 0xFFU),
        static_cast<uint8_t>((value >> 16U) & 0xFFU),
        static_cast<uint8_t>((value >> 24U) & 0xFFU)
    };

    return file.write(bytes, sizeof(bytes)) == sizeof(bytes);
}