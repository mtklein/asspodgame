// dialogue.c — Dialogue system implementation
#include "dialogue.h"
#include "input.h"
#include "entity.h"
#include "game_data.h"
#include "portrait_data.h"
#include "sprite.h"

// Dialogue box uses BG2 as a text layer
#define DLG_SBB     24   // Screenblock for dialogue text
#define DLG_CBB     2    // Charblock for font tiles

// Text box layout (in tiles)
#define BOX_X       1
#define BOX_Y       15
#define BOX_W       28
#define BOX_H       5
#define TEXT_X      2
#define TEXT_Y      16
#define TEXT_W      26
#define NAME_X      2
#define NAME_Y      15

#define PRINT_SPEED    2   // Frames per character
#define LINES_PER_PAGE 3
#define MAX_PAGES      16

DialogueState dlg_state = DLG_INACTIVE;

static u16 current_tree_id;
static u16 current_node;
static int char_index;
static int print_timer;
static int choice_cursor;
static int current_page;
static int page_char_counts[MAX_PAGES];
static int num_pages;
static int blink_timer;
static const char *current_speaker;
static int portrait_is_pc;  // 1 = right side (PC), 0 = left side (NPC)

static void show_portrait(const char *speaker) {
    current_speaker = speaker;
    const u32 *data = portrait_find(speaker);
    if (!data) {
        sprite_hide(PORTRAIT_OAM_SLOT);
        return;
    }
    // Determine if this is a PC portrait (right side) or NPC (left side)
    portrait_is_pc = 0;
    if (speaker) {
        const char *s = speaker;
        // Check for player character names
        if ((s[0]=='T'&&s[1]=='r'&&s[2]=='e') || (s[0]=='K'&&s[1]=='i'&&s[2]=='p'))
            portrait_is_pc = 1;
    }
    sprite_load_tiles(data, PORTRAIT_TILE_BASE, PORTRAIT_BYTES);
    sprite_set_size(PORTRAIT_OAM_SLOT, ATTR0_SQUARE, ATTR1_SIZE_32);
    sprite_set_tile(PORTRAIT_OAM_SLOT, PORTRAIT_TILE_BASE, 0);
    sprite_show(PORTRAIT_OAM_SLOT);
    // Position: NPC on left (x=4), PC on right (x=204)
    int px = portrait_is_pc ? 204 : 4;
    int py = 120 - 36;  // above the text box
    sprite_set_pos(PORTRAIT_OAM_SLOT, px, py);
}

static void hide_portrait(void) {
    sprite_hide(PORTRAIT_OAM_SLOT);
    current_speaker = 0;
}

static const DialogueTree* get_tree(void) {
    if (current_tree_id < NUM_DIALOGUE_TREES)
        return &dialogue_trees[current_tree_id];
    return 0;
}

static const DialogueNode* get_node(void) {
    const DialogueTree *tree = get_tree();
    if (tree && current_node < (u16)tree->num_nodes)
        return &tree->nodes[current_node];
    return 0;
}

static int word_len(const char *s) {
    int n = 0;
    while (s[n] && s[n] != ' ' && s[n] != '\n') n++;
    return n;
}

static void compute_pages(const char *text) {
    num_pages = 0;
    if (!text || !*text) {
        num_pages = 1;
        page_char_counts[0] = 0;
        return;
    }

    const char *p = text;
    int count = 0;

    while (*p && num_pages < MAX_PAGES) {
        int cx = 0;
        int lines = 0;
        int page_start = count;

        while (*p && lines < LINES_PER_PAGE) {
            if (*p == '\n') {
                p++; count++;
                cx = 0;
                lines++;
                continue;
            }

            if (*p == ' ' && cx > 0) {
                int wl = word_len(p + 1);
                if (cx + 1 + wl > TEXT_W && wl <= TEXT_W) {
                    p++; count++;
                    cx = 0;
                    lines++;
                    continue;
                }
            }

            if (cx >= TEXT_W) {
                cx = 0;
                lines++;
                if (lines >= LINES_PER_PAGE) break;
            }

            p++; count++;
            cx++;
        }

        page_char_counts[num_pages] = count - page_start;
        num_pages++;
    }

    if (num_pages == 0) {
        num_pages = 1;
        page_char_counts[0] = 0;
    }
}

