#pragma once

#include "graphics/common/SdCard.h"
#include <lvgl.h>

#if defined(HAS_SDCARD) && !defined(HAS_SD_MMC) && !defined(ARCH_PORTDUINO)

using ScreenshotFileSystem = SdFs;
using ScreenshotFile = FsFile;

#else

#include <FS.h>

using ScreenshotFileSystem = fs::FS;
using ScreenshotFile = File;

#endif

class TFTScreenshot
{
  public:
    static bool captureScreen(
        ScreenshotFileSystem &filesystem,
        const char *path
    );

    static bool captureObject(
        ScreenshotFileSystem &filesystem,
        const char *path,
        lv_obj_t *object
    );

  private:
    static bool writeBmp(
        ScreenshotFileSystem &filesystem,
        const char *path,
        const lv_draw_buf_t *snapshot
    );

    static bool writeUint16(
        ScreenshotFile &file,
        uint16_t value
    );

    static bool writeUint32(
        ScreenshotFile &file,
        uint32_t value
    );
};