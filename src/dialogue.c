// dialogue.c — Dialogue system implementation
#include "dialogue.h"
#include "input.h"
#include "entity.h"
#include "game_data.h"

// Dialogue box uses BG2 as a text layer
#define DLG_SBB     24   // Screenblock for dialogue text
#define DLG_CBB     2    // Charblock for font tiles

// Text box dimensions (in tiles)
#define BOX_X       1
#define BOX_Y       15   // Bottom of screen
#define BOX_W       28
#define BOX_H       5
#define TEXT_X      3    // Text offset within box (room for portrait)
#define TEXT_Y      16
#define TEXT_W      24
#define NAME_X     3
#define NAME_Y     15

// Print speed (frames per character)
#define PRINT_SPEED 2

DialogueState dlg_state = DLG_INACTIVE;

static u16 current_tree_id;
static u16 current_node;
static int char_index;      // How many characters revealed so far
static int print_timer;
static int choice_cursor;   // Current choice selection

// Get current tree
static const DialogueTree* get_tree(void) {
    if (current_tree_id < NUM_DIALOGUE_TREES) {
        return &dialogue_trees[current_tree_id];
    }
    return 0;
}

static const DialogueNode* get_node(void) {
    const DialogueTree *tree = get_tree();
    if (tree && current_node < (u16)tree->num_nodes) {
        return &tree->nodes[current_node];
    }
    return 0;
}

void dialogue_start(u16 tree_id) {
    current_tree_id = tree_id;
    current_node = 0;
    char_index = 0;
    print_timer = 0;
    choice_cursor = 0;
    dlg_state = DLG_PRINTING;

    // Enable BG2 for text overlay
    REG_DISPCNT |= DCNT_BG2;
    REG_BG2CNT = BG_PRIO(0) | BG_CBB(DLG_CBB) | BG_SBB(DLG_SBB) | BG_4BPP | BG_SIZE_256x256;
    REG_BG2HOFS = 0;
    REG_BG2VOFS = 0;

    // Draw the box background tiles
    text_clear_region(DLG_SBB, 0, 0, 32, 20);

    // Draw box border (using special border tiles)
    u16 *sbb = (u16*)SBB_ADDR(DLG_SBB);
    for (int x = BOX_X; x < BOX_X + BOX_W; x++) {
        for (int y = BOX_Y; y < BOX_Y + BOX_H; y++) {
            u16 tile = 1; // Filled box tile
            if (y == BOX_Y) tile = (x == BOX_X) ? 2 : (x == BOX_X + BOX_W - 1) ? 4 : 3;
            else if (y == BOX_Y + BOX_H - 1) tile = (x == BOX_X) ? 7 : (x == BOX_X + BOX_W - 1) ? 9 : 8;
            else if (x == BOX_X) tile = 5;
            else if (x == BOX_X + BOX_W - 1) tile = 6;

            sbb[y * 32 + x] = SE_TILE(tile) | SE_PALBANK(15);
        }
    }
}

