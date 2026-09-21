from PIL import Image
from pathlib import Path

def rgb888_to_rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def letterbox_resize(img, target_w, target_h):
    src_w, src_h = img.size

    scale = min(target_w / src_w, target_h / src_h)
    new_w = int(src_w * scale)
    new_h = int(src_h * scale)

    img_resized = img.resize((new_w, new_h), Image.Resampling.LANCZOS)

    canvas = Image.new("RGB", (target_w, target_h), (0, 0, 0))

    offset_x = (target_w - new_w) // 2
    offset_y = (target_h - new_h) // 2

    canvas.paste(img_resized, (offset_x, offset_y))

    return canvas

def image_to_rgb565(img):
    pixels = img.load()
    w, h = img.size

    data = []
    for y in range(h):
        for x in range(w):
            r, g, b = pixels[x, y]
            data.append(rgb888_to_rgb565(r, g, b))

    return data, w, h

def format_arduino(data, name):
    out = []
    out.append("#include <Arduino.h>\n")
    out.append(f"const uint16_t {name}[] PROGMEM = {{")

    for i, val in enumerate(data):
        if i % 10 == 0:
            out.append("  ")
        out[-1] += f"0x{val:04X}, "

    out.append("};")
    return "\n".join(out)

def main():
    print("=== ST7735 RGB565 Bitmap Generator ===\n")

    base_dir = Path(__file__).resolve().parent

    rel_path = input("Enter image path (relative to script folder): ").strip()
    full_path = base_dir / rel_path

    if not full_path.exists():
        print(f"\nERROR: File not found -> {full_path}")
        return

    width = int(input("Enter target width: "))
    height = int(input("Enter target height: "))

    print("\nLoading image...")
    img = Image.open(full_path).convert("RGB")

    print("Resizing (no cropping, letterbox mode)...")
    img = letterbox_resize(img, width, height)

    print("Converting to RGB565...")
    data, w, h = image_to_rgb565(img)

    # Create safe names
    image_name = full_path.stem.replace(" ", "_")
    array_name = f"{image_name}_{w}x{h}"

    print("Generating Arduino code...")
    code = format_arduino(data, array_name)

    # Output file name
    output_file = base_dir / f"{image_name}_{w}x{h}.txt"

    print(f"\nSaving to: {output_file}")

    with open(output_file, "w", encoding="utf-8") as f:
        f.write(f"// Original file: {full_path.name}\n")
        f.write(f"// Resolution: {w} x {h}\n\n")
        f.write(code)

    print("Done!")

if __name__ == "__main__":
    main()