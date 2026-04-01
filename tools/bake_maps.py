#!/usr/bin/env python3
"""bake_maps.py -- Convert Tiled-compatible JSON map files to C source/header.

Usage: bake_maps.py <input_dir> <output.c> <output.h>

Reads all *.json files from input_dir, sorts by c_name, and generates C code
with map data arrays, entity spawn tables, and map transition tables.
"""

import json
import os
import sys

# Map c_name -> (MAP_ID define name, index)
MAP_ID_TABLE = {
    "map_dfwtf_hq":    ("MAP_ID_HQ",      0),
    "map_dfw_streets":  ("MAP_ID_STREETS",  1),
    "map_taco_bongo":   ("MAP_ID_TACO",     2),
}

# Short internal names for array naming (matching original C code style)
SHORT_NAMES = {
    "map_dfwtf_hq":    "hq",
    "map_dfw_streets":  "streets",
    "map_taco_bongo":   "taco",
}

# Entity type string -> C enum name
ENTITY_TYPES = {
    "ENT_NONE":           "ENT_NONE",
    "ENT_PLAYER_TREVOR":  "ENT_PLAYER_TREVOR",
    "ENT_PLAYER_KIP":     "ENT_PLAYER_KIP",
    "ENT_NPC":            "ENT_NPC",
    "ENT_ITEM":           "ENT_ITEM",
    "ENT_TRIGGER":        "ENT_TRIGGER",
}

# Direction string -> C enum value
DIRECTION_VALUES = {
    "DIR_DOWN":  0,
    "DIR_UP":    1,
    "DIR_LEFT":  2,
    "DIR_RIGHT": 3,
}

# Sprite base string -> numeric tile value
SPRITE_BASES = {
    "SPR_TREVOR_BASE":  0,
    "SPR_KIP_BASE":     32,
    "SPR_RUBIK_BASE":   64,
    "SPR_GENERIC_BASE": 96,
}

TRANSITION_CODE_START = 10


def load_maps(input_dir):
    """Load all JSON map files from the input directory."""
    maps = []
    for fname in sorted(os.listdir(input_dir)):
        if not fname.endswith(".json"):
            continue
        path = os.path.join(input_dir, fname)
        with open(path, "r") as f:
            data = json.load(f)
        maps.append((fname, data))
    # Sort by c_name property
    maps.sort(key=lambda m: m[1]["properties"]["c_name"])
    return maps


def get_layer(map_data, name):
    """Find a layer by name."""
    for layer in map_data["layers"]:
        if layer["name"] == name:
            return layer
    return None


def format_array(data, per_line=16):
    """Format a list of integer values as C array initializer lines."""
    lines = []
    for i in range(0, len(data), per_line):
        chunk = data[i:i + per_line]
        line = ",".join(str(v) for v in chunk) + ","
        lines.append("    " + line)
    return "\n".join(lines)


def extract_objects(map_data):
    """Extract entity spawn and transition objects from the objects layer."""
    obj_layer = get_layer(map_data, "objects")
    if obj_layer is None:
        return [], []

    spawns = []
    transitions = []

    for obj in obj_layer.get("objects", []):
        obj_type = obj.get("type", "")
        props = obj.get("properties", {})

        if obj_type in ("player", "npc"):
            spawn = {
                "name": obj.get("name", ""),
                "obj_type": obj_type,
                "x": obj.get("x", 0),
                "y": obj.get("y", 0),
                "properties": props,
            }
            spawns.append(spawn)
        elif obj_type == "transition":
            trans = {
                "name": obj.get("name", ""),
                "x": obj.get("x", 0),
                "y": obj.get("y", 0),
                "width": obj.get("width", 8),
                "height": obj.get("height", 8),
                "properties": props,
            }
            transitions.append(trans)

    return spawns, transitions


def resolve_entity_type(spawn):
    """Determine the EntityType enum value for a spawn."""
    props = spawn["properties"]
    if spawn["obj_type"] == "player":
        etype = props.get("entity_type", "ENT_NONE")
        return ENTITY_TYPES.get(etype, "ENT_NONE")
    elif spawn["obj_type"] == "npc":
        return "ENT_NPC"
    return "ENT_NONE"


def resolve_direction(props):
    """Get direction value from properties."""
    dir_str = props.get("direction", "DIR_DOWN")
    return DIRECTION_VALUES.get(dir_str, 0)


def resolve_sprite_base(props):
    """Get sprite base tile value from properties."""
    spr_str = props.get("sprite", "SPR_GENERIC_BASE")
    return SPRITE_BASES.get(spr_str, 96)


def resolve_dialogue_id(props):
    """Get dialogue ID string from properties (passed through as C define)."""
    return props.get("dialogue_id", "0")