void dialogue_start(u16 tree_id) {
    current_tree_id = tree_id;
    current_node = 0;
    char_index = 0;
    print_timer = 0;
    choice_cursor = 0;
    current_page = 0;
    blink_timer = 0;
    dlg_state = DLG_PRINTING;

    // Enable BG2 for text overlay
    REG_DISPCNT |= DCNT_BG2;
    REG_BG2CNT = BG_PRIO(0) | BG_CBB(DLG_CBB) | BG_SBB(DLG_SBB) | BG_4BPP | BG_SIZE_256x256;
    REG_BG2HOFS = 0;
    REG_BG2VOFS = 0;

    // Clear and draw box
    text_clear_region(DLG_SBB, 0, 0, 32, 20);

    u16 *sbb = (u16*)SBB_ADDR(DLG_SBB);
    for (int x = BOX_X; x < BOX_X + BOX_W; x++) {
        for (int y = BOX_Y; y < BOX_Y + BOX_H; y++) {
            u16 tile = 1; // Filled
            if (y == BOX_Y)
                tile = (x == BOX_X) ? 2 : (x == BOX_X+BOX_W-1) ? 4 : 3;
            else if (y == BOX_Y + BOX_H - 1)
                tile = (x == BOX_X) ? 7 : (x == BOX_X+BOX_W-1) ? 9 : 8;
            else if (x == BOX_X) tile = 5;
            else if (x == BOX_X+BOX_W-1) tile = 6;
            sbb[y * 32 + x] = SE_TILE(tile) | SE_PALBANK(15);
        }
    }

    const DialogueNode *node = get_node();
    if (node) {
        compute_pages(node->text);
        show_portrait(node->speaker);
    }
}

static void reset_for_node(void) {
    char_index = 0;
    print_timer = 0;
    current_page = 0;
    blink_timer = 0;
    const DialogueNode *node = get_node();
    if (node) {
        compute_pages(node->text);
        show_portrait(node->speaker);
    }
}

void dialogue_update(void) {
    if (dlg_state == DLG_INACTIVE) return;

    const DialogueNode *node = get_node();
    if (!node) {
        dlg_state = DLG_INACTIVE;
        REG_DISPCNT &= ~DCNT_BG2;
        hide_portrait();
        return;
    }

    switch ((int)dlg_state) {
    case DLG_PRINTING: {
        int speed = key_held(KEY_B) ? 0 : PRINT_SPEED;
        print_timer++;
        if (print_timer >= speed) {
            print_timer = 0;
            char_index++;
        }
        int page_len = page_char_counts[current_page];
        if (char_index >= page_len) {
            char_index = page_len;
            if (current_page < num_pages - 1)
                dlg_state = DLG_PAGING;
            else
                dlg_state = (node->num_choices > 0) ? DLG_CHOOSING : DLG_WAITING;
        }
        if (key_hit(KEY_A)) {
            char_index = page_len;
            if (current_page < num_pages - 1)
                dlg_state = DLG_PAGING;
            else
                dlg_state = (node->num_choices > 0) ? DLG_CHOOSING : DLG_WAITING;
        }
        break;
    }
    case DLG_PAGING:
        blink_timer++;
        if (key_hit(KEY_A)) {
            current_page++;
            char_index = 0;
            print_timer = 0;
            blink_timer = 0;
            dlg_state = DLG_PRINTING;
        }
        break;

    case DLG_WAITING:
        if (key_hit(KEY_A)) {
            if (node->next == 0xFFFF) {
                dlg_state = DLG_INACTIVE;
                REG_DISPCNT &= ~DCNT_BG2;
            } else {
                current_node = node->next;
                reset_for_node();
                dlg_state = DLG_PRINTING;
            }
        }
        break;

    case DLG_CHOOSING:
        if (key_hit(KEY_UP) && choice_cursor > 0) choice_cursor--;
        if (key_hit(KEY_DOWN) && choice_cursor < node->num_choices - 1) choice_cursor++;

        if (key_hit(KEY_A)) {
            const DialogueChoice *ch = &node->choices[choice_cursor];

            // Award fate points
            if (ch->fate_reward > 0) {
                Entity *player = &entities[active_player];
                player->stats.fate_points += ch->fate_reward;
                if (player->stats.fate_points > player->stats.fate_max)
                    player->stats.fate_points = player->stats.fate_max;
            }

            if (ch->next_node == 0xFFFF || ch->next_node == 0) {
                dlg_state = DLG_INACTIVE;
                REG_DISPCNT &= ~DCNT_BG2;
            } else {
                current_node = ch->next_node;
                choice_cursor = 0;
                reset_for_node();
                dlg_state = DLG_PRINTING;
            }
        }
        break;

    default:
        break;
    }
}

