#!/usr/bin/env python3
"""Generate the T-Deck FriendMeshOS splash and theme-recolorable mark."""

from pathlib import Path
import textwrap

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[1]
BRANDING = ROOT / "branding"
MARK_PNG = BRANDING / "friendmeshos-mark-30x17.png"
SPLASH_PNG = BRANDING / "logo_320x240.png"
MARK_CPP = (
    ROOT
    / "lib/meshtastic-device-ui/source/graphics/TFT/FriendMeshBranding.cpp"
)

NAVY = "#07111F"
SURFACE = "#0F172A"
SLATE = "#334155"
CYAN = "#22D3EE"
WHITE = "#F8FAFC"
MUTED = "#94A3B8"


def font(size: int, bold: bool = False) -> ImageFont.FreeTypeFont:
    candidates = [
        Path("/System/Library/Fonts/Supplemental/Arial Bold.ttf" if bold else "/System/Library/Fonts/Supplemental/Arial.ttf"),
        Path("/System/Library/Fonts/Helvetica.ttc"),
        Path("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf" if bold else "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"),
    ]
    for candidate in candidates:
        if candidate.exists():
            return ImageFont.truetype(str(candidate), size=size)
    return ImageFont.load_default(size=size)


def draw_mark(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], color: str) -> None:
    left, top, right, bottom = box
    width = right - left
    height = bottom - top
    stroke = max(2, round(min(width, height) * 0.07))
    radius = max(2, round(min(width, height) * 0.10))
    top_node = (left + width // 2, top + radius)
    left_node = (left + radius, bottom - radius)
    right_node = (right - radius, bottom - radius)

    for start, end in ((top_node, left_node), (left_node, right_node), (right_node, top_node)):
        draw.line((start, end), fill=color, width=stroke)
    for x, y in (top_node, left_node, right_node):
        draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=color)

    arrow_top = top + round(height * 0.36)
    arrow_bottom = bottom - round(height * 0.18)
    center = left + width // 2
    half = max(2, round(width * 0.13))
    notch = max(1, round(width * 0.045))
    draw.polygon(
        (
            (center, arrow_top),
            (center - half, arrow_bottom),
            (center, arrow_bottom - notch * 2),
            (center + half, arrow_bottom),
        ),
        fill=color,
    )


def render_mark() -> Image.Image:
    scale = 8
    image = Image.new("RGBA", (30 * scale, 17 * scale), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw_mark(draw, (3 * scale, 1 * scale, 27 * scale, 16 * scale), WHITE)
    image = image.resize((30, 17), Image.Resampling.LANCZOS)
    image.save(MARK_PNG, optimize=True)
    return image


def centered_text(
    draw: ImageDraw.ImageDraw,
    canvas_width: int,
    text: str,
    y: int,
    text_font: ImageFont.ImageFont,
    fill: str,
) -> None:
    bounds = draw.textbbox((0, 0), text, font=text_font)
    width = bounds[2] - bounds[0]
    draw.text(((canvas_width - width) // 2, y), text, font=text_font, fill=fill)


def render_splash() -> None:
    scale = 4
    image = Image.new("RGB", (320 * scale, 240 * scale), NAVY)
    draw = ImageDraw.Draw(image)

    draw.rounded_rectangle(
        (12 * scale, 12 * scale, 308 * scale, 228 * scale),
        radius=12 * scale,
        fill=SURFACE,
        outline=SLATE,
        width=2 * scale,
    )
    for x, y in ((28, 34), (292, 34), (28, 172), (292, 172)):
        draw.ellipse(
            ((x - 2) * scale, (y - 2) * scale, (x + 2) * scale, (y + 2) * scale),
            fill=CYAN,
        )

    draw_mark(draw, (125 * scale, 32 * scale, 195 * scale, 92 * scale), CYAN)
    centered_text(draw, 320 * scale, "FriendMeshOS", 108 * scale, font(34 * scale, bold=True), WHITE)
    centered_text(
        draw,
        320 * scale,
        "OFF-GRID  |  FRIENDS  |  FIELD",
        154 * scale,
        font(10 * scale, bold=True),
        MUTED,
    )
    image.resize((320, 240), Image.Resampling.LANCZOS).save(SPLASH_PNG, optimize=True)


def rgb565a8_bytes(image: Image.Image) -> list[int]:
    rgba = image.convert("RGBA")
    rgb565: list[int] = []
    alpha: list[int] = []
    pixels = rgba.get_flattened_data() if hasattr(rgba, "get_flattened_data") else rgba.getdata()
    for red, green, blue, opacity in pixels:
        value = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3)
        rgb565.extend((value & 0xFF, value >> 8))
        alpha.append(opacity)
    return rgb565 + alpha


def c_array(values: bytes | list[int]) -> str:
    rows = []
    for index in range(0, len(values), 24):
        rows.append("    " + ", ".join(f"0x{value:02x}" for value in values[index : index + 24]) + ",")
    return "\n".join(rows)


def render_cpp(image: Image.Image) -> None:
    mark_data = c_array(rgb565a8_bytes(image))
    splash_data = c_array(SPLASH_PNG.read_bytes())
    MARK_CPP.write_text(
        textwrap.dedent(
            f"""\
            #if defined(FRIENDMESHOS_TDECK) && defined(VIEW_320x240)

            #include "graphics/view/TFT/FriendMeshBranding.h"

            #ifndef LV_ATTRIBUTE_MEM_ALIGN
            #define LV_ATTRIBUTE_MEM_ALIGN
            #endif

            static const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST uint8_t friendmeshos_mark_map[] = {{
            {mark_data}
            }};

            static const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST uint8_t friendmeshos_splash_map[] = {{
            {splash_data}
            }};

            const lv_image_dsc_t friendmeshos_mark_image = [] {{
                lv_image_dsc_t descriptor{{}};
                descriptor.header.magic = LV_IMAGE_HEADER_MAGIC;
                descriptor.header.cf = LV_COLOR_FORMAT_RGB565A8;
                descriptor.header.flags = 0;
                descriptor.header.w = 30;
                descriptor.header.h = 17;
                descriptor.header.stride = 60;
                descriptor.data_size = sizeof(friendmeshos_mark_map);
                descriptor.data = friendmeshos_mark_map;
                return descriptor;
            }}();

            const lv_image_dsc_t friendmeshos_splash_image = [] {{
                lv_image_dsc_t descriptor{{}};
                descriptor.header.magic = LV_IMAGE_HEADER_MAGIC;
                descriptor.header.cf = LV_COLOR_FORMAT_RAW_ALPHA;
                descriptor.header.flags = 0;
                descriptor.header.w = 320;
                descriptor.header.h = 240;
                descriptor.header.stride = 0;
                descriptor.data_size = sizeof(friendmeshos_splash_map);
                descriptor.data = friendmeshos_splash_map;
                return descriptor;
            }}();

            #endif
            """
        )
    )


def main() -> None:
    mark = render_mark()
    render_splash()
    render_cpp(mark)
    print(MARK_PNG.relative_to(ROOT))
    print(SPLASH_PNG.relative_to(ROOT))
    print(MARK_CPP.relative_to(ROOT))


if __name__ == "__main__":
    main()
