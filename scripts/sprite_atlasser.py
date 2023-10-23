import cv2
import json
import argparse
import os

def find_rows(sprite_data, y_buffer):
    rows = []
    for sprite in sprite_data:
        if len(rows) == 0:
            rows.append([sprite])
        else:
            for row in rows:
                if sprite["y"] + y_buffer >= row[0]["y"] and sprite["y"] <= row[0]["y"] + row[0]["h"]:
                    row.append(sprite)
                    break
            else:
                rows.append([sprite])
    return rows

def generate_sprite_atlas_data(image_path, y_buffer):
    image = cv2.imread(image_path, cv2.IMREAD_UNCHANGED)

    if image is None:
        print(f"Failed to load image from {image_path}")
        return

    contours, _ = cv2.findContours(image[:, :, 3], cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    sprite_data = []

    for contour in contours:
        x, y, w, h = cv2.boundingRect(contour)
        points = [(x, y), (x + w, y), (x + w, y + h), (x, y + h)]

        sprite_data.append({
            "x": x,
            "y": y,
            "w": w,
            "h": h
        })

    rows = find_rows(sprite_data, y_buffer)
    rows.reverse()

    for row in rows:
        row.sort(key=lambda sprite: sprite["x"])

    single_array = []
    for row in rows:
        single_array += row

    return single_array

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate a sprite atlas from an image")
    parser.add_argument("-i", "--image", required=True, help="Path to the spritesheet image")
    parser.add_argument("-y", "--y_threshold", type=int, default=0, required=False, help="Y-buffer value for determining if a sprite is in a row")
    parser.add_argument("-d", "--debug", action="store_true", required=False, help="Whether or not to generate an image with the bounding boxes")
    args = parser.parse_args()

    image_path = args.image
    y_threshold = args.y_threshold
    debug = args.debug

    input_file_name = os.path.splitext(os.path.basename(image_path))[0]
    input_file_dir = os.path.dirname(image_path)
    output_json_file = f"{input_file_dir}/{input_file_name}_atlas.json"
    output_bounding_image = f"{input_file_dir}/{input_file_name}_bounding.png"

    sprite_atlas_data = generate_sprite_atlas_data(image_path, y_threshold)

    if sprite_atlas_data:
        with open(output_json_file, "w") as json_file:
            json.dump(sprite_atlas_data, json_file, indent=2)
        print(f"Sprite atlas data saved to {output_json_file}")

    if debug:
        image = cv2.imread(image_path, cv2.IMREAD_UNCHANGED)
        for i, sprite in enumerate(sprite_atlas_data):
            cv2.rectangle(image, (sprite["x"], sprite["y"]), (sprite["x"] + sprite["w"], sprite["y"] + sprite["h"]), (255, 0, 0, 255), 1)
            cv2.putText(image, str(i), (sprite["x"], sprite["y"] + sprite["h"]), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255, 255), 1)

        cv2.imwrite(output_bounding_image, image)