static const char* skip_pages(const char *text, int pages) {
    const char *p = text;
    for (int pg = 0; pg < pages; pg++) {
        int cx = 0;
        int chars = page_char_counts[pg];
        int count = 0;

        while (*p && count < chars) {
            if (*p == '\n') {
                p++; count++;
                cx = 0;
                continue;
            }
            if (*p == ' ' && cx > 0) {
                int wl = word_len(p + 1);
                if (cx + 1 + wl > TEXT_W && wl <= TEXT_W) {
                    p++; count++;
                    cx = 0;
                    continue;
                }
            }
            if (cx >= TEXT_W) {
                cx = 0;
            }
            p++; count++; cx++;
        }
    }
    return p;
}

void dialogue_draw(void) {
    if (dlg_state == DLG_INACTIVE) return;

    const DialogueNode *node = get_node();
    if (!node) return;

    text_clear_region(DLG_SBB, TEXT_X, TEXT_Y, TEXT_W, LINES_PER_PAGE);
    text_clear_region(DLG_SBB, NAME_X, NAME_Y, 12, 1);

    if (node->speaker)
        text_draw_string(node->speaker, DLG_SBB, NAME_X, NAME_Y, 12);

    if (node->text && dlg_state != DLG_CHOOSING) {
        u16 *sbb = (u16*)SBB_ADDR(DLG_SBB);
        const char *p = skip_pages(node->text, current_page);
        int cx = 0, cy = 0, count = 0;

        while (*p && count < char_index) {
            if (*p == '\n') {
                p++; count++;
                cx = 0; cy++;
                continue;
            }
            if (*p == ' ' && cx > 0) {
                int wl = word_len(p + 1);
                if (cx + 1 + wl > TEXT_W && wl <= TEXT_W) {
                    p++; count++;
                    cx = 0; cy++;
                    continue;
                }
            }
            if (cx >= TEXT_W) {
                cx = 0; cy++;
            }
            if (cy >= LINES_PER_PAGE) break;

            int tile_idx = 32 + (*p - ' ');
            if (*p < ' ' || *p > '~') tile_idx = 32;
            sbb[(TEXT_Y + cy) * 32 + (TEXT_X + cx)] = SE_TILE(tile_idx) | SE_PALBANK(15);
            cx++; p++; count++;
        }

        if (dlg_state == DLG_PAGING && (blink_timer / 16) % 2 == 0) {
            int vx = TEXT_X + TEXT_W - 1;
            int vy = TEXT_Y + LINES_PER_PAGE - 1;
            sbb[vy * 32 + vx] = SE_TILE(32 + ('v' - ' ')) | SE_PALBANK(15);
        }
    }

    if (dlg_state == DLG_CHOOSING && node->num_choices > 0) {
        u16 *sbb = (u16*)SBB_ADDR(DLG_SBB);
        for (int i = 0; i < node->num_choices && i < MAX_CHOICES; i++) {
            int cy = TEXT_Y + i;
            sbb[cy * 32 + TEXT_X] =
                SE_TILE(i == choice_cursor ? (32 + '>'-' ') : 32) | SE_PALBANK(15);
            if (node->choices[i].text)
                text_draw_string(node->choices[i].text, DLG_SBB, TEXT_X + 1, cy, TEXT_W - 1);
        }
    }
}

