#!/usr/bin/env python3
"""gen_sprites.py -- Generate character sprites via ComfyUI from description files.

Usage: gen_sprites.py <character.json> [--output DIR] [--workflow WORKFLOW.json]
       gen_sprites.py levels/characters/trevor_steel.json --output out/sprites/

Reads a character description JSON, composes a pixel art prompt from the
appearance fields, submits it to a running ComfyUI instance, and saves
the output PNG to the specified directory.

Requires ComfyUI running at http://127.0.0.1:8188 (default).
"""

import json, os, sys, time, urllib.request, urllib.error, uuid


COMFYUI_URL = os.environ.get("COMFYUI_URL", "http://127.0.0.1:8000")

# Default workflow template for pixel art sprite generation.
# This is an SD 1.5 txt2img workflow. The model, prompt, and output
# filename are patched before submission.
DEFAULT_WORKFLOW = {
    "3": {
        "class_type": "KSampler",
        "inputs": {
            "seed": 0,
            "steps": 20,
            "cfg": 7.0,
            "sampler_name": "euler_ancestral",
            "scheduler": "normal",
            "denoise": 1.0,
            "model": ["4", 0],
            "positive": ["6", 0],
            "negative": ["7", 0],
            "latent_image": ["5", 0]
        }
    },
    "4": {
        "class_type": "CheckpointLoaderSimple",
        "inputs": {
            "ckpt_name": "allInOnePixelModel_v1.ckpt"
        }
    },
    "5": {
        "class_type": "EmptyLatentImage",
        "inputs": {
            "width": 128,
            "height": 64,
            "batch_size": 1
        }
    },
    "6": {
        "class_type": "CLIPTextEncode",
        "inputs": {
            "text": "",
            "clip": ["4", 1]
        }
    },
    "7": {
        "class_type": "CLIPTextEncode",
        "inputs": {
            "text": "blurry, 3d, realistic, photo, high resolution, text, watermark",
            "clip": ["4", 1]
        }
    },
    "8": {
        "class_type": "VAEDecode",
        "inputs": {
            "samples": ["3", 0],
            "vae": ["4", 2]
        }
    },
    "9": {
        "class_type": "SaveImage",
        "inputs": {
            "filename_prefix": "sprite",
            "images": ["8", 0]
        }
    }
}


def build_prompt(char_data):
    """Compose a pixel art generation prompt from character description."""
    app = char_data.get("appearance", {})
    name = char_data.get("name", "character")

    parts = ["pixelsprite", "16bit", "game sprite sheet",
             "4 directions side by side", "front back left right",
             "2 rows walk cycle", "transparent background",
             "tiny character", "top-down RPG"]

    summary = app.get("summary", "")
    if summary:
        # Take the first two sentences of the appearance summary
        sentences = summary.split(". ")
        parts.append(". ".join(sentences[:2]))

    build = app.get("build", "")
    if build:
        parts.append(build)

    outfit = app.get("outfit", "")
    if outfit:
        parts.append(outfit)

    hair = app.get("hair", "")
    if hair:
        parts.append(hair + " hair")

    return ", ".join(parts)


def submit_workflow(workflow, prompt_text, seed=None, filename_prefix="sprite"):
    """Submit a workflow to ComfyUI and return the prompt_id."""
    wf = json.loads(json.dumps(workflow))  # deep copy

    # Patch the prompt text
    wf["6"]["inputs"]["text"] = prompt_text

    # Patch seed
    if seed is not None:
        wf["3"]["inputs"]["seed"] = seed
    else:
        import random
        wf["3"]["inputs"]["seed"] = random.randint(0, 2**32 - 1)

    # Patch output filename
    wf["9"]["inputs"]["filename_prefix"] = filename_prefix

    payload = json.dumps({"prompt": wf}).encode()
    req = urllib.request.Request(
        COMFYUI_URL + "/prompt",
        data=payload,
        headers={"Content-Type": "application/json"},
    )
    resp = urllib.request.urlopen(req)
    result = json.loads(resp.read())
    return result.get("prompt_id")


def wait_for_completion(prompt_id, timeout=120):
    """Poll ComfyUI until the prompt completes."""
    start = time.time()
    while time.time() - start < timeout:
        try:
            resp = urllib.request.urlopen(COMFYUI_URL + "/history/" + prompt_id)
            history = json.loads(resp.read())
            if prompt_id in history:
                return history[prompt_id]
        except urllib.error.URLError:
            pass
        time.sleep(1)
    raise TimeoutError("ComfyUI prompt timed out after %ds" % timeout)


def get_output_images(history_entry):
    """Extract output image filenames from a history entry."""
    images = []
    outputs = history_entry.get("outputs", {})
    for node_id, node_out in outputs.items():
        for img in node_out.get("images", []):
            images.append(img)
    return images


def download_image(image_info, output_dir):
    """Download a generated image from ComfyUI."""
    filename = image_info["filename"]
    subfolder = image_info.get("subfolder", "")
    img_type = image_info.get("type", "output")

    url = "%s/view?filename=%s&subfolder=%s&type=%s" % (
        COMFYUI_URL, filename, subfolder, img_type)

    os.makedirs(output_dir, exist_ok=True)
    out_path = os.path.join(output_dir, filename)
    urllib.request.urlretrieve(url, out_path)
    return out_path


def main():
    if len(sys.argv) < 2:
        print("Usage: gen_sprites.py <character.json> [--output DIR] [--seed N] [--workflow FILE]",
              file=sys.stderr)
        sys.exit(1)

    char_path = sys.argv[1]
    output_dir = "out/sprites"
    seed = None
    workflow_path = None

    i = 2
    while i < len(sys.argv):
        if sys.argv[i] == "--output" and i + 1 < len(sys.argv):
            output_dir = sys.argv[i + 1]; i += 2
        elif sys.argv[i] == "--seed" and i + 1 < len(sys.argv):
            seed = int(sys.argv[i + 1]); i += 2
        elif sys.argv[i] == "--workflow" and i + 1 < len(sys.argv):
            workflow_path = sys.argv[i + 1]; i += 2
        else:
            i += 1

    with open(char_path) as f:
        char_data = json.load(f)

    char_id = char_data.get("id", "unknown")
    prompt_text = build_prompt(char_data)
    print("Character: %s" % char_data.get("name", char_id))
    print("Prompt: %s" % prompt_text)

    if workflow_path:
        with open(workflow_path) as f:
            workflow = json.load(f)
    else:
        workflow = DEFAULT_WORKFLOW

    # Check ComfyUI is running
    try:
        urllib.request.urlopen(COMFYUI_URL + "/system_stats")
    except urllib.error.URLError:
        print("ERROR: ComfyUI not running at %s" % COMFYUI_URL, file=sys.stderr)
        print("Start it with: open -a ComfyUI", file=sys.stderr)
        sys.exit(1)

    print("Submitting to ComfyUI...")
    prompt_id = submit_workflow(workflow, prompt_text, seed=seed,
                                filename_prefix="sprite_%s" % char_id)
    print("Prompt ID: %s" % prompt_id)
    print("Waiting for generation...")

    history = wait_for_completion(prompt_id)
    images = get_output_images(history)

    if not images:
        print("ERROR: No images generated", file=sys.stderr)
        sys.exit(1)

    for img in images:
        path = download_image(img, output_dir)
        print("Saved: %s" % path)

    print("Done. Use import_sprite.py to convert to GBA format.")


if __name__ == "__main__":
    main()
