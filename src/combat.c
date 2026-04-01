// combat.c — Turn-based combat implementation
#include "combat.h"
#include "input.h"
#include "dialogue.h"
#include "sprite.h"

CombatState combat_state = COMBAT_INACTIVE;
CombatEnemy combat_enemies[MAX_COMBAT_ENEMIES];
int num_combat_enemies = 0;

static int menu_cursor = 0;
static int target_cursor = 0;
static CombatAction selected_action;
static int anim_timer = 0;
static int result_margin = 0;
static const char *result_text = "";

// Simple LCG random (seeded from timer)
static u32 rng_state = 12345;
static u32 rng_next(void) {
    rng_state = rng_state * 1103515245 + 12345;
    return (rng_state >> 16) & 0x7FFF;
}

int fate_roll(int stat, int dc) {
    // Fate-style: roll 4dF (each is -1, 0, or +1) + stat
    // Simplified: random -4 to +4, plus stat, minus DC
    int roll = 0;
    for (int i = 0; i < 4; i++) {
        int die = (int)(rng_next() % 3) - 1; // -1, 0, +1
        roll += die;
    }
    return stat + roll - dc;
}

void combat_start(const CombatEnemy *enemies, int count) {
    if (count > MAX_COMBAT_ENEMIES) count = MAX_COMBAT_ENEMIES;
    num_combat_enemies = count;
    for (int i = 0; i < count; i++) {
        combat_enemies[i] = enemies[i];
        combat_enemies[i].alive = 1;
    }
    combat_state = COMBAT_START;
    menu_cursor = 0;
    anim_timer = 0;

    // Seed RNG from hardware timer
    rng_state = REG_TM0CNT_L | ((u32)REG_VCOUNT << 16);
}

void combat_update(void) {
    if (combat_state == COMBAT_INACTIVE) return;

    Entity *player = &entities[active_player];

    switch (combat_state) {
    case COMBAT_START:
        anim_timer++;
        if (anim_timer > 30) { // Brief "encounter!" flash
            combat_state = COMBAT_PLAYER_TURN;
            anim_timer = 0;
            menu_cursor = 0;
        }
        break;

    case COMBAT_PLAYER_TURN:
        // Navigate action menu
        if (key_hit(KEY_UP) && menu_cursor > 0) menu_cursor--;
        if (key_hit(KEY_DOWN) && menu_cursor < 4) menu_cursor++;

        if (key_hit(KEY_A)) {
            selected_action = (CombatAction)menu_cursor;

            // Find first alive enemy
            target_cursor = -1;
            for (int i = 0; i < num_combat_enemies; i++) {
                if (combat_enemies[i].alive) { target_cursor = i; break; }
            }

            if (target_cursor >= 0) {
                // Execute action
                int stat = 0;
                int dc = 2; // Base difficulty
                CombatEnemy *target = &combat_enemies[target_cursor];

                switch (selected_action) {
                case ACT_SHOOT:
                    stat = player->stats.shoot;
                    dc += target->stats.cool; // Enemy dodges with COOL
                    break;
                case ACT_PUNCH:
                    stat = player->stats.brawn;
                    dc += target->stats.brawn;
                    break;
                case ACT_TALK:
                    stat = player->stats.talk;
                    dc += target->stats.cool + 1; // Harder to talk down
                    break;
                case ACT_FATE:
                    if (player->stats.fate_points > 0) {
                        player->stats.fate_points--;
                        stat = player->stats.shoot + 2; // Big bonus
                        dc += target->stats.cool;
                    } else {
                        result_text = "No Fate Points!";
                        combat_state = COMBAT_RESULT;
                        anim_timer = 0;
                        break;
                    }
                    break;
                default:
                    break;
                }

                result_margin = fate_roll(stat, dc);

                if (selected_action == ACT_TALK && result_margin >= 0) {
                    // Talked them down — instant KO with style
                    target->stats.hp = 0;
                    target->alive = 0;
                    result_text = "Talked 'em down!";
                } else if (result_margin >= 3) {
                    // Critical success
                    int dmg = result_margin + 1;
                    target->stats.hp -= dmg;
                    if (target->stats.hp <= 0) {
                        target->alive = 0;
                        result_text = "Critical! KO!";
                    } else {
                        result_text = "Critical hit!";
                    }
                } else if (result_margin >= 0) {
                    // Success
                    int dmg = result_margin + 1;
                    target->stats.hp -= dmg;
                    if (target->stats.hp <= 0) {
                        target->alive = 0;
                        result_text = "Got 'em!";
                    } else {
                        result_text = "Hit!";
                    }
                } else {
                    // Failure
                    result_text = "Whiffed it!";
                    // Fate point consolation
                    if (result_margin <= -3 && player->stats.fate_points < player->stats.fate_max) {
                        player->stats.fate_points++;
                        result_text = "Epic fail! +1 FP";
                    }
                }

                combat_state = COMBAT_ANIMATE;
                anim_timer = 0;
            }
        }
        break;

    case COMBAT_ANIMATE:
        anim_timer++;
        if (anim_timer > 20) {
            combat_state = COMBAT_RESULT;
            anim_timer = 0;
        }
        break;

    case COMBAT_RESULT:
        anim_timer++;
        if (anim_timer > 40 || key_hit(KEY_A)) {
            // Check if all enemies down
            int all_dead = 1;
            for (int i = 0; i < num_combat_enemies; i++) {
                if (combat_enemies[i].alive) { all_dead = 0; break; }
            }

            if (all_dead) {
                combat_state = COMBAT_VICTORY;
                anim_timer = 0;
                // Reward fate points
                player->stats.fate_points++;
                if (player->stats.fate_points > player->stats.fate_max)
                    player->stats.fate_points = player->stats.fate_max;
            } else {
                // Enemy turn
                combat_state = COMBAT_ENEMY_TURN;
                anim_timer = 0;
            }
        }
        break;

    case COMBAT_ENEMY_TURN:
        anim_timer++;
        if (anim_timer == 15) {
            // Each alive enemy attacks
            for (int i = 0; i < num_combat_enemies; i++) {
                if (!combat_enemies[i].alive) continue;

                int margin = fate_roll(combat_enemies[i].stats.shoot, player->stats.cool + 2);
                if (margin >= 0) {
                    player->stats.hp -= (margin + 1);
                    result_text = "Ouch!";
                } else {
                    result_text = "Dodged!";
                }
            }
        }
        if (anim_timer > 30) {
            if (player->stats.hp <= 0) {
                combat_state = COMBAT_DEFEAT;
                anim_timer = 0;
            } else {
                combat_state = COMBAT_PLAYER_TURN;
                menu_cursor = 0;
                anim_timer = 0;
            }
        }
        break;

    case COMBAT_VICTORY:
        if (key_hit(KEY_A)) {
            combat_state = COMBAT_INACTIVE;
        }
        break;

    case COMBAT_DEFEAT:
        if (key_hit(KEY_A)) {
            // Restore HP (Fate RPG doesn't do permadeath, just consequences)
            player->stats.hp = player->stats.hp_max / 2;
            combat_state = COMBAT_INACTIVE;
        }
        break;

    default:
        break;
    }
}