def write_collision_transitions(collision_data, transitions, map_width):
    """Write transition collision codes into the collision array.

    Each transition object gets a code starting at TRANSITION_CODE_START (10).
    The code is written into every tile covered by the transition's pixel rect.

    Returns the modified collision data and a list of
    (collision_code, target_map, spawn_x, spawn_y) tuples.
    """
    collision = list(collision_data)
    transition_records = []
    code = TRANSITION_CODE_START

    for trans in transitions:
        px = trans["x"]
        py = trans["y"]
        pw = trans["width"]
        ph = trans["height"]
        props = trans["properties"]
        target_map = props["target_map"]
        spawn_x = props["spawn_x"]
        spawn_y = props["spawn_y"]

        # Convert pixel rect to tile range
        tile_x0 = px // 8
        tile_y0 = py // 8
        tile_x1 = (px + pw + 7) // 8
        tile_y1 = (py + ph + 7) // 8

        for ty in range(tile_y0, tile_y1):
            for tx in range(tile_x0, tile_x1):
                idx = ty * map_width + tx
                if 0 <= idx < len(collision):
                    collision[idx] = code

        transition_records.append((code, target_map, spawn_x, spawn_y))
        code += 1

    return collision, transition_records


def short_name(c_name):
    """Get the short internal name for array naming."""
    return SHORT_NAMES.get(c_name, c_name.replace("map_", ""))


def generate_c(maps):
    """Generate the output .c file content."""
    lines = []
    lines.append("// gen_maps.c -- Auto-generated by bake_maps.py")
    lines.append("// Do not edit by hand.")
    lines.append('#include "gen_maps.h"')
    lines.append('#include "gen_dialogue.h"')
    lines.append("#include <stddef.h>")
    lines.append("")

    # Per-map info keyed by c_name, for building the tables later
    spawn_info = {}   # c_name -> (array_name, count) or (None, 0)
    trans_info = {}   # c_name -> (array_name, count) or (None, 0)

    for fname, map_data in maps:
        props = map_data["properties"]
        c_name = props["c_name"]
        real_w = props["real_width"]
        real_h = props["real_height"]
        grid_w = map_data["width"]
        grid_h = map_data["height"]

        sn = short_name(c_name)
        fg_arr_name = "map_%s_fg_data" % sn
        col_arr_name = "map_%s_collision" % sn
        layer_name = "map_%s_fg" % sn

        fg_layer = get_layer(map_data, "fg")
        col_layer = get_layer(map_data, "collision")

        fg_data = fg_layer["data"] if fg_layer else [0] * (grid_w * grid_h)
        col_data = col_layer["data"] if col_layer else [0] * (grid_w * grid_h)

        spawns, transitions = extract_objects(map_data)

        # Write transition collision codes into the collision array
        col_data, trans_records = write_collision_transitions(
            col_data, transitions, grid_w
        )

        # -- fg tile data --
        lines.append("static const u16 %s[%d * %d] = {" % (
            fg_arr_name, grid_w, grid_h))
        lines.append(format_array(fg_data))
        lines.append("};")
        lines.append("")

        # -- collision data --
        lines.append("static const u8 %s[%d * %d] = {" % (
            col_arr_name, grid_w, grid_h))
        lines.append(format_array(col_data))
        lines.append("};")
        lines.append("")

        # -- MapLayer struct --
        lines.append(
            "static const MapLayer %s = { .width = %d, .height = %d, "
            ".data = %s };" % (layer_name, grid_w, grid_h, fg_arr_name))
        lines.append("")

        # -- Map struct --
        lines.append("const Map %s = {" % c_name)
        lines.append("    .bg = { .width = 0, .height = 0, .data = NULL },")
        lines.append("    .fg = %s," % layer_name)
        lines.append("    .collision = %s," % col_arr_name)
        lines.append("    .width = %d," % real_w)
        lines.append("    .height = %d," % real_h)
        lines.append("};")
        lines.append("")

        # -- Entity spawns --
        spawns_arr_name = "spawns_%s" % sn
        if spawns:
            lines.append(
                "static const EntitySpawn %s[] = {" % spawns_arr_name)
            for sp in spawns:
                etype = resolve_entity_type(sp)
                p = sp["properties"]
                direction = resolve_direction(p)
                base_tile = resolve_sprite_base(p)
                walk_speed = p.get("walk_speed", 0)
                dlg_id = resolve_dialogue_id(p)
                behavior = p.get("behavior", 0)
                shoot = p.get("shoot", 0)
                brawn = p.get("brawn", 0)
                brains = p.get("brains", 0)
                talk = p.get("talk", 0)
                cool = p.get("cool", 0)
                hp = p.get("hp", 0)
                hp_max = p.get("hp_max", 0)
                fate_points = p.get("fate_points", 0)
                fate_max = p.get("fate_max", 0)

                lines.append(
                    "    { %s, %d, %d, %d, %d, %d, %s, %d,"
                    " %d,%d,%d,%d,%d, %d,%d,%d,%d }," % (
                        etype, sp["x"], sp["y"], direction, base_tile,
                        walk_speed, dlg_id, behavior,
                        shoot, brawn, brains, talk, cool,
                        hp, hp_max, fate_points, fate_max,
                    ))
            lines.append("};")
            lines.append("")
            spawn_info[c_name] = (spawns_arr_name, len(spawns))
        else:
            spawn_info[c_name] = (None, 0)

        # -- Transition records --
        trans_arr_name = "transitions_%s" % sn
        if trans_records:
            lines.append(
                "static const MapTransition %s[] = {" % trans_arr_name)
            for code, target, sx, sy in trans_records:
                lines.append("    { %d, %s, %d, %d }," % (
                    code, target, sx, sy))
            lines.append("};")
            lines.append("")
            trans_info[c_name] = (trans_arr_name, len(trans_records))
        else:
            trans_info[c_name] = (None, 0)

    # Build the ordered list of (map_id_index, c_name) for table emission
    ordered = []
    for fname, map_data in maps:
        c_name = map_data["properties"]["c_name"]
        if c_name in MAP_ID_TABLE:
            _, idx = MAP_ID_TABLE[c_name]
            ordered.append((idx, c_name))
    ordered.sort()

    # -- MapSpawnTable array (indexed by MAP_ID) --
    lines.append("const MapSpawnTable map_spawn_tables[MAP_NUM_MAPS] = {")
    for idx, c_name in ordered:
        arr_name, count = spawn_info[c_name]
        if arr_name:
            lines.append("    { %s, %d }," % (arr_name, count))
        else:
            lines.append("    { NULL, 0 },")
    lines.append("};")
    lines.append("")

    # -- MapTransitionTable array (indexed by MAP_ID) --
    lines.append(
        "const MapTransitionTable map_transition_tables[MAP_NUM_MAPS] = {")
    for idx, c_name in ordered:
        arr_name, count = trans_info[c_name]
        if arr_name:
            lines.append("    { %s, %d }," % (arr_name, count))
        else:
            lines.append("    { NULL, 0 },")
    lines.append("};")
    lines.append("")

    return "\n".join(lines)