void dialogue_update(void) {
    if (dlg_state == DLG_INACTIVE) return;

    const DialogueNode *node = get_node();
    if (!node) {
        dlg_state = DLG_INACTIVE;
        REG_DISPCNT &= ~DCNT_BG2;
        return;
    }

    switch (dlg_state) {
    case DLG_PRINTING: {
        print_timer++;
        if (print_timer >= PRINT_SPEED) {
            print_timer = 0;
            char_index++;

            // Check if all text revealed
            int len = 0;
            const char *p = node->text;
            while (*p) { len++; p++; }

            if (char_index >= len) {
                dlg_state = (node->num_choices > 0) ? DLG_CHOOSING : DLG_WAITING;
            }
        }

        // Press A or B to instant-reveal
        if (key_hit(KEY_A) || key_hit(KEY_B)) {
            int len = 0;
            const char *p = node->text;
            while (*p) { len++; p++; }
            char_index = len;
            dlg_state = (node->num_choices > 0) ? DLG_CHOOSING : DLG_WAITING;
        }
        break;
    }

    case DLG_WAITING:
        if (key_hit(KEY_A)) {
            if (node->next == 0) {
                // End of dialogue
                dlg_state = DLG_INACTIVE;
                REG_DISPCNT &= ~DCNT_BG2;
            } else {
                current_node = node->next;
                char_index = 0;
                print_timer = 0;
                dlg_state = DLG_PRINTING;
            }
        }
        break;

    case DLG_CHOOSING: {
        // Navigate choices
        if (key_hit(KEY_UP) && choice_cursor > 0) choice_cursor--;
        if (key_hit(KEY_DOWN) && choice_cursor < node->num_choices - 1) choice_cursor++;

        if (key_hit(KEY_A)) {
            const DialogueTree *tree = get_tree();
            int offset = tree->choice_offsets[current_node];
            const DialogueChoice *choice = &tree->choices[offset + choice_cursor];

            // Award fate points
            if (choice->fate_reward > 0) {
                Entity *player = &entities[active_player];
                player->stats.fate_points += choice->fate_reward;
                if (player->stats.fate_points > player->stats.fate_max) {
                    player->stats.fate_points = player->stats.fate_max;
                }
            }

            // Stat check (if any)
            if (choice->stat_check > 0) {
                // TODO: implement dice roll + stat vs DC
                // For now, auto-pass
            }

            // Advance to next node
            if (choice->next_node == 0) {
                dlg_state = DLG_INACTIVE;
                REG_DISPCNT &= ~DCNT_BG2;
            } else {
                current_node = choice->next_node;
                char_index = 0;
                print_timer = 0;
                choice_cursor = 0;
                dlg_state = DLG_PRINTING;
            }
        }
        break;
    }

    default:
        break;
    }
}

void dialogue_draw(void) {
    if (dlg_state == DLG_INACTIVE) return;

    const DialogueNode *node = get_node();
    if (!node) return;

    // Clear text area
    text_clear_region(DLG_SBB, TEXT_X, TEXT_Y, TEXT_W, 3);
    text_clear_region(DLG_SBB, NAME_X, NAME_Y, 12, 1);

    // Draw speaker name
    if (node->speaker) {
        text_draw_string(node->speaker, DLG_SBB, NAME_X, NAME_Y, 12);
    }

    // Draw text (up to char_index characters)
    // We need a temporary truncated string
    // Since we can't easily truncate, we pass char_index as a length limit
    // handled inside text_draw_string
    if (node->text) {
        // Draw with word wrap, up to char_index chars
        int cx = TEXT_X;
        int cy = TEXT_Y;
        int count = 0;
        const char *p = node->text;

        u16 *sbb = (u16*)SBB_ADDR(DLG_SBB);

        while (*p && count < char_index) {
            if (*p == '\n' || (cx >= TEXT_X + TEXT_W)) {
                cx = TEXT_X;
                cy++;
                if (*p == '\n') { p++; count++; continue; }
            }
            if (cy >= TEXT_Y + 3) break; // Max 3 lines

            // Map ASCII to font tile index (font starts at tile 32 in CBB)
            int tile_idx = 32 + (*p - ' ');
            if (*p < ' ' || *p > '~') tile_idx = 32; // Space for unknown
            sbb[cy * 32 + cx] = SE_TILE(tile_idx) | SE_PALBANK(15);

            cx++;
            p++;
            count++;
        }
    }

    // Draw choices if in choosing state
    if (dlg_state == DLG_CHOOSING && node->num_choices > 0) {
        const DialogueTree *tree = get_tree();
        int offset = tree->choice_offsets[current_node];

        for (int i = 0; i < node->num_choices && i < 3; i++) {
            const DialogueChoice *c = &tree->choices[offset + i];
            int cy = TEXT_Y + i;
            int cx = TEXT_X + 1;

            // Draw cursor
            u16 *sbb = (u16*)SBB_ADDR(DLG_SBB);
            sbb[cy * 32 + TEXT_X] = SE_TILE(i == choice_cursor ? (32 + '>'-' ') : 32) | SE_PALBANK(15);

            if (c->text) {
                text_draw_string(c->text, DLG_SBB, cx, cy, TEXT_W - 1);
            }
        }
    }
}

// --- Text rendering ---

// A simple 1bpp → 4bpp font stored in code
// We'll use a minimal built-in font and load it to CBB 2
#include "font_data.h"

