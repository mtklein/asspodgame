#!/usr/bin/env python3
"""bake_maps.py -- Convert Tiled JSON map files to C source/header.

Usage: bake_maps.py <input_dir> <output.c> <output.h>

Reads all *.json map files from input_dir. Collision is derived from
tile properties in the tileset .tsj file, not from a collision layer.
"""

import json, os, sys

def build_map_tables(maps):
    """Build MAP_ID_TABLE and SHORT_NAMES from map properties."""
    id_table = {}
    short_names = {}
    for _, md in maps:
        c_name = get_map_prop(md, "c_name")
        map_id = get_map_prop(md, "map_id")
        idx = get_map_prop(md, "map_index")
        sn = get_map_prop(md, "short_name", c_name.replace("map_", ""))
        if c_name and map_id is not None and idx is not None:
            id_table[c_name] = (map_id, idx)
            short_names[c_name] = sn
    return id_table, short_names

DIRECTION_VALUES = {
    "DIR_DOWN": 0, "DIR_UP": 1, "DIR_LEFT": 2, "DIR_RIGHT": 3,
}

SPRITE_BASES = {
    "SPR_TREVOR_BASE": 0, "SPR_KIP_BASE": 32,
    "SPR_RUBIK_BASE": 64, "SPR_GENERIC_BASE": 96,
}

TRANSITION_CODE_START = 10


def load_tileset_collision(tsj_path):
    """Load collision properties per tile ID from a .tsj file."""
    with open(tsj_path) as f:
        tsj = json.load(f)
    collision = {}
    for tile in tsj.get("tiles", []):
        tid = tile["id"]
        for prop in tile.get("properties", []):
            if prop["name"] == "collision":
                collision[tid] = prop["value"]
    return collision


def get_prop(obj, name, default=None):
    """Get a named property from a Tiled object's properties list."""
    for p in obj.get("properties", []):
        if p["name"] == name:
            return p["value"]
    return default


def get_map_prop(map_data, name, default=None):
    """Get a named property from a Tiled map's properties."""
    props = map_data.get("properties", {})
    if isinstance(props, list):
        for p in props:
            if p["name"] == name:
                return p["value"]
        return default
    return props.get(name, default)


def load_maps(input_dir):
    maps = []
    for fname in sorted(os.listdir(input_dir)):
        if not fname.endswith(".json"):
            continue
        path = os.path.join(input_dir, fname)
        with open(path) as f:
            data = json.load(f)
        maps.append((fname, data))
    maps.sort(key=lambda m: get_map_prop(m[1], "c_name"))
    return maps


def get_layer(map_data, name):
    for layer in map_data["layers"]:
        if layer["name"] == name:
            return layer
    return None


def resolve_tileset(map_data, input_dir):
    """Find the tileset .tsj path and firstgid from the map's tileset refs."""
    for ts_ref in map_data.get("tilesets", []):
        source = ts_ref.get("source", "")
        firstgid = ts_ref.get("firstgid", 1)
        if source:
            # Resolve relative to the map's directory
            maps_dir = os.path.join(input_dir)
            tsj_path = os.path.normpath(os.path.join(maps_dir, source))
            return tsj_path, firstgid
    return None, 1


def gid_to_tile(gid, firstgid):
    """Convert a Tiled GID to a raw tile index."""
    if gid == 0:
        return 0  # No tile -> tile 0 (empty)
    return gid - firstgid


def derive_collision(fg_data, firstgid, tile_collision):
    """Build collision array from fg tile data and tileset collision props."""
    col = []
    for gid in fg_data:
        tile_idx = gid_to_tile(gid, firstgid)
        col.append(tile_collision.get(tile_idx, 0))
    return col


def format_array(data, per_line=16):
    lines = []
    for i in range(0, len(data), per_line):
        chunk = data[i:i + per_line]
        lines.append("    " + ",".join(str(v) for v in chunk) + ",")
    return "\n".join(lines)


def extract_objects(map_data):
    obj_layer = get_layer(map_data, "objects")
    if not obj_layer:
        return [], []
    spawns, transitions = [], []
    for obj in obj_layer.get("objects", []):
        obj_type = obj.get("type", "")
        if obj_type in ("player", "npc"):
            spawns.append(obj)
        elif obj_type == "transition":
            transitions.append(obj)
    return spawns, transitions