def generate_h(maps):
    """Generate the output .h file content."""
    lines = []
    lines.append("#ifndef GEN_MAPS_H")
    lines.append("#define GEN_MAPS_H")
    lines.append('#include "map.h"')
    lines.append('#include "entity.h"')
    lines.append("")

    # MAP_ID defines in index order
    id_entries = []
    for fname, map_data in maps:
        c_name = map_data["properties"]["c_name"]
        if c_name in MAP_ID_TABLE:
            id_name, idx = MAP_ID_TABLE[c_name]
            id_entries.append((idx, id_name))
    id_entries.sort()

    for idx, id_name in id_entries:
        lines.append("#define %-16s %d" % (id_name, idx))
    lines.append("#define MAP_NUM_MAPS    %d" % len(id_entries))
    lines.append("")

    # Struct definitions
    lines.append("typedef struct {")
    lines.append("    u8 type;         // EntityType value")
    lines.append("    s32 x, y;")
    lines.append("    u8 dir;          // Direction value")
    lines.append("    u16 base_tile;")
    lines.append("    u8 walk_speed;")
    lines.append("    u16 dialogue_id;")
    lines.append("    u8 npc_behavior;")
    lines.append("    s8 shoot, brawn, brains, talk, cool;")
    lines.append("    s8 hp, hp_max, fate_points, fate_max;")
    lines.append("} EntitySpawn;")
    lines.append("")

    lines.append("typedef struct {")
    lines.append("    const EntitySpawn *spawns;")
    lines.append("    int num_spawns;")
    lines.append("} MapSpawnTable;")
    lines.append("")

    lines.append("typedef struct {")
    lines.append("    u8 collision_code;")
    lines.append("    u8 target_map_id;")
    lines.append("    s32 spawn_x, spawn_y;")
    lines.append("} MapTransition;")
    lines.append("")

    lines.append("typedef struct {")
    lines.append("    const MapTransition *transitions;")
    lines.append("    int num_transitions;")
    lines.append("} MapTransitionTable;")
    lines.append("")

    # Extern declarations for Map structs
    for fname, map_data in maps:
        c_name = map_data["properties"]["c_name"]
        lines.append("extern const Map %s;" % c_name)
    lines.append("extern const MapSpawnTable map_spawn_tables[MAP_NUM_MAPS];")
    lines.append(
        "extern const MapTransitionTable "
        "map_transition_tables[MAP_NUM_MAPS];")
    lines.append("")

    lines.append("#endif")
    lines.append("")

    return "\n".join(lines)


def main():
    if len(sys.argv) != 4:
        print("Usage: %s <input_dir> <output.c> <output.h>" % sys.argv[0],
              file=sys.stderr)
        sys.exit(1)

    input_dir = sys.argv[1]
    output_c = sys.argv[2]
    output_h = sys.argv[3]

    maps = load_maps(input_dir)

    if not maps:
        print("No .json map files found in %s" % input_dir, file=sys.stderr)
        sys.exit(1)

    c_src = generate_c(maps)
    h_src = generate_h(maps)

    with open(output_c, "w") as f:
        f.write(c_src)

    with open(output_h, "w") as f:
        f.write(h_src)


if __name__ == "__main__":
    main()