void text_init(void) {
    // Load font tiles into charblock 2
    // Tile 0 = transparent, tiles 1-9 = box border, tiles 32+ = ASCII font
    u32 *dest = (u32*)CBB_ADDR(DLG_CBB);

    // Tile 0: fully transparent
    for (int i = 0; i < 8; i++) dest[i] = 0;

    // Tiles 1-9: box border tiles (simple solid/border patterns)
    // Tile 1: solid fill
    for (int i = 8; i < 16; i++) dest[i] = 0x11111111;
    // Tile 2: top-left corner
    dest[16] = 0x22222222; dest[17] = 0x21111111; dest[18] = 0x21111111; dest[19] = 0x21111111;
    dest[20] = 0x21111111; dest[21] = 0x21111111; dest[22] = 0x21111111; dest[23] = 0x21111111;
    // Tile 3: top edge
    dest[24] = 0x22222222; dest[25] = 0x11111111; dest[26] = 0x11111111; dest[27] = 0x11111111;
    dest[28] = 0x11111111; dest[29] = 0x11111111; dest[30] = 0x11111111; dest[31] = 0x11111111;
    // Tile 4: top-right corner
    dest[32] = 0x22222222; dest[33] = 0x11111112; dest[34] = 0x11111112; dest[35] = 0x11111112;
    dest[36] = 0x11111112; dest[37] = 0x11111112; dest[38] = 0x11111112; dest[39] = 0x11111112;
    // Tile 5: left edge
    dest[40] = 0x21111111; dest[41] = 0x21111111; dest[42] = 0x21111111; dest[43] = 0x21111111;
    dest[44] = 0x21111111; dest[45] = 0x21111111; dest[46] = 0x21111111; dest[47] = 0x21111111;
    // Tile 6: right edge
    dest[48] = 0x11111112; dest[49] = 0x11111112; dest[50] = 0x11111112; dest[51] = 0x11111112;
    dest[52] = 0x11111112; dest[53] = 0x11111112; dest[54] = 0x11111112; dest[55] = 0x11111112;
    // Tile 7: bottom-left
    dest[56] = 0x21111111; dest[57] = 0x21111111; dest[58] = 0x21111111; dest[59] = 0x21111111;
    dest[60] = 0x21111111; dest[61] = 0x21111111; dest[62] = 0x21111111; dest[63] = 0x22222222;
    // Tile 8: bottom edge
    dest[64] = 0x11111111; dest[65] = 0x11111111; dest[66] = 0x11111111; dest[67] = 0x11111111;
    dest[68] = 0x11111111; dest[69] = 0x11111111; dest[70] = 0x11111111; dest[71] = 0x22222222;
    // Tile 9: bottom-right
    dest[72] = 0x11111112; dest[73] = 0x11111112; dest[74] = 0x11111112; dest[75] = 0x11111112;
    dest[76] = 0x11111112; dest[77] = 0x11111112; dest[78] = 0x11111112; dest[79] = 0x22222222;

    // Load ASCII font (space=32 to tilde=126 → tiles 32..126)
    // font_data is generated by tools/genfont.py
    u32 *font_dest = (u32*)CBB_ADDR(DLG_CBB) + (32 * 8);
    for (int i = 0; i < FONT_NUM_TILES * 8; i++) {
        font_dest[i] = font_4bpp[i];
    }

    // Set up palette 15 for text
    u16 *pal = (u16*)(MEM_PAL_BG + 15 * 32);
    pal[0] = 0x0000;              // Transparent
    pal[1] = RGB15(4, 4, 8);     // Dark blue-grey (box fill)
    pal[2] = RGB15(20, 16, 8);   // Gold (box border)
    pal[15] = RGB15(31, 31, 31); // White (font color — we'll use this in font tiles)
}

void text_draw_string(const char *str, int sbb, int x, int y, int max_width) {
    u16 *map = (u16*)SBB_ADDR(sbb);
    int cx = x;
    while (*str && cx < x + max_width) {
        int tile = 32 + (*str - ' ');
        if (*str < ' ' || *str > '~') tile = 32;
        map[y * 32 + cx] = SE_TILE(tile) | SE_PALBANK(15);
        cx++;
        str++;
    }
}

void text_clear_region(int sbb, int x, int y, int w, int h) {
    u16 *map = (u16*)SBB_ADDR(sbb);
    for (int row = y; row < y + h; row++) {
        for (int col = x; col < x + w; col++) {
            map[row * 32 + col] = 0;
        }
    }
}