def write_collision_transitions(collision, transitions, grid_w):
    collision = list(collision)
    records = []
    code = TRANSITION_CODE_START
    for tr in transitions:
        px, py = int(tr["x"]), int(tr["y"])
        pw, ph = int(tr["width"]), int(tr["height"])
        target = get_prop(tr, "target_map")
        sx = get_prop(tr, "spawn_x", 0)
        sy = get_prop(tr, "spawn_y", 0)
        tx0, ty0 = px // 8, py // 8
        tx1, ty1 = (px + pw + 7) // 8, (py + ph + 7) // 8
        for ty in range(ty0, ty1):
            for tx in range(tx0, tx1):
                idx = ty * grid_w + tx
                if 0 <= idx < len(collision):
                    collision[idx] = code
        records.append((code, target, sx, sy))
        code += 1
    return collision, records


def resolve_entity_type(obj):
    obj_type = obj.get("type", "")
    if obj_type == "player":
        return get_prop(obj, "entity_type", "ENT_NONE")
    return "ENT_NPC"


def emit_spawn(obj, lines):
    etype = resolve_entity_type(obj)
    d = DIRECTION_VALUES.get(get_prop(obj, "direction", "DIR_DOWN"), 0)
    bt = SPRITE_BASES.get(get_prop(obj, "sprite", "SPR_GENERIC_BASE"), 96)
    ws = get_prop(obj, "walk_speed", 0)
    dlg = get_prop(obj, "dialogue_id", "0")
    beh = get_prop(obj, "behavior", 0)
    sh = get_prop(obj, "shoot", 0)
    br = get_prop(obj, "brawn", 0)
    bn = get_prop(obj, "brains", 0)
    tk = get_prop(obj, "talk", 0)
    co = get_prop(obj, "cool", 0)
    hp = get_prop(obj, "hp", 0)
    hm = get_prop(obj, "hp_max", 0)
    fp = get_prop(obj, "fate_points", 0)
    fm = get_prop(obj, "fate_max", 0)
    lines.append(
        "    { %s, %d, %d, %d, %d, %d, %s, %d,"
        " %d,%d,%d,%d,%d, %d,%d,%d,%d }," % (
            etype, int(obj["x"]), int(obj["y"]), d, bt, ws, dlg, beh,
            sh, br, bn, tk, co, hp, hm, fp, fm))


def generate_c(maps, input_dir, id_table, short_names):
    lines = [
        "// gen_maps.c -- Auto-generated by bake_maps.py",
        "// Do not edit by hand.",
        '#include "gen_maps.h"',
        '#include "gen_dialogue.h"',
        "#include <stddef.h>",
        "",
    ]

    spawn_info = {}
    trans_info = {}

    for fname, map_data in maps:
        c_name = get_map_prop(map_data, "c_name")
        real_w = get_map_prop(map_data, "real_width")
        real_h = get_map_prop(map_data, "real_height")
        grid_w = map_data["width"]
        grid_h = map_data["height"]
        sn = short_names.get(c_name, c_name.replace("map_", ""))

        # Resolve tileset
        tsj_path, firstgid = resolve_tileset(map_data, input_dir)
        tile_collision = load_tileset_collision(tsj_path) if tsj_path else {}

        # Get fg layer and convert GIDs to raw tile indices
        fg_layer = get_layer(map_data, "fg")
        fg_gids = fg_layer["data"] if fg_layer else [0] * (grid_w * grid_h)
        fg_data = [gid_to_tile(g, firstgid) for g in fg_gids]

        # Derive collision from tile properties
        col_data = derive_collision(fg_gids, firstgid, tile_collision)

        # Extract objects
        spawns, transitions = extract_objects(map_data)

        # Write transitions into collision
        col_data, trans_records = write_collision_transitions(
            col_data, transitions, grid_w)

        # Emit arrays
        fg_arr = "map_%s_fg_data" % sn
        col_arr = "map_%s_collision" % sn
        layer_name = "map_%s_fg" % sn

        lines.append("static const u16 %s[%d * %d] = {" % (fg_arr, grid_w, grid_h))
        lines.append(format_array(fg_data))
        lines.append("};")
        lines.append("")

        lines.append("static const u8 %s[%d * %d] = {" % (col_arr, grid_w, grid_h))
        lines.append(format_array(col_data))
        lines.append("};")
        lines.append("")

        lines.append("static const MapLayer %s = { .width = %d, .height = %d, .data = %s };"
                      % (layer_name, grid_w, grid_h, fg_arr))
        lines.append("")

        lines.append("const Map %s = {" % c_name)
        lines.append("    .bg = { .width = 0, .height = 0, .data = NULL },")
        lines.append("    .fg = %s," % layer_name)
        lines.append("    .collision = %s," % col_arr)
        lines.append("    .width = %d," % real_w)
        lines.append("    .height = %d," % real_h)
        lines.append("};")
        lines.append("")

        # Spawns
        spawns_arr = "spawns_%s" % sn
        if spawns:
            lines.append("static const EntitySpawn %s[] = {" % spawns_arr)
            for sp in spawns:
                emit_spawn(sp, lines)
            lines.append("};")
            lines.append("")
            spawn_info[c_name] = (spawns_arr, len(spawns))
        else:
            spawn_info[c_name] = (None, 0)

        # Transitions
        trans_arr = "transitions_%s" % sn
        if trans_records:
            lines.append("static const MapTransition %s[] = {" % trans_arr)
            for code, target, sx, sy in trans_records:
                lines.append("    { %d, %s, %d, %d }," % (code, target, sx, sy))
            lines.append("};")
            lines.append("")
            trans_info[c_name] = (trans_arr, len(trans_records))
        else:
            trans_info[c_name] = (None, 0)

    # Ordered tables
    ordered = []
    for _, md in maps:
        cn = get_map_prop(md, "c_name")
        if cn in id_table:
            ordered.append((id_table[cn][1], cn))
    ordered.sort()

    lines.append("const MapSpawnTable map_spawn_tables[MAP_NUM_MAPS] = {")
    for _, cn in ordered:
        arr, cnt = spawn_info[cn]
        lines.append("    { %s, %d }," % (arr or "NULL", cnt))
    lines.append("};")
    lines.append("")

    lines.append("const MapTransitionTable map_transition_tables[MAP_NUM_MAPS] = {")
    for _, cn in ordered:
        arr, cnt = trans_info[cn]
        lines.append("    { %s, %d }," % (arr or "NULL", cnt))
    lines.append("};")
    lines.append("")

    return "\n".join(lines)