void combat_draw(void) {
    if (combat_state == COMBAT_INACTIVE) return;

    // Draw combat UI on BG2 (same layer as dialogue)
    REG_DISPCNT |= DCNT_BG2;
    REG_BG2CNT = BG_PRIO(0) | BG_CBB(2) | BG_SBB(24) | BG_4BPP | BG_SIZE_256x256;

    u16 *sbb = (u16*)SBB_ADDR(24);

    // Clear screen
    text_clear_region(24, 0, 0, 30, 20);

    Entity *player = &entities[active_player];

    // Draw player stats (top left)
    const char *pname = (player->type == ENT_PLAYER_TREVOR) ? "STEEL" : "SIMPKINS";
    text_draw_string(pname, 24, 1, 1, 10);

    // HP bar
    text_draw_string("HP:", 24, 1, 2, 3);
    for (int i = 0; i < player->stats.hp_max && i < 10; i++) {
        int tile = (i < player->stats.hp) ? (32 + '#' - ' ') : (32 + '.' - ' ');
        sbb[2 * 32 + 4 + i] = SE_TILE(tile) | SE_PALBANK(15);
    }

    // Fate Points
    text_draw_string("FP:", 24, 1, 3, 3);
    for (int i = 0; i < player->stats.fate_max && i < 5; i++) {
        int tile = (i < player->stats.fate_points) ? (32 + '*' - ' ') : (32 + '.' - ' ');
        sbb[3 * 32 + 4 + i] = SE_TILE(tile) | SE_PALBANK(15);
    }

    // Draw enemies (top right)
    for (int i = 0; i < num_combat_enemies; i++) {
        int y = 1 + i * 2;
        if (combat_enemies[i].alive) {
            text_draw_string(combat_enemies[i].name, 24, 18, y, 12);
            // HP
            text_draw_string("HP:", 24, 18, y + 1, 3);
            for (int j = 0; j < combat_enemies[i].stats.hp_max && j < 8; j++) {
                int tile = (j < combat_enemies[i].stats.hp) ? (32 + '#' - ' ') : (32 + '.' - ' ');
                sbb[(y+1) * 32 + 21 + j] = SE_TILE(tile) | SE_PALBANK(15);
            }
        } else {
            text_draw_string(combat_enemies[i].name, 24, 18, y, 12);
            text_draw_string("[DOWN]", 24, 18, y + 1, 8);
        }
    }

    // Draw based on state
    switch (combat_state) {
    case COMBAT_START:
        text_draw_string("!! ENCOUNTER !!", 24, 8, 9, 15);
        break;

    case COMBAT_PLAYER_TURN: {
        // Action menu
        const char *actions[] = {"SHOOT", "PUNCH", "TALK", "ITEM", "INVOKE FATE"};
        for (int i = 0; i < 5; i++) {
            // Cursor
            if (i == menu_cursor) {
                sbb[(12 + i) * 32 + 2] = SE_TILE(32 + '>' - ' ') | SE_PALBANK(15);
            }
            text_draw_string(actions[i], 24, 3, 12 + i, 12);
        }
        break;
    }

    case COMBAT_ANIMATE:
        text_draw_string("...", 24, 14, 10, 3);
        break;

    case COMBAT_RESULT:
    case COMBAT_ENEMY_TURN:
        if (result_text) {
            text_draw_string(result_text, 24, 10, 10, 20);
        }
        break;

    case COMBAT_VICTORY:
        text_draw_string("CASE CLOSED!", 24, 9, 9, 14);
        text_draw_string("+1 Fate Point", 24, 9, 11, 14);
        text_draw_string("Press A", 24, 11, 14, 8);
        break;

    case COMBAT_DEFEAT:
        text_draw_string("BUSTED...", 24, 10, 9, 12);
        text_draw_string("Steel and Simpkins", 24, 6, 11, 20);
        text_draw_string("regroup at HQ.", 24, 8, 12, 16);
        text_draw_string("Press A", 24, 11, 14, 8);
        break;

    default:
        break;
    }
}
