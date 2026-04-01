#!/usr/bin/env python3
"""bake_dialogue.py - Convert dialogue JSON files into C source/header for GBA."""

import json
import os
import sys

MAX_CHOICES = 3


def c_escape(s):
    """Escape a string for use in a C string literal.

    JSON already decodes \\n into real newlines, so we re-encode them as \\n
    for the C source. Backslashes and double quotes are also escaped.
    """
    out = []
    for ch in s:
        if ch == '\\':
            out.append('\\\\')
        elif ch == '"':
            out.append('\\"')
        elif ch == '\n':
            out.append('\\n')
        else:
            out.append(ch)
    return ''.join(out)


def load_trees(input_dir):
    """Load all *.json dialogue files from input_dir, sorted by index."""
    trees = []
    for fname in os.listdir(input_dir):
        if not fname.endswith('.json'):
            continue
        path = os.path.join(input_dir, fname)
        with open(path, 'r') as f:
            tree = json.load(f)
        trees.append(tree)
    trees.sort(key=lambda t: t['index'])
    return trees


def gen_c(trees):
    """Generate the .c file contents."""
    lines = []
    lines.append('#include "gen_dialogue.h"')
    lines.append('#include <stddef.h>')
    lines.append('')

    # Emit each node array
    for tree in trees:
        dlg_id = tree['id']
        # Derive the C array name from the id: DLG_RUBIK_INTRO -> dlg_rubik_intro_nodes
        array_name = dlg_id.lower() + '_nodes'
        num_nodes = len(tree['nodes'])

        lines.append(f'static const DialogueNode {array_name}[] = {{')

        for node in tree['nodes']:
            speaker = c_escape(node['speaker'])
            text = c_escape(node['text'])

            has_choices = 'choices' in node
            if has_choices:
                choices = node['choices']
                num_choices = len(choices)
            else:
                choices = []
                num_choices = 0

            # Determine next value
            if has_choices:
                next_val = '0xFFFF'
            else:
                raw_next = node.get('next')
                if raw_next is None:
                    next_val = '0xFFFF'
                else:
                    next_val = str(raw_next)

            # Build choices array string
            choice_parts = []
            for c in choices:
                c_text = c_escape(c['text'])
                c_next = c['next']
                c_fate = c.get('fate', 0)
                choice_parts.append(f'{{ "{c_text}", {c_next}, {c_fate} }}')

            # Pad remaining slots with {0}
            for _ in range(MAX_CHOICES - len(choices)):
                choice_parts.append('{0}')

            choices_str = ', '.join(choice_parts)

            if num_choices == 0:
                # Compact single-line format for no-choice nodes
                lines.append(f'    {{ .speaker = "{speaker}", .text = "{text}",')
                lines.append(f'      .num_choices = 0, .next = {next_val}, .choices = {{{{0}}}} }},')
            else:
                lines.append(f'    {{ .speaker = "{speaker}", .text = "{text}",')
                lines.append(f'      .num_choices = {num_choices}, .next = {next_val},')
                lines.append(f'      .choices = {{')
                for cp in choice_parts[:num_choices]:
                    lines.append(f'        {cp},')
                lines.append(f'      }}')
                lines.append(f'    }},')

        lines.append('};')
        lines.append('')

    # Master table
    max_index = max(t['index'] for t in trees)
    num_trees = max_index + 1

    lines.append(f'const DialogueTree dialogue_trees[NUM_DIALOGUE_TREES] = {{')
    lines.append(f'    {{ .nodes = NULL,{" " * 17}.num_nodes = 0 }},  // 0: unused')

    # Build a lookup by index
    by_index = {t['index']: t for t in trees}

    for i in range(1, num_trees):
        tree = by_index[i]
        array_name = tree['id'].lower() + '_nodes'
        num_nodes = len(tree['nodes'])
        # Pad array name to align .num_nodes
        padded = f'.nodes = {array_name},'
        padded = padded.ljust(35)
        lines.append(f'    {{ {padded}.num_nodes = {num_nodes} }},  // {i}')

    lines.append('};')
    lines.append('')

    return '\n'.join(lines)


def gen_h(trees):
    """Generate the .h file contents."""
    max_index = max(t['index'] for t in trees)
    num_trees = max_index + 1

    lines = []
    lines.append('#ifndef GEN_DIALOGUE_H')
    lines.append('#define GEN_DIALOGUE_H')
    lines.append('')
    lines.append('#include "dialogue.h"')
    lines.append('')
    lines.append(f'#define NUM_DIALOGUE_TREES {num_trees}')
    lines.append('')

    for tree in trees:
        dlg_id = tree['id']
        idx = tree['index']
        # Pad the define name for alignment
        padded = dlg_id.ljust(20)
        lines.append(f'#define {padded}{idx}')

    lines.append('')
    lines.append(f'extern const DialogueTree dialogue_trees[NUM_DIALOGUE_TREES];')
    lines.append('')
    lines.append('#endif')
    lines.append('')

    return '\n'.join(lines)


def main():
    if len(sys.argv) != 4:
        print(f'Usage: {sys.argv[0]} <input_dir> <output.c> <output.h>', file=sys.stderr)
        sys.exit(1)

    input_dir = sys.argv[1]
    out_c = sys.argv[2]
    out_h = sys.argv[3]

    trees = load_trees(input_dir)
    if not trees:
        print(f'Error: no .json files found in {input_dir}', file=sys.stderr)
        sys.exit(1)

    c_code = gen_c(trees)
    h_code = gen_h(trees)

    with open(out_c, 'w') as f:
        f.write(c_code)

    with open(out_h, 'w') as f:
        f.write(h_code)


if __name__ == '__main__':
    main()
