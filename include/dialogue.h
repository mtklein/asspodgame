// dialogue.h — Dialogue and text box system
// Supports branching dialogue trees with Fate Point rewards
#ifndef DIALOGUE_H
#define DIALOGUE_H

#include "gba.h"

// A single dialogue node
typedef struct {
    const char *speaker;    // Speaker name (shown in name plate)
    const char *text;       // Dialogue text (auto-wrapped)
    u8 portrait_id;         // Portrait graphic index
    u8 num_choices;         // 0 = press A to continue, 1-3 = choices
    u16 next;               // Next node if no choices (0 = end)
} DialogueNode;

// A choice within a dialogue
typedef struct {
    const char *text;       // Choice text
    u16 next_node;          // Where this choice leads
    s8 fate_reward;         // Fate points gained (funny/in-character choices)
    u8 stat_check;          // 0=none, 1=SHOOT, 2=BRAWN, 3=BRAINS, 4=TALK, 5=COOL
    s8 stat_dc;             // Difficulty to pass the check
} DialogueChoice;

// Dialogue tree = array of nodes + parallel array of choices
typedef struct {
    const DialogueNode *nodes;
    int num_nodes;
    const DialogueChoice *choices;  // Flat array, indexed by node
    const u8 *choice_offsets;       // choices[choice_offsets[node_id]..] for each node
} DialogueTree;

// Dialogue state
typedef enum {
    DLG_INACTIVE = 0,
    DLG_PRINTING,    // Text appearing character by character
    DLG_WAITING,     // Full text shown, waiting for A
    DLG_CHOOSING,    // Player selecting a choice
} DialogueState;

extern DialogueState dlg_state;

// Start a dialogue by tree ID
void dialogue_start(u16 tree_id);

// Update dialogue (call each frame)
void dialogue_update(void);

// Draw dialogue box (call each frame after update)
void dialogue_draw(void);

// Is dialogue currently active?
static inline int dialogue_active(void) {
    return dlg_state != DLG_INACTIVE;
}

// --- Text rendering ---
// Load the font tileset into a BG charblock
void text_init(void);

// Draw a string to a screenblock position
void text_draw_string(const char *str, int sbb, int x, int y, int max_width);

// Clear a rectangular region of a screenblock
void text_clear_region(int sbb, int x, int y, int w, int h);

#endif // DIALOGUE_H