// --- Text rendering ---
#include "font_data.h"

void text_init(void) {
    u32 *dest = (u32*)CBB_ADDR(DLG_CBB);

    // Tile 0: transparent
    for (int i = 0; i < 8; i++) dest[i] = 0;

    // Tiles 1-9: FF-style blue gradient box with thin white border
    // Colors: 1=dark blue, 2=white border, 3=mid blue, 4=lighter blue
    // Tile 1: gradient fill (dark blue top to mid blue bottom)
    dest[8]=0x11111111; dest[9]=0x11111111; dest[10]=0x31111111; dest[11]=0x31313131;
    dest[12]=0x33333333; dest[13]=0x33333333; dest[14]=0x43333333; dest[15]=0x44444444;
    // Tile 2: top-left corner (white border + gradient)
    dest[16]=0x22222222; dest[17]=0x21111111; dest[18]=0x21111111; dest[19]=0x21111111;
    dest[20]=0x21313131; dest[21]=0x23333333; dest[22]=0x23333333; dest[23]=0x24333333;
    // Tile 3: top edge
    dest[24]=0x22222222; dest[25]=0x11111111; dest[26]=0x11111111; dest[27]=0x11111111;
    dest[28]=0x31313131; dest[29]=0x33333333; dest[30]=0x33333333; dest[31]=0x43333333;
    // Tile 4: top-right corner
    dest[32]=0x22222222; dest[33]=0x11111112; dest[34]=0x11111112; dest[35]=0x11111112;
    dest[36]=0x31313132; dest[37]=0x33333332; dest[38]=0x33333332; dest[39]=0x43333332;
    // Tile 5: left edge (gradient continues)
    dest[40]=0x24444444; dest[41]=0x24444444; dest[42]=0x24444444; dest[43]=0x24444444;
    dest[44]=0x24444444; dest[45]=0x24444444; dest[46]=0x24444444; dest[47]=0x24444444;
    // Tile 6: right edge
    dest[48]=0x44444442; dest[49]=0x44444442; dest[50]=0x44444442; dest[51]=0x44444442;
    dest[52]=0x44444442; dest[53]=0x44444442; dest[54]=0x44444442; dest[55]=0x44444442;
    // Tile 7: bottom-left
    dest[56]=0x24444444; dest[57]=0x24444444; dest[58]=0x24444444; dest[59]=0x24444444;
    dest[60]=0x24444444; dest[61]=0x24444444; dest[62]=0x24444444; dest[63]=0x22222222;
    // Tile 8: bottom edge
    dest[64]=0x44444444; dest[65]=0x44444444; dest[66]=0x44444444; dest[67]=0x44444444;
    dest[68]=0x44444444; dest[69]=0x44444444; dest[70]=0x44444444; dest[71]=0x22222222;
    // Tile 9: bottom-right
    dest[72]=0x44444442; dest[73]=0x44444442; dest[74]=0x44444442; dest[75]=0x44444442;
    dest[76]=0x44444442; dest[77]=0x44444442; dest[78]=0x44444442; dest[79]=0x22222222;

    // Load ASCII font (tiles 32..126 in charblock)
    u32 *font_dest = dest + (32 * 8);
    for (int i = 0; i < FONT_NUM_TILES * 8; i++)
        font_dest[i] = font_4bpp[i];

    // Palette 15 for text/UI — FF-style blue gradient
    u16 *pal = (u16*)(MEM_PAL_BG + 15 * 32);
    pal[0]  = 0x0000;              // Transparent
    pal[1]  = RGB15(1, 1, 12);    // Dark blue (top of gradient)
    pal[2]  = RGB15(24, 24, 28);  // White-ish (thin border)
    pal[3]  = RGB15(3, 3, 16);    // Mid blue
    pal[4]  = RGB15(5, 5, 20);    // Lighter blue (bottom of gradient)
    pal[15] = RGB15(31, 31, 31);  // White (font)
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
    for (int row = y; row < y + h; row++)
        for (int col = x; col < x + w; col++)
            map[row * 32 + col] = 0;
}
