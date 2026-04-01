// dialogue.h — Dialogue and text box system
// Supports branching dialogue trees with Fate Point rewards
#ifndef DIALOGUE_H
#define DIALOGUE_H

#include "gba.h"

#define MAX_CHOICES 3

// A choice within a dialogue node
typedef struct {
    const char *text;       // Choice text (NULL = unused slot)
    u16 next_node;          // Where this choice leads (0 = end)
    s8 fate_reward;         // Fate points gained (funny/in-character choices)
} DialogueChoice;

// A single dialogue node
typedef struct {
    const char *speaker;    // Speaker name (shown in name plate)
    const char *text;       // Dialogue text (auto-wrapped)
    u8 num_choices;         // 0 = press A to continue, 1-3 = choices
    u16 next;               // Next node if no choices (0xFFFF = end)
    DialogueChoice choices[MAX_CHOICES];
} DialogueNode;

// Dialogue tree = array of nodes
typedef struct {
    const DialogueNode *nodes;
    int num_nodes;
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
void text_init(void);
void text_draw_string(const char *str, int sbb, int x, int y, int max_width);
void text_clear_region(int sbb, int x, int y, int w, int h);

#endif // DIALOGUE_H