def generate_h(maps, id_table):
    lines = [
        "#ifndef GEN_MAPS_H",
        "#define GEN_MAPS_H",
        '#include "map.h"',
        '#include "entity.h"',
        "",
    ]

    entries = []
    for _, md in maps:
        cn = get_map_prop(md, "c_name")
        if cn in id_table:
            entries.append((id_table[cn][1], id_table[cn][0]))
    entries.sort()

    for idx, name in entries:
        lines.append("#define %-16s %d" % (name, idx))
    lines.append("#define MAP_NUM_MAPS    %d" % len(entries))
    lines.append("")

    lines += [
        "typedef struct {",
        "    u8 type;",
        "    s32 x, y;",
        "    u8 dir;",
        "    u16 base_tile;",
        "    u8 walk_speed;",
        "    u16 dialogue_id;",
        "    u8 npc_behavior;",
        "    s8 shoot, brawn, brains, talk, cool;",
        "    s8 hp, hp_max, fate_points, fate_max;",
        "} EntitySpawn;",
        "",
        "typedef struct {",
        "    const EntitySpawn *spawns;",
        "    int num_spawns;",
        "} MapSpawnTable;",
        "",
        "typedef struct {",
        "    u8 collision_code;",
        "    u8 target_map_id;",
        "    s32 spawn_x, spawn_y;",
        "} MapTransition;",
        "",
        "typedef struct {",
        "    const MapTransition *transitions;",
        "    int num_transitions;",
        "} MapTransitionTable;",
        "",
    ]

    for _, md in maps:
        cn = get_map_prop(md, "c_name")
        lines.append("extern const Map %s;" % cn)
    lines.append("extern const MapSpawnTable map_spawn_tables[MAP_NUM_MAPS];")
    lines.append("extern const MapTransitionTable map_transition_tables[MAP_NUM_MAPS];")
    lines.append("")
    lines.append("#endif")
    lines.append("")

    return "\n".join(lines)


def main():
    if len(sys.argv) != 4:
        print("Usage: %s <input_dir> <output.c> <output.h>" % sys.argv[0],
              file=sys.stderr)
        sys.exit(1)

    input_dir, output_c, output_h = sys.argv[1], sys.argv[2], sys.argv[3]
    maps = load_maps(input_dir)
    if not maps:
        print("No .json map files found in %s" % input_dir, file=sys.stderr)
        sys.exit(1)

    id_table, short_names = build_map_tables(maps)

    with open(output_c, "w") as f:
        f.write(generate_c(maps, input_dir, id_table, short_names))
    with open(output_h, "w") as f:
        f.write(generate_h(maps, id_table))


if __name__ == "__main__":
    main()
