# Spirit of Half-Life 1.2 — Comprehensive LRC Modification Guide

## Introduction

This document is a comprehensive reimplementation guide for all **Spirit of Half-Life 1.2** (SoHL) modifications made by **Laurie R. Cheers (Laurascence)**, identified by `//LRC` comments throughout the codebase. These modifications extend the vanilla Half-Life SDK with new entity systems, enhanced triggers, visual effects, and gameplay features.

When porting SoHL features to a newer vanilla Half-Life SDK, use this guide to identify every change that must be reimplemented. Each entry lists the source file, line number, the actual code, and a description of what it does. All **795 LRC-commented lines** across **96 unique source files** are catalogued below.

---

## Table of Contents

- [1. MoveWith System](#1-movewith-system)
- [2. Think/NextThink System](#2-thinknextthink-system)
- [3. Trigger System Enhancements](#3-trigger-system-enhancements)
- [4. State System](#4-state-system)
- [5. Fog System](#5-fog-system)
- [6. Sky System](#6-sky-system)
- [7. Custom HUD Color](#7-custom-hud-color)
- [8. Shiny/Reflective Surfaces](#8-shinyreflective-surfaces)
- [9. Dynamic Lighting](#9-dynamic-lighting)
- [10. Monster/NPC Enhancements](#10-monsternpc-enhancements)
- [11. Func_Tank Enhancements](#11-func_tank-enhancements)
- [12. Platform/Train Enhancements](#12-platformtrain-enhancements)
- [13. Door Enhancements](#13-door-enhancements)
- [14. Button Enhancements](#14-button-enhancements)
- [15. Breakable Enhancements](#15-breakable-enhancements)
- [16. Light Enhancements](#16-light-enhancements)
- [17. Sound Enhancements](#17-sound-enhancements)
- [18. Effects Enhancements](#18-effects-enhancements)
- [19. Model/Animation](#19-modelanimation)
- [20. Particle System](#20-particle-system)
- [21. Scripted Sequence Enhancements](#21-scripted-sequence-enhancements)
- [22. New Entity Definitions](#22-new-entity-definitions)
- [23. Other/Misc Changes](#23-othermiscellaneous-changes)
- [24. New Files](#24-new-files)
- [25. Summary Statistics](#25-summary-statistics)

---

## 1. MoveWith System

The MoveWith system is the largest SoHL addition. It allows any entity to be "parented" to another, moving and rotating with it. This requires a linked-list structure of parent/child/sibling relationships, an "assist list" for coordinating physics updates, and extensive changes to entity base classes.

### 1.1 Core Data Structures (dlls/cbase.h)

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/cbase.h` | 196 | `CBaseEntity* m_pMoveWith;` | Pointer to the entity this one moves with |
| `dlls/cbase.h` | 197 | `int m_MoveWith;` | Targetname string of the MoveWith entity |
| `dlls/cbase.h` | 198 | `CBaseEntity* m_pChildMoveWith;` | One of the entities moving with this one |
| `dlls/cbase.h` | 199 | `CBaseEntity* m_pSiblingMoveWith;` | Linked list: another entity moving with the same parent |
| `dlls/cbase.h` | 200 | `Vector m_vecMoveWithOffset;` | Position offset relative to parent's origin |
| `dlls/cbase.h` | 201 | `Vector m_vecRotWithOffset;` | Angle offset relative to parent's angles |
| `dlls/cbase.h` | 202 | `CBaseEntity* m_pAssistLink;` | Link to next entity in the AssistList for physics coordination |
| `dlls/cbase.h` | 203 | `Vector m_vecPostAssistVel;` | Velocity to apply after assist processing |
| `dlls/cbase.h` | 204 | `Vector m_vecPostAssistAVel;` | Angular velocity to apply after assist processing |
| `dlls/cbase.h` | 209 | `int m_iLFlags;` | New flag set (pev->spawnflags and pev->flags are full) |
| `dlls/cbase.h` | 212 | `virtual void DesiredAction(void) {};` | For postponing work until PostThink time |
| `dlls/cbase.h` | 215 | `Vector m_vecSpawnOffset;` | Fix things that MoveWith a door which Starts Open |
| `dlls/cbase.h` | 262 | `// MoveWith for all!` | KeyValue handler for "movewith" key on all entities |
| `dlls/cbase.h` | 283 | `// if I MoveWith something, then only cross transitions if the MoveWith entity does too.` | Transition logic tied to MoveWith parent |
| `dlls/cbase.h` | 289 | `virtual void Activate(void);` | Overridden Activate to initialize MoveWith |
| `dlls/cbase.h` | 290 | `void InitMoveWith(void);` | Called by Activate() to set up MoveWith values |
| `dlls/cbase.h` | 293 | `virtual void PostSpawn(void) {}` | Called by Activate() for entity-specific init |
| `dlls/cbase.h` | 99 | `//extern CBaseEntity *g_pDesiredList;` | Global DesiredVel list reference (commented out) |
| `dlls/cbase.h` | 554 | `// moved here from player.cpp` | UTIL_SetOrigin declaration moved for wider access |

### 1.2 Save/Restore Fields (dlls/cbase.cpp)

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/cbase.cpp` | 784 | `DEFINE_FIELD(CBaseEntity, m_MoveWith, FIELD_STRING)` | Save/restore MoveWith targetname |
| `dlls/cbase.cpp` | 785 | `DEFINE_FIELD(CBaseEntity, m_pMoveWith, FIELD_CLASSPTR)` | Save/restore parent pointer |
| `dlls/cbase.cpp` | 786 | `DEFINE_FIELD(CBaseEntity, m_pChildMoveWith, FIELD_CLASSPTR)` | Save/restore child pointer |
| `dlls/cbase.cpp` | 787 | `DEFINE_FIELD(CBaseEntity, m_pSiblingMoveWith, FIELD_CLASSPTR)` | Save/restore sibling pointer |
| `dlls/cbase.cpp` | 789 | `DEFINE_FIELD(CBaseEntity, m_iLFlags, FIELD_INTEGER)` | Save/restore LRC flags |
| `dlls/cbase.cpp` | 791 | `DEFINE_FIELD(CBaseEntity, m_vecMoveWithOffset, FIELD_VECTOR)` | Save/restore position offset |
| `dlls/cbase.cpp` | 792 | `DEFINE_FIELD(CBaseEntity, m_vecRotWithOffset, FIELD_VECTOR)` | Save/restore rotation offset |
| `dlls/cbase.cpp` | 793 | `DEFINE_FIELD(CBaseEntity, m_activated, FIELD_BOOLEAN)` | Save/restore activation state |
| `dlls/cbase.cpp` | 796 | `// don't save m_pAssistLink, rebuild on restore` | AssistList is rebuilt, not saved |
| `dlls/cbase.cpp` | 797 | `DEFINE_FIELD(CBaseEntity, m_vecPostAssistVel, FIELD_VECTOR)` | Save/restore post-assist velocity |
| `dlls/cbase.cpp` | 798 | `DEFINE_FIELD(CBaseEntity, m_vecPostAssistAVel, FIELD_VECTOR)` | Save/restore post-assist angular velocity |
| `dlls/cbase.cpp` | 510 | `// rebuild the new assistlist as the game starts` | AssistList initialization on game start |
| `dlls/cbase.cpp` | 516 | `// and the aliaslist too` | AliasLis initialization on game start |
| `dlls/cbase.cpp` | 528 | `// called by activate() to support movewith` | InitMoveWith implementation |

### 1.3 MoveWith Core Logic (dlls/movewith.cpp)

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/movewith.cpp` | 8 | `CWorld* g_pWorld = NULL;` | Global world entity pointer for assist list head |
| `dlls/movewith.cpp` | 10 | `BOOL g_doingDesired = FALSE;` | Marks whether Desired functions are being processed |
| `dlls/movewith.cpp` | 423 | `// change the origin to the given position, and bring any movewiths along too` | UTIL_AssignOrigin that propagates to children |
| `dlls/movewith.cpp` | 429 | `// bInitiator is true if called directly` | Differentiates direct calls from recursive MoveWith calls |
| `dlls/movewith.cpp` | 528 | `#define MAX_MOVEWITH_DEPTH` | Arbitrary limit to detect infinite loops in MoveWith chains |
| `dlls/movewith.cpp` | 531 | `// Tell the entity that whatever it's moving with is about to change velocity` | Pre-velocity-change notification for MoveWith children |
| `dlls/movewith.cpp` | 567 | `// velocity is ignored on entities that aren't thinking!` | Workaround for engine ignoring velocity on non-thinking ents |
| `dlls/movewith.cpp` | 601 | `// UTIL_SetVelocity implementation` | Sets velocity and propagates to MoveWith children |
| `dlls/movewith.cpp` | 619 | `int sloopbreaker = MAX_MOVEWITH_DEPTH;` | Loop protection in velocity propagation |
| `dlls/movewith.cpp` | 636 | `// one more MoveWith utility for simple RotWith` | Simplified rotation propagation utility |
| `dlls/movewith.cpp` | 689 | `int sloopbreaker = MAX_MOVEWITH_DEPTH;` | Loop protection in rotation propagation |

### 1.4 MoveWith Header Flags (dlls/movewith.h)

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/movewith.h` | all | `LF_NOTEASY, LF_NOTMEDIUM, LF_NOTHARD` | Difficulty flags stored in m_iLFlags |
| `dlls/movewith.h` | all | `LF_POSTASSISTVEL, LF_POSTASSISTAVEL` | Flags indicating post-assist velocity needs applying |
| `dlls/movewith.h` | all | `LF_DOASSIST, LF_CORRECTSPEED` | Flags for assist list membership and speed correction |
| `dlls/movewith.h` | all | `LF_DODESIRED, LF_DESIRED_THINK, LF_DESIRED_POSTASSIST` | Flags for deferred think/action processing |
| `dlls/movewith.h` | all | `LF_DESIRED_INFO, LF_DESIRED_ACTION` | Additional deferred processing flags |
| `dlls/movewith.h` | all | `LF_ALIASLIST` | Flag for alias list membership |
| `dlls/movewith.h` | all | `CheckDesiredList(), CheckAssistList()` | Global functions to process deferred lists each frame |
| `dlls/movewith.h` | all | `UTIL_DesiredAction(), UTIL_DesiredThink()` | Schedule deferred actions/thinks for an entity |

### 1.5 MoveWith in Other Files

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/client.cpp` | 829 | `// moved to here from CBasePlayer::PostThink` | AssistList processing moved to client frame |
| `dlls/client.cpp` | 865 | `CheckAssistList();` | Process all entities in the assist list each frame |
| `dlls/subs.cpp` | 106 | `// remove this from the AssistList` | Clean up assist list on entity removal |
| `dlls/subs.cpp` | 142 | `// do the same thing if another entity is moving with _me_` | Propagate removal to MoveWith children |
| `dlls/bmodels.cpp` | 79 | `if (!m_pMoveWith)` | Conditional logic based on MoveWith state |
| `dlls/bmodels.cpp` | 82 | `// MoveWith check` | MoveWith integration in brush models |
| `dlls/bmodels.cpp` | 46 | `return (pevBModel->absmin + pevBModel->absmax) * 0.5;` | Bug fix for rotating entity center calculation |
| `dlls/maprules.cpp` | 608 | `if (pev->origin == g_vecZero)` | MoveWith support for game_player_equip |
| `dlls/triggers.cpp` | 176 | `UTIL_DesiredAction(this);` | Defer trigger think until player spawns |
| `dlls/subs.cpp` | 848 | `// info_movewith, the first entity I've made` | info_movewith entity definition |
| `dlls/world.cpp` | 485 | `// set up the world lists` | Initialize global MoveWith/Assist/Alias lists |
| `dlls/world.cpp` | 36 | `#include "movewith.h"` | Include MoveWith header in world |
| `dlls/cbase.h` | 998 | `// much as I hate to add new globals...` | CWorld pointer for reading world entity data |


---

## 2. Think/NextThink System

SoHL replaces the vanilla `pev->nextthink` mechanism with a wrapper system that tracks think times independently. This is necessary because the engine sometimes modifies `pev->nextthink` without notice, and the MoveWith system needs precise control over think timing.

### 2.1 Core Declarations (dlls/cbase.h)

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/cbase.h` | 206 | `float m_fNextThink;` | Marks when a think will be performed (may differ from pev->nextthink) |
| `dlls/cbase.h` | 208 | `float m_fPevNextThink;` | Always set equal to pev->nextthink to detect engine changes |
| `dlls/cbase.h` | 219 | `virtual void SetNextThink(float delay)` | Wrapper calling SetNextThink(delay, FALSE) |
| `dlls/cbase.h` | 219 | `virtual void SetNextThink(float delay, BOOL correctSpeed)` | Set relative think delay with optional speed correction for MoveWith |
| `dlls/cbase.h` | 219 | `virtual void AbsoluteNextThink(float time)` | Set absolute think time |
| `dlls/cbase.h` | 219 | `virtual void AbsoluteNextThink(float time, BOOL correctSpeed)` | Absolute think with speed correction |
| `dlls/cbase.h` | 219 | `void SetEternalThink()` | Think forever (continuous thinking) |
| `dlls/cbase.h` | 241 | `void DontThink(void);` | Replace `SetThink(NULL)` or `pev->nextthink = -1` |
| `dlls/cbase.h` | 243 | `void ResetThink(void);` | Called by parent when a think needs to be aborted |
| `dlls/cbase.h` | 216 | `BOOL m_activated;` | Signifies entity has been activated (moved from func_train) |

### 2.2 Think Implementation (dlls/cbase.cpp)

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/cbase.cpp` | 275 | `// rearranged so that we can correct m_fNextThink too` | Think dispatch rearranged for MoveWith compatibility |
| `dlls/cbase.cpp` | 586 | `// SetNextThink implementation` | Core SetNextThink logic |
| `dlls/cbase.cpp` | 599 | `// AbsoluteNextThink implementation` | Core AbsoluteNextThink logic |
| `dlls/cbase.cpp` | 618 | `// for getting round the engine's preconceptions` | LTIME helper - entity local time |
| `dlls/cbase.cpp` | 654 | `// DontThink implementation` | Cancels all pending thinks |
| `dlls/cbase.cpp` | 672 | `// check in case the engine has changed our nextthink` | ThinkCorrection - detects engine overrides of nextthink |
| `dlls/cbase.cpp` | 794 | `DEFINE_FIELD(CBaseEntity, m_fNextThink, FIELD_TIME)` | Save/restore think time |
| `dlls/cbase.cpp` | 795 | `DEFINE_FIELD(CBaseEntity, m_fPevNextThink, FIELD_TIME)` | Save/restore engine think time |
| `dlls/cbase.cpp` | 809 | `ThinkCorrection();` | Called on restore to fix think times |
| `dlls/cbase.cpp` | 507 | `// Activate implementation` | Game start activation with think correction |

### 2.3 Think in Weapons (dlls/weapons.h)

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/weapons.h` | 310 | `virtual void SetNextThink(float delay);` | Weapons override of SetNextThink |

### 2.4 Think in Subs (dlls/subs.cpp)

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/subs.cpp` | 664 | `DontThink();` | Use DontThink() in LinearMoveDone |
| `dlls/subs.cpp` | 677 | `// mapping toggle-states to global states` | State mapping for toggle entities |


---

## 3. Trigger System Enhancements

SoHL extensively enhances the trigger system with new USE types, new trigger entities, enhanced multi_manager, and locus-based targeting.

### 3.1 New USE Types (dlls/cbase.h)

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/cbase.h` | 101 | `USE_KILL = 4, USE_SAME = 5, USE_NOT = 6` | New use types: USE_KILL removes entity, USE_SAME preserves previous state, USE_NOT inverts |

### 3.2 SUB_UseTargets Enhancements (dlls/subs.cpp)

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/subs.cpp` | 278 | `// allow changing of usetype` | USE_SAME/USE_NOT/USE_KILL processing in SUB_UseTargets |
| `dlls/subs.cpp` | 352 | `// Firing has finished, aliases can now reflect new values` | Flush aliases after firing |
| `dlls/subs.cpp` | 399 | `// Valve had a hacked thing here` | Improved target handling |
| `dlls/subs.cpp` | 416 | `// now just USE_KILLs its target, for consistency` | Target uses USE_KILL instead of UTIL_Remove |
| `dlls/subs.cpp` | 470 | `// now using m_hActivator` | Activator handle tracking |
| `dlls/subs.cpp` | 214 | `DEFINE_FIELD(CBaseDelay, m_hActivator, FIELD_EHANDLE)` | Save/restore activator handle |
| `dlls/cbase.h` | 641 | `EHANDLE m_hActivator;` | Activator handle moved from CBaseToggle to CBaseDelay |

### 3.3 Multi_Manager Enhancements (dlls/triggers.cpp)

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/triggers.cpp` | 502 | `float m_flWait;` | Minimum wait time before firing |
| `dlls/triggers.cpp` | 503 | `float m_flMaxWait;` | Maximum random wait time |
| `dlls/triggers.cpp` | 504 | `string_t m_sMaster;` | Master entity for multi_manager |
| `dlls/triggers.cpp` | 505 | `int m_iMode;` | Mode: 0=timed, 1=pick random, 2=each random |
| `dlls/triggers.cpp` | 506 | `int m_iszThreadName;` | Thread name for cloned managers |
| `dlls/triggers.cpp` | 507 | `int m_iszLocusThread;` | Locus thread name |
| `dlls/triggers.cpp` | 511 | `USE_TYPE m_triggerType;` | Trigger type override |
| `dlls/triggers.cpp` | 532-543 | `DEFINE_FIELD(CMultiManager, ...)` | Save/restore for all new fields |
| `dlls/triggers.cpp` | 555 | `// support Delay, Wait and Master` | Extended KeyValue parsing |
| `dlls/triggers.cpp` | 568-595 | `KeyValue for master, threadname, mode, triggerstate` | New key-value handlers |
| `dlls/triggers.cpp` | 619 | `// keep a count of how many targets` | Target counting for error messages |
| `dlls/triggers.cpp` | 635 | `// additional manager init` | Additional manager initialization |
| `dlls/triggers.cpp` | 698 | `// different manager modes` | Mode-based firing logic |
| `dlls/triggers.cpp` | 706-710 | `Random/constant/immediate wait` | Wait time processing |
| `dlls/triggers.cpp` | 818-828 | `Wait handling in clone` | Wait time in cloned managers |
| `dlls/triggers.cpp` | 840 | `m_iState = STATE_OFF;` | State tracking for finished managers |
| `dlls/triggers.cpp` | 861 | `m_iState = STATE_ON;` | State tracking for active managers |
| `dlls/triggers.cpp` | 899-903 | `Thread name, trigger type, mode, wait copying` | Clone initialization |
| `dlls/triggers.cpp` | 959 | `// "master" support` | Master entity checking |
| `dlls/triggers.cpp` | 985-995 | `Random/constant/immediate wait in Think` | Think-based wait processing |
| `dlls/triggers.cpp` | 1010 | `SF_MULTIMAN_SAMETRIG` | Same-trigger flag support |

### 3.4 New Trigger Entities

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/triggers.cpp` | 1045 | `// multi_watcher entity` | multi_watcher: watches multiple entity states |
| `dlls/triggers.cpp` | 1389 | `// trigger_command` | trigger_command: executes server console commands |
| `dlls/triggers.cpp` | 1394 | `// RenderFxFader` | Subsidiary entity for env_render fade effects |
| `dlls/triggers.cpp` | 1544 | `// amt fix for normal mode` | Fix render amount for fade purposes |
| `dlls/triggers.cpp` | 1563 | `// fade the entity in/out` | Entity fade-in/fade-out logic |
| `dlls/triggers.cpp` | 1839 | `//LRCT` | Experimental trigger code |
| `dlls/triggers.cpp` | 2187 | `// refactored trigger_changelevel` | Refactored trigger_changelevel |
| `dlls/triggers.cpp` | 2527 | `// trigger_changecvar` | trigger_changecvar: changes server cvars |
| `dlls/triggers.cpp` | 2978 | `// trigger_inout` | Fires only on enter/leave, maintains entity registry |
| `dlls/triggers.cpp` | 3072 | `// adding debug trace` | Debug trace for trigger_inout |
| `dlls/triggers.cpp` | 3092 | `// removing debug trace` | Debug trace for entity removal |
| `dlls/triggers.cpp` | 3385 | `// no capital letters in changelevels` | Changelevel name validation |
| `dlls/triggers.cpp` | 3790 | `// NODRAW optimization` | NODRAW is better-performance way to hide things |
| `dlls/triggers.cpp` | 3929 | `// trigger_bounce` | Bounces touching entities |
| `dlls/triggers.cpp` | 3969 | `// trigger_onsight` | Fires when entities can see each other |
| `dlls/triggers.cpp` | 4185 | `// landmark based teleports` | Enhanced teleporter with landmark support |
| `dlls/triggers.cpp` | 4251 | `pOther->pev->v_angle = pTarget->pev->angles;` | Set view angle on teleport |
| `dlls/triggers.cpp` | 4401 | `// trigger_startpatrol` | Starts monster patrol routes |
| `dlls/triggers.cpp` | 4459 | `// trigger_motion` | Applies position/angle/velocity to entities |
| `dlls/triggers.cpp` | 4700 | `// motion_manager` | Continuous motion control entity |
| `dlls/triggers.cpp` | 5049 | `// complex trigger entity` | Additional complex trigger |
| `dlls/triggers.cpp` | 5374 | `// additional trigger enhancement` | Additional trigger enhancements |
| `dlls/triggers.cpp` | 5453 | `if (pev->velocity.Length() < 10.0)` | Velocity threshold check |
| `dlls/triggers.cpp` | 376 | `// trigger_rottest` | Rotation testing entity |
| `dlls/triggers.cpp` | 31-33 | `#include "weapons.h", "movewith.h", "locus.h"` | New includes for trigger enhancements |

### 3.5 Locus System (dlls/cbase.h, dlls/locus.h)

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/cbase.h` | 248 | `// loci` | CalcPosition/CalcVelocity/CalcRatio virtual functions |
| `dlls/effects.cpp` | 26 | `#include "locus.h"` | Locus utilities included in effects |
| `dlls/effects.cpp` | 2547 | `Vector Direction(CBaseEntity* pActivator);` | Direction calculation with locus activator |
| `dlls/triggers.cpp` | 33 | `#include "locus.h"` | Locus utilities included in triggers |

### 3.6 Alias System (dlls/cbase.h)

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/cbase.h` | 253 | `// aliases` | CBaseAlias pointer and alias handling |
| `dlls/cbase.h` | 1001 | `// moved here from alias.cpp` | Alias class definitions in header |
| `dlls/util.cpp` | 598 | `// aliases change mid-traversal` | Alias flush safety mechanism |
| `dlls/util.h` | 616 | `// for aliases and groups` | Alias/group utility declarations |

## 4. State System

SoHL adds a `GetState()` virtual method to many entities, returning `STATE_ON`, `STATE_OFF`, `STATE_TURN_ON`, or `STATE_TURN_OFF`. This enables more sophisticated master/slave entity logic.

| File | Line | Code | Description |
|------|------|------|-------------|
| `cl_dll/hl/hl_baseentity.cpp` | 175 | `STATE CBaseToggle::GetState(void) { return STATE_ON; } //LRC` | LRC modification |
| `dlls/bmodels.cpp` | 401 | `virtual STATE GetState(void) { return m_iState; }; //LRC` | LRC modification |
| `dlls/buttons.cpp` | 443 | `//LRC- while we're in STATE_OFF, mastered entities can't do anything.` | while we're in STATE_OFF, mastered entities can't do anything. |
| `dlls/buttons.cpp` | 1706 | `m_iState = STATE_ON; //LRC` | LRC modification |
| `dlls/buttons.cpp` | 1714 | `m_iState = STATE_OFF; //LRC` | LRC modification |
| `dlls/cbase.h` | 324 | `//LRC- superceded by GetState ( pActivator ).` | superceded by GetState ( pActivator ). |
| `dlls/cbase.h` | 426 | `int ShouldToggle(USE_TYPE useType); //LRC this version uses GetState()` | this version uses GetState() |
| `dlls/effects.cpp` | 2676 | `virtual STATE GetState(void) { return m_iState; }; //LRC` | LRC modification |
| `dlls/effects.cpp` | 2677 | `void Think(void) { m_iState = STATE_OFF; }; //LRC` | LRC modification |
| `dlls/effects.cpp` | 2702 | `m_iState = STATE_OFF; //LRC` | LRC modification |
| `dlls/effects.cpp` | 2739 | `m_iState = STATE_ON; //LRC` | LRC modification |
| `dlls/effects.cpp` | 2751 | `virtual STATE GetState(void) { return m_iState; }; // LRC` | LRC modification |
| `dlls/effects.cpp` | 2780 | `m_iState = STATE_OFF; //LRC` | LRC modification |
| `dlls/effects.cpp` | 2805 | `m_iState = STATE_TURN_ON; //LRC` | LRC modification |
| `dlls/lights.cpp` | 109 | `virtual STATE GetState(void) { return m_iState; }; //LRC` | LRC modification |
| `dlls/plats.cpp` | 1064 | `if (m_iState == STATE_OFF) //LRC - don't restart the sound every time we hit a path_corner, it sounds weird` | don't restart the sound every time we hit a path_corner, it sounds weird |
| `dlls/scripted.cpp` | 189 | `m_iState = STATE_OFF; //LRC` | LRC modification |
| `dlls/scripted.cpp` | 424 | `m_iState = STATE_ON; // LRC: assume we'll set it to 'on', unless proven otherwise...` | assume we'll set it to 'on', unless proven otherwise... |
| `dlls/scripted.cpp` | 718 | `pTarget->m_iState = STATE_ON; //LRC` | LRC modification |
| `dlls/scripted.cpp` | 938 | `BOOL m_playing; //LRC- is the sentence playing? (for GetState)` | is the sentence playing? (for GetState) |
| `dlls/triggers.cpp` | 840 | `m_iState = STATE_OFF; //LRC- STATE_OFF means "yes, we've finished".` | STATE_OFF means "yes, we've finished". |
| `dlls/triggers.cpp` | 861 | `m_iState = STATE_ON; //LRC- while we're in STATE_ON we're firing targets, and haven't finished yet.` | while we're in STATE_ON we're firing targets, and haven't finished yet. |

## 5. Fog System

SoHL adds volumetric fog via `env_fog` entity, with client-side rendering, fog fading effects, and a `SetFog` network message.

| File | Line | Code | Description |
|------|------|------|-------------|
| `cl_dll/hl/hl_weapons.cpp` | 51 | `int g_iWaterLevel; //LRC - for DMC fog` | for DMC fog |
| `cl_dll/hl/hl_weapons.cpp` | 838 | `g_iWaterLevel = player.pev->waterlevel = from->client.waterlevel; //LRC - for DMC fog` | for DMC fog |
| `cl_dll/hud.cpp` | 361 | `HOOK_MESSAGE(SetFog); //LRC` | LRC modification |
| `cl_dll/hud.h` | 699 | `void _cdecl MsgFunc_SetFog(const char* pszName, int iSize, void* pbuf); //LRC` | LRC modification |
| `cl_dll/hud_msg.cpp` | 28 | `//LRC - the fogging fog` | the fogging fog |
| `cl_dll/hud_msg.cpp` | 69 | `//LRC - reset fog` | reset fog |
| `cl_dll/hud_msg.cpp` | 86 | `//LRC - clear the fog` | clear the fog |
| `cl_dll/hud_redraw.cpp` | 98 | `//LRC - fog fading values` | fog fading values |
| `cl_dll/hud_redraw.cpp` | 115 | `//LRC - handle fog fading effects. (is this the right place for it?)` | handle fog fading effects. (is this the right place for it?) |
| `cl_dll/view.cpp` | 489 | `extern void RenderFog(void); //LRC` | LRC modification |
| `dlls/effects.cpp` | 4323 | `// LRC - env_fog, extended a bit from the DMC version` | env_fog, extended a bit from the DMC version |
| `dlls/player.cpp` | 163 | `int gmsgSetFog = 0; //LRC` | LRC modification |
| `dlls/player.cpp` | 228 | `gmsgSetFog = REG_USER_MSG("SetFog", 9); //LRC` | LRC modification |

## 6. Sky System

SoHL adds a 3D skybox system via `env_sky`, allowing the view to be repositioned for sky rendering. Communicated via `SetSky` network message.

| File | Line | Code | Description |
|------|------|------|-------------|
| `cl_dll/hud.cpp` | 364 | `HOOK_MESSAGE(SetSky); //LRC` | LRC modification |
| `cl_dll/hud.h` | 591 | `//LRC - for the moment, skymode has only two settings` | for the moment, skymode has only two settings |
| `cl_dll/hud.h` | 624 | `Vector m_vecSkyPos; //LRC` | LRC modification |
| `cl_dll/hud.h` | 625 | `int m_iSkyMode; //LRC` | LRC modification |
| `cl_dll/hud.h` | 701 | `void _cdecl MsgFunc_SetSky(const char* pszName, int iSize, void* pbuf); //LRC` | LRC modification |
| `cl_dll/hud_msg.cpp` | 97 | `m_iSkyMode = SKY_OFF; //LRC` | LRC modification |
| `dlls/effects.cpp` | 4565 | `// LRC - env_sky, an unreal tournament-style sky effect` | env_sky, an unreal tournament-style sky effect |
| `dlls/player.cpp` | 165 | `int gmsgSetSky = 0; //LRC` | LRC modification |
| `dlls/player.cpp` | 230 | `gmsgSetSky = REG_USER_MSG("SetSky", 7); //LRC` | LRC modification |

## 7. Custom HUD Color

SoHL allows mappers to change the HUD color via the `HUDColor` message and `m_iHUDColor` member.

| File | Line | Code | Description |
|------|------|------|-------------|
| `cl_dll/ammo_secondary.cpp` | 63 | `UnpackRGB(r, g, b, gHUD.m_iHUDColor); //LRC` | LRC modification |
| `cl_dll/health.cpp` | 227 | `UnpackRGB(r, g, b, gHUD.m_iHUDColor); //LRC` | LRC modification |
| `cl_dll/hud.cpp` | 360 | `HOOK_MESSAGE(HUDColor); //LRC` | LRC modification |
| `cl_dll/hud.h` | 634 | `int m_iHUDColor; //LRC` | LRC modification |
| `cl_dll/hud.h` | 698 | `int _cdecl MsgFunc_HUDColor(const char* pszName, int iSize, void* pbuf); //LRC` | LRC modification |
| `dlls/player.cpp` | 166 | `int gmsgHUDColor = 0; //LRC` | LRC modification |
| `dlls/player.cpp` | 231 | `gmsgHUDColor = REG_USER_MSG("HUDColor", 4); //LRC` | LRC modification |

## 8. Shiny/Reflective Surfaces

SoHL adds reflective surface rendering via `CShinySurface` class, `AddShine` message, and `kRenderFxReflection` render mode.

| File | Line | Code | Description |
|------|------|------|-------------|
| `cl_dll/hud.cpp` | 363 | `HOOK_MESSAGE(AddShine); //LRC` | LRC modification |
| `cl_dll/hud.cpp` | 416 | `m_pShinySurface = NULL; //LRC` | LRC modification |
| `cl_dll/hud.h` | 623 | `CShinySurface* m_pShinySurface; //LRC` | LRC modification |
| `cl_dll/hud.h` | 702 | `void _cdecl MsgFunc_AddShine(const char* pszName, int iSize, void* pbuf); //LRC` | LRC modification |
| `cl_dll/hud_msg.cpp` | 90 | `//LRC - clear all shiny surfaces` | clear all shiny surfaces |
| `cl_dll/tri.cpp` | 50 | `//LRC - code for CShinySurface, declared in hud.h` | code for CShinySurface, declared in hud.h |
| `common/const.h` | 720 | `kRenderFxReflection,		//LRC - draw a reflection under my feet` | draw a reflection under my feet |
| `dlls/bmodels.cpp` | 276 | `//LRC - shiny surfaces` | shiny surfaces |
| `dlls/player.cpp` | 167 | `int gmsgAddShine = 0; // LRC` | LRC modification |
| `dlls/player.cpp` | 232 | `gmsgAddShine = REG_USER_MSG("AddShine", -1); //LRC` | LRC modification |

## 9. Dynamic Lighting

SoHL adds keyed dynamic lights via the `KeyedDLight` network message.

| File | Line | Code | Description |
|------|------|------|-------------|
| `cl_dll/hud.cpp` | 362 | `HOOK_MESSAGE(KeyedDLight); //LRC` | LRC modification |
| `cl_dll/hud.h` | 700 | `void _cdecl MsgFunc_KeyedDLight(const char* pszName, int iSize, void* pbuf); //LRC` | LRC modification |
| `dlls/effects.cpp` | 3886 | `//LRC- env_dlight; Dynamic Entity Light creator` | env_dlight; Dynamic Entity Light creator |
| `dlls/player.cpp` | 164 | `int gmsgKeyedDLight = 0; //LRC` | LRC modification |
| `dlls/player.cpp` | 229 | `gmsgKeyedDLight = REG_USER_MSG("KeyedDLight", -1); //LRC` | LRC modification |
| `dlls/util.h` | 614 | `int GetStdLightStyle(int iStyle); //LRC- declared here so it can be used by everything that` | declared here so it can be used by everything that |

## 10. Monster/NPC Enhancements

SoHL makes extensive changes to monsters/NPCs including:
- Custom model support via `SET_MODEL(ENT(pev), STRING(pev->model))` pattern (allows mappers to override models)
- Custom `PRECACHE_MODEL` using `pev->model` instead of hardcoded paths
- Monster allegiance system (`CLASS_FACTION_A/B/C`, "Behaves As")
- Custom gibs support (`HasCustomGibs`)
- Pre-disaster mode flag (`SF_MONSTER_PREDISASTER`)
- Modified `CanPlaySequence` prototype
- Scripted action precision attacks (`PreciseAttack()`)
- Base body tracking for Barney (`m_iBaseBody`)
- Health default checks (`pev->health == 0`)
- Various combat and AI tweaks

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/AI_BaseNPC_Schedule.cpp` | 1291 | `TaskComplete(); //LRC - start playing immediately` | start playing immediately |
| `dlls/AI_BaseNPC_Schedule.cpp` | 1351 | `//LRC` | LRC modification |
| `dlls/AI_BaseNPC_Schedule.cpp` | 1363 | `// LRC - if it's a teleport script, do the turn too` | if it's a teleport script, do the turn too |
| `dlls/AI_BaseNPC_Schedule.cpp` | 1366 | `if (m_pCine->m_fTurnType == 0) //LRC` | LRC modification |
| `dlls/agrunt.cpp` | 71 | `//LRC - body definitions for the Agrunt model` | body definitions for the Agrunt model |
| `dlls/agrunt.cpp` | 494 | `//LRC - hornets have the same allegiance as their creators` | hornets have the same allegiance as their creators |
| `dlls/agrunt.cpp` | 502 | `if (m_pCine && m_pCine->PreciseAttack()) //LRC- are we doing a scripted action?` | are we doing a scripted action? |
| `dlls/agrunt.cpp` | 611 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/agrunt.cpp` | 643 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/apache.cpp` | 126 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/apache.cpp` | 163 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/barnacle.cpp` | 109 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/barnacle.cpp` | 294 | `pTouchEnt->pev->velocity = pev->velocity; //LRC- make him come _with_ me` | make him come _with_ me |
| `dlls/barnacle.cpp` | 295 | `pTouchEnt->pev->basevelocity = pev->velocity; //LRC` | LRC modification |
| `dlls/barnacle.cpp` | 389 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/barney.cpp` | 80 | `int m_iBaseBody; //LRC - for barneys with different bodies` | for barneys with different bodies |
| `dlls/barney.cpp` | 96 | `DEFINE_FIELD(CBarney, m_iBaseBody, FIELD_INTEGER), //LRC` | LRC modification |
| `dlls/barney.cpp` | 432 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/barney.cpp` | 440 | `if (pev->health == 0) //LRC` | LRC modification |
| `dlls/barney.cpp` | 446 | `m_iBaseBody = pev->body; //LRC` | LRC modification |
| `dlls/barney.cpp` | 462 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/barney.cpp` | 496 | `if (pev->spawnflags & SF_MONSTER_PREDISASTER) //LRC` | LRC modification |
| `dlls/barney.cpp` | 557 | `// LRC - if my reaction to the player has been overridden, don't do this stuff` | if my reaction to the player has been overridden, don't do this stuff |
| `dlls/barney.cpp` | 868 | `PlaySentence(m_szGrp[TLK_DECLINE], 2, VOL_NORM, ATTN_NORM); //LRC` | LRC modification |
| `dlls/basemonster.h` | 128 | `// LRC- to allow level-designers to change monster allegiances` | to allow level-designers to change monster allegiances |
| `dlls/basemonster.h` | 326 | `virtual int HasCustomGibs(void) { return FALSE; } //LRC` | LRC modification |
| `dlls/basemonster.h` | 382 | `//LRC` | LRC modification |
| `dlls/bigmomma.cpp` | 29 | `//LRC brought in from animation.h` | brought in from animation.h |
| `dlls/bigmomma.cpp` | 684 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/bigmomma.cpp` | 713 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/bloater.cpp` | 194 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/bloater.cpp` | 217 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/bullsquid.cpp` | 231 | `LINK_ENTITY_TO_CLASS(monster_bullsquid, CBullsquid); //LRC - let's get the right name...` | let's get the right name... |
| `dlls/bullsquid.cpp` | 554 | `if (m_pCine) // LRC- are we being told to do this by a scripted_action?` | are we being told to do this by a scripted_action? |
| `dlls/bullsquid.cpp` | 695 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/bullsquid.cpp` | 721 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/combat.cpp` | 32 | `#include "../engine/studio.h" //LRC` | LRC modification |
| `dlls/combat.cpp` | 194 | `//LRC - changed signature, to support custom gib models` | changed signature, to support custom gib models |
| `dlls/combat.cpp` | 202 | `//LRC - check the model itself to find out how many gibs are available` | check the model itself to find out how many gibs are available |
| `dlls/combat.cpp` | 265 | `pGib = NULL; //LRC` | LRC modification |
| `dlls/combat.cpp` | 269 | `//LRC - work out gibs from blood colour, instead of from class.` | work out gibs from blood colour, instead of from class. |
| `dlls/combat.cpp` | 297 | `//LRC - work out gibs from blood colour, instead.` | work out gibs from blood colour, instead. |
| `dlls/combat.cpp` | 351 | `if (iszCustomGibs = HasCustomGibs()) //LRC - monster_generic can have a custom gibset` | monster_generic can have a custom gibset |
| `dlls/combat.cpp` | 842 | `pev->solid = SOLID_TRIGGER; //LRC - so that they don't get in each other's way when we fire lots` | so that they don't get in each other's way when we fire lots |
| `dlls/combat.cpp` | 986 | `//LRC - new behaviours, for m_iPlayerReact.` | new behaviours, for m_iPlayerReact. |
| `dlls/combat.cpp` | 1321 | `if (tr.flFraction != 1.0 && tr.pHit != ENT(pEntity->pev)) //LRC - added so that monsters can "see" some bsp objects` | added so that monsters can "see" some bsp objects |
| `dlls/controller.cpp` | 371 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/controller.cpp` | 396 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/controller.cpp` | 673 | `if (m_pCine != NULL) // LRC- is this a script that's telling it to fire?` | is this a script that's telling it to fire? |
| `dlls/defaultai.cpp` | 800 | `//LRC` | LRC modification |
| `dlls/defaultai.cpp` | 809 | `//LRC` | LRC modification |
| `dlls/gargantua.cpp` | 763 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/gargantua.cpp` | 796 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/gargantua.cpp` | 1278 | `if (m_pCine) // LRC- are we obeying a scripted_action?` | are we obeying a scripted_action? |
| `dlls/genericmonster.cpp` | 27 | `//LRC- this seems to interfere with SF_MONSTER_CLIP` | this seems to interfere with SF_MONSTER_CLIP |
| `dlls/genericmonster.cpp` | 141 | `//LRC - if the level designer forgets to set a model, don't crash!` | if the level designer forgets to set a model, don't crash! |
| `dlls/genericmonster.cpp` | 203 | `PRECACHE_MODEL((char*)STRING(m_iszGibModel)); //LRC` | LRC modification |
| `dlls/genericmonster.cpp` | 265 | `pev->yaw_speed = 8; //LRC -- what?` | - what? |
| `dlls/genericmonster.cpp` | 300 | `PRECACHE_MODEL((char*)STRING(m_iszGibModel)); //LRC` | LRC modification |
| `dlls/gman.cpp` | 126 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/gman.cpp` | 147 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/hassassin.cpp` | 200 | `if (m_hEnemy == NULL && !m_pCine) //LRC` | LRC modification |
| `dlls/hassassin.cpp` | 265 | `//LRC` | LRC modification |
| `dlls/hassassin.cpp` | 299 | `if (m_pCine) //LRC...` | ... |
| `dlls/hassassin.cpp` | 344 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/hassassin.cpp` | 376 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/headcrab.cpp` | 286 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/headcrab.cpp` | 319 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/headcrab.cpp` | 520 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/headcrab.cpp` | 533 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/hgrunt.cpp` | 270 | `//LRC- only hate alien grunts if my behaviour hasn't been overridden` | only hate alien grunts if my behaviour hasn't been overridden |
| `dlls/hgrunt.cpp` | 797 | `if (m_hEnemy == NULL && m_pCine == NULL) //LRC - scripts may fire when you have no enemy` | scripts may fire when you have no enemy |
| `dlls/hgrunt.cpp` | 867 | `if (pev->spawnflags & SF_MONSTER_NO_WPN_DROP) break; //LRC` | LRC modification |
| `dlls/hgrunt.cpp` | 903 | `//LRC - a bit of a hack. Ideally the grunts would work out in advance whether it's ok to throw.` | a bit of a hack. Ideally the grunts would work out in advance whether it's ok to throw. |
| `dlls/hgrunt.cpp` | 931 | `//LRC: firing due to a script?` | firing due to a script? |
| `dlls/hgrunt.cpp` | 1047 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/hgrunt.cpp` | 1116 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/hgrunt.cpp` | 1120 | `PRECACHE_SOUND("weapons/dryfire1.wav"); //LRC` | LRC modification |
| `dlls/hgrunt.cpp` | 1982 | `// LRC: added a test to stop a marine without a launcher from firing.` | added a test to stop a marine without a launcher from firing. |
| `dlls/houndeye.cpp` | 336 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/houndeye.cpp` | 364 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/ichthyosaur.cpp` | 481 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/ichthyosaur.cpp` | 522 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/islave.cpp` | 538 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/islave.cpp` | 567 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/islave.cpp` | 611 | `//LRC - if my player reaction has been overridden, leave this alone` | if my player reaction has been overridden, leave this alone |
| `dlls/islave_deamon.cpp` | 530 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/islave_deamon.cpp` | 559 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/islave_deamon.cpp` | 597 | `//LRC - if my player reaction has been overridden, leave this alone` | if my player reaction has been overridden, leave this alone |
| `dlls/leech.cpp` | 182 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/leech.cpp` | 301 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/monstermaker.cpp` | 48 | `void TryMakeMonster(void); //LRC - to allow for a spawndelay` | to allow for a spawndelay |
| `dlls/monstermaker.cpp` | 49 | `CBaseMonster* MakeMonster(void); //LRC - actually make a monster (and return the new creation)` | actually make a monster (and return the new creation) |
| `dlls/monstermaker.cpp` | 69 | `float m_fSpawnDelay; // LRC- delay between triggering targets and making a child (for env_warpball, mainly)` | delay between triggering targets and making a child (for env_warpball, mainly) |
| `dlls/monstermaker.cpp` | 274 | `//LRC - custom monster behaviour` | custom monster behaviour |
| `dlls/monsters.cpp` | 2137 | `//LRC- there are perfectly good reasons for making a monster stuck, so it shouldn't always be an error.` | there are perfectly good reasons for making a monster stuck, so it shouldn't always be an error. |
| `dlls/monsters.cpp` | 2251 | `if (iTargClass == CLASS_PLAYER && m_iPlayerReact) //LRC` | LRC modification |
| `dlls/monsters.cpp` | 3040 | `else if (FStrEq(pkvd->szKeyName, "m_iClass")) //LRC` | LRC modification |
| `dlls/monsters.cpp` | 3045 | `else if (FStrEq(pkvd->szKeyName, "m_iPlayerReact")) //LRC` | LRC modification |
| `dlls/monsters.cpp` | 3165 | `//LRC - to help debug when sequences won't play...` | to help debug when sequences won't play... |
| `dlls/monsters.cpp` | 3551 | `//LRC - an entity for monsters to shoot at.` | an entity for monsters to shoot at. |
| `dlls/monsters.h` | 51 | `#define SF_MONSTER_NO_YELLOW_BLOBS		128 //LRC- if the monster is stuck, don't give errors or show yellow blobs.` | if the monster is stuck, don't give errors or show yellow blobs. |
| `dlls/monsters.h` | 52 | `//LRC- wasn't implemented. #define	SF_MONSTER_WAIT_FOR_SCRIPT		128 //spawnflag that makes monsters wait to check for ...` | wasn't implemented. #define	SF_MONSTER_WAIT_FOR_SCRIPT		128 //spawnflag that makes monsters wait to check for attacking until the script is done or they've been attacked |
| `dlls/monsters.h` | 55 | `#define SF_MONSTER_NO_WPN_DROP			1024 //LRC- never drop your weapon (player can't pick it up.)` | never drop your weapon (player can't pick it up.) |
| `dlls/monsters.h` | 56 | `//LRC - this clashes with 'not in deathmatch'. Replaced with m_iPlayerReact.` | this clashes with 'not in deathmatch'. Replaced with m_iPlayerReact. |
| `dlls/monsters.h` | 57 | `//#define SF_MONSTER_INVERT_PLAYERREACT	2048 //LRC- if this monster would usually attack the player, don't attack unl...` | if this monster would usually attack the player, don't attack unless provoked. If you would usually NOT attack the player, attack him. |
| `dlls/monsters.h` | 161 | `static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, int notfirst, const char* szGibModel); //LRC` | LRC modification |
| `dlls/nihilanth.cpp` | 289 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/nihilanth.cpp` | 338 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/osprey.cpp` | 151 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/osprey.cpp` | 165 | `pev->speed = 80; //LRC - default speed, in case path corners don't give a speed.` | default speed, in case path corners don't give a speed. |
| `dlls/osprey.cpp` | 194 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/osprey.cpp` | 232 | `m_iUnits = 4; //LRC - stop whining, just make the damn grunts...` | stop whining, just make the damn grunts... |
| `dlls/osprey.cpp` | 368 | `//LRC - ugh. we shouldn't require our path corners to specify a speed!` | ugh. we shouldn't require our path corners to specify a speed! |
| `dlls/osprey.cpp` | 372 | `m_vel2 = gpGlobals->v_forward * pev->speed; //LRC` | LRC modification |
| `dlls/osprey.cpp` | 417 | `int loopbreaker = 100; //LRC - <slap> don't loop indefinitely!` | <slap> don't loop indefinitely! |
| `dlls/osprey.cpp` | 421 | `loopbreaker--; //LRC` | LRC modification |
| `dlls/rat.cpp` | 46 | `return m_iClass ? m_iClass : CLASS_INSECT; //LRC- maybe someone needs to give them a basic biology lesson...` | maybe someone needs to give them a basic biology lesson... |
| `dlls/rat.cpp` | 76 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/rat.cpp` | 98 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/roach.cpp` | 125 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/roach.cpp` | 155 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/scientist.cpp` | 665 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/scientist.cpp` | 707 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/scientist.cpp` | 1254 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/scientist.cpp` | 1258 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/scientist.cpp` | 1277 | `if (!FBitSet(pev->spawnflags, SF_SITTINGSCI_POSTDISASTER)) //LRC- allow a sitter to be postdisaster.` | allow a sitter to be postdisaster. |
| `dlls/talkmonster.cpp` | 45 | `DEFINE_FIELD(CTalkMonster, m_iszDecline, FIELD_STRING), //LRC` | LRC modification |
| `dlls/talkmonster.cpp` | 46 | `DEFINE_FIELD(CTalkMonster, m_iszSpeakAs, FIELD_STRING), //LRC` | LRC modification |
| `dlls/talkmonster.cpp` | 794 | `if (m_iszSpeakAs) //LRC: changing voice groups for monsters` | changing voice groups for monsters |
| `dlls/talkmonster.cpp` | 801 | `//LRC - this is pretty dodgy; test with save/restore.` | this is pretty dodgy; test with save/restore. |
| `dlls/talkmonster.cpp` | 810 | `if (pev->spawnflags & SF_MONSTER_PREDISASTER) //LRC` | LRC modification |
| `dlls/talkmonster.cpp` | 815 | `if (pev->spawnflags & SF_MONSTER_PREDISASTER) //LRC` | LRC modification |
| `dlls/talkmonster.cpp` | 820 | `if (pev->spawnflags & SF_MONSTER_PREDISASTER) //LRC` | LRC modification |
| `dlls/talkmonster.cpp` | 1456 | `//LRC- redefined, now returns true if following would be physically possible` | redefined, now returns true if following would be physically possible |
| `dlls/talkmonster.cpp` | 1472 | `//LRC- rewritten` | rewritten |
| `dlls/talkmonster.cpp` | 1528 | `else if (FStrEq(pkvd->szKeyName, "RefusalSentence")) //LRC` | LRC modification |
| `dlls/talkmonster.cpp` | 1533 | `else if (FStrEq(pkvd->szKeyName, "SpeakAs")) //LRC` | LRC modification |
| `dlls/talkmonster.cpp` | 1549 | `if (m_iszDecline) //LRC` | LRC modification |
| `dlls/talkmonster.h` | 51 | `//LRC- refuse to accompany` | refuse to accompany |
| `dlls/tempmonster.cpp` | 91 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/zombie.cpp` | 290 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/zombie.cpp` | 316 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |

## 11. Func_Tank Enhancements

SoHL significantly enhances func_tank entities with parent entity support, operability from specific points, and improved firing logic.

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/func_tank.cpp` | 33 | `#define SF_TANK_LASERSPOT		0x0040 //LRC` | LRC modification |
| `dlls/func_tank.cpp` | 34 | `#define SF_TANK_MATCHTARGET		0x0080 //LRC` | LRC modification |
| `dlls/func_tank.cpp` | 36 | `#define SF_TANK_SEQFIRE			0x10000 //LRC - a TankSequence is telling me to fire` | a TankSequence is telling me to fire |
| `dlls/func_tank.cpp` | 69 | `int m_iCrosshair; //LRC - show a crosshair while in use. (currently this is just yes or no,` | show a crosshair while in use. (currently this is just yes or no, |
| `dlls/func_tank.cpp` | 203 | `CFuncTankControls* m_pControls; //LRC - tankcontrols is used as a go-between.` | tankcontrols is used as a go-between. |
| `dlls/func_tank.cpp` | 208 | `CTankSequence* m_pSequence; //LRC - if set, then this is the sequence the tank is currently performing` | if set, then this is the sequence the tank is currently performing |
| `dlls/func_tank.cpp` | 209 | `CBaseEntity* m_pSequenceEnemy; //LRC - the entity that our sequence wants us to attack` | the entity that our sequence wants us to attack |
| `dlls/func_tank.cpp` | 212 | `//LRC - unprotected these, so that TankSequence can look at them` | unprotected these, so that TankSequence can look at them |
| `dlls/func_tank.cpp` | 220 | `//LRC	Vector		m_vecControllerUsePos;` | Vector		m_vecControllerUsePos; |
| `dlls/func_tank.cpp` | 247 | `int m_iszFireMaster; //LRC - Fire-Master entity (prevents firing when inactive)` | Fire-Master entity (prevents firing when inactive) |
| `dlls/func_tank.cpp` | 254 | `CPointEntity* m_pFireProxy; //LRC - locus position for custom shots` | locus position for custom shots |
| `dlls/func_tank.cpp` | 281 | `DEFINE_FIELD(CFuncTank, m_pControls, FIELD_CLASSPTR), //LRC` | LRC modification |
| `dlls/func_tank.cpp` | 282 | `DEFINE_FIELD(CFuncTank, m_pSequence, FIELD_CLASSPTR), //LRC` | LRC modification |
| `dlls/func_tank.cpp` | 283 | `DEFINE_FIELD(CFuncTank, m_pSequenceEnemy, FIELD_CLASSPTR), //LRC` | LRC modification |
| `dlls/func_tank.cpp` | 284 | `DEFINE_FIELD(CFuncTank, m_pSpot, FIELD_CLASSPTR), //LRC` | LRC modification |
| `dlls/func_tank.cpp` | 285 | `//LRC	DEFINE_FIELD( CFuncTank, m_pController, FIELD_CLASSPTR ),` | DEFINE_FIELD( CFuncTank, m_pController, FIELD_CLASSPTR ), |
| `dlls/func_tank.cpp` | 286 | `//LRC	DEFINE_FIELD( CFuncTank, m_vecControllerUsePos, FIELD_VECTOR ),` | DEFINE_FIELD( CFuncTank, m_vecControllerUsePos, FIELD_VECTOR ), |
| `dlls/func_tank.cpp` | 290 | `DEFINE_FIELD(CFuncTank, m_iszFireMaster, FIELD_STRING), //LRC` | LRC modification |
| `dlls/func_tank.cpp` | 291 | `DEFINE_FIELD(CFuncTank, m_iszLocusFire, FIELD_STRING), //LRC` | LRC modification |
| `dlls/func_tank.cpp` | 292 | `DEFINE_FIELD(CFuncTank, m_pFireProxy, FIELD_CLASSPTR), //LRC` | LRC modification |
| `dlls/func_tank.cpp` | 316 | `//	if (pev->health) pev->flags \|= FL_MONSTER; //LRC - maybe?` | maybe? |
| `dlls/func_tank.cpp` | 332 | `if (m_iszLocusFire) //LRC - locus trigger` | locus trigger |
| `dlls/func_tank.cpp` | 543 | `//LRC- various commands moved from here to FuncTankControls` | various commands moved from here to FuncTankControls |
| `dlls/func_tank.cpp` | 556 | `StopRotSound(); //LRC` | LRC modification |
| `dlls/func_tank.cpp` | 599 | `// LRC- this is now never called. Think functions are handling it all.` | this is now never called. Think functions are handling it all. |
| `dlls/func_tank.cpp` | 638 | `// LRC- actually, we handle firing with TrackTarget, to support multitank.` | actually, we handle firing with TrackTarget, to support multitank. |
| `dlls/func_tank.cpp` | 642 | `// LRC- tankcontrols handles all this` | tankcontrols handles all this |
| `dlls/func_tank.cpp` | 645 | `//			// LRC- add one more tank to the ones the player's using` | add one more tank to the ones the player's using |
| `dlls/func_tank.cpp` | 810 | `//LRC` | LRC modification |
| `dlls/func_tank.cpp` | 817 | `//LRC` | LRC modification |
| `dlls/func_tank.cpp` | 895 | `// LRC- changed here to allow "match target" as well as "match angles" mode.` | changed here to allow "match target" as well as "match angles" mode. |
| `dlls/func_tank.cpp` | 906 | `edict_t* ownerTemp = pev->owner; //LRC store the owner, so we can put it back after the check` | store the owner, so we can put it back after the check |
| `dlls/func_tank.cpp` | 907 | `pev->owner = pController->edict(); //LRC when doing the matchtarget check, don't hit the player or the tank.` | when doing the matchtarget check, don't hit the player or the tank. |
| `dlls/func_tank.cpp` | 917 | `pev->owner = ownerTemp; //LRC put the owner back` | put the owner back |
| `dlls/func_tank.cpp` | 1219 | `//LRC` | LRC modification |
| `dlls/func_tank.cpp` | 1433 | `//LRC - tripbeams` | tripbeams |
| `dlls/func_tank.cpp` | 1550 | `DEFINE_FIELD(CFuncTankControls, m_iCrosshair, FIELD_INTEGER), //LRC` | LRC modification |
| `dlls/func_tank.cpp` | 1575 | `//LRC- copied here from FuncTank.` | copied here from FuncTank. |
| `dlls/func_tank.cpp` | 1610 | `// LRC- rewritten to allow TankControls to be the thing that handles the relationship` | rewritten to allow TankControls to be the thing that handles the relationship |
| `dlls/func_tank.cpp` | 1626 | `//LRC- Now uses FindEntityByTargetname, so that aliases work.` | Now uses FindEntityByTargetname, so that aliases work. |
| `dlls/func_tank.cpp` | 1651 | `//LRC - allow tank crosshairs` | allow tank crosshairs |
| `dlls/func_tank.cpp` | 1670 | `//LRC- Now uses FindEntityByTargetname, so that aliases work.` | Now uses FindEntityByTargetname, so that aliases work. |
| `dlls/func_tank.cpp` | 1741 | `if (pev->frags == 0) //LRC- in case the level designer didn't set it.` | in case the level designer didn't set it. |
| `dlls/func_tank.cpp` | 1747 | `//LRC	SetNextThink( 0.3 );	// After all the func_tanks have spawned` | SetNextThink( 0.3 );	// After all the func_tanks have spawned |
| `dlls/func_tank.cpp` | 1753 | `//LRC - Scripted Tank Sequence` | Scripted Tank Sequence |

## 12. Platform/Train Enhancements

SoHL enhances platforms and trains with MoveWith support, state system, and improved path following.

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/plats.cpp` | 65 | `//LRC - scripted_trainsequence` | scripted_trainsequence |
| `dlls/plats.cpp` | 762 | `//LRC` | LRC modification |
| `dlls/plats.cpp` | 782 | `//LRC - now part of CBaseEntity:	BOOL		m_activated;` | now part of CBaseEntity:	BOOL		m_activated; |
| `dlls/plats.cpp` | 793 | `//LRC - now part of CBaseEntity:	DEFINE_FIELD( CFuncTrain, m_activated, FIELD_BOOLEAN ),` | now part of CBaseEntity:	DEFINE_FIELD( CFuncTrain, m_activated, FIELD_BOOLEAN ), |
| `dlls/plats.cpp` | 796 | `DEFINE_FIELD(CFuncTrain, m_pSequence, FIELD_CLASSPTR), //LRC` | LRC modification |
| `dlls/plats.cpp` | 797 | `DEFINE_FIELD(CFuncTrain, m_vecAvelocity, FIELD_VECTOR), //LRC` | LRC modification |
| `dlls/plats.cpp` | 961 | `//LRC - search backwards` | search backwards |
| `dlls/plats.cpp` | 1016 | `//pev->avelocity = pTarg->pev->avelocity; //LRC` | LRC modification |
| `dlls/plats.cpp` | 1022 | `//pev->angles = m_pevCurrentTarget->angles; //LRC - if we just passed a "turn to face" corner, set angle exactly.` | if we just passed a "turn to face" corner, set angle exactly. |
| `dlls/plats.cpp` | 1030 | `if (FBitSet(pTarg->pev->spawnflags, SF_CORNER_TELEPORT)) //LRC - cosmetic change to use pTarg` | cosmetic change to use pTarg |
| `dlls/plats.cpp` | 1049 | `if (pTarg->pev->armorvalue) //LRC - "teleport and turn to face" means you set an angle as you teleport.` | "teleport and turn to face" means you set an angle as you teleport. |
| `dlls/plats.cpp` | 1074 | `if (pTarg->pev->armorvalue) //LRC - "turn to face" the next corner` | "turn to face" the next corner |
| `dlls/plats.cpp` | 1110 | `//LRC- called by Activate. (but not when a game is loaded.)` | called by Activate. (but not when a game is loaded.) |
| `dlls/plats.cpp` | 1128 | `pev->message = pevTarg->targetname; //LRC - record the old target so that we can find it again` | record the old target so that we can find it again |
| `dlls/plats.cpp` | 1179 | `//LRC` | LRC modification |
| `dlls/plats.cpp` | 1189 | `//LRC` | LRC modification |
| `dlls/plats.cpp` | 1227 | `else if (pev->dmg == -1) //LRC- a train that doesn't crush people!` | a train that doesn't crush people! |
| `dlls/plats.cpp` | 1247 | `//LRC - making movement sounds which continue after a game is loaded.` | making movement sounds which continue after a game is loaded. |
| `dlls/plats.cpp` | 1257 | `//LRC` | LRC modification |
| `dlls/plats.cpp` | 1273 | `//LRC - continue the movement sound after loading a game` | continue the movement sound after loading a game |
| `dlls/plats.cpp` | 1351 | `m_vecBaseAvel = pev->avelocity; //LRC - save it for later` | save it for later |
| `dlls/plats.cpp` | 1421 | `PRECACHE_SOUND((char*)STRING(pev->noise)); //LRC` | LRC modification |
| `dlls/plats.cpp` | 1442 | `DEFINE_FIELD(CFuncTrackTrain, m_vecMasterAvel, FIELD_VECTOR), //LRC` | LRC modification |
| `dlls/plats.cpp` | 1443 | `DEFINE_FIELD(CFuncTrackTrain, m_vecBaseAvel, FIELD_VECTOR), //LRC` | LRC modification |
| `dlls/plats.cpp` | 1444 | `DEFINE_FIELD(CFuncTrackTrain, m_pSequence, FIELD_CLASSPTR), //LRC` | LRC modification |
| `dlls/plats.cpp` | 1560 | `UTIL_SetVelocity(this, g_vecZero); //LRC` | LRC modification |
| `dlls/plats.cpp` | 1563 | `UTIL_SetAvelocity(this, g_vecZero); //LRC` | LRC modification |
| `dlls/plats.cpp` | 1588 | `//pev->avelocity = m_vecMasterAvel * delta; //LRC` | LRC modification |
| `dlls/plats.cpp` | 1605 | `if (m_sounds) //LRC - flashy event-based method, for normal sounds.` | flashy event-based method, for normal sounds. |
| `dlls/plats.cpp` | 1617 | `//LRC - down-to-earth method, for custom sounds.` | down-to-earth method, for custom sounds. |
| `dlls/plats.cpp` | 1649 | `if (m_sounds) //LRC - flashy event-based method, for normal sounds.` | flashy event-based method, for normal sounds. |
| `dlls/plats.cpp` | 1668 | `//LRC - down-to-earth method, for custom sounds.` | down-to-earth method, for custom sounds. |
| `dlls/plats.cpp` | 1722 | `UTIL_SetVelocity(this, (nextPos - pev->origin) * 10); //LRC` | LRC modification |
| `dlls/plats.cpp` | 1735 | `if (!FBitSet(pev->spawnflags, SF_TRACKTRAIN_AVELOCITY)) //LRC` | LRC modification |
| `dlls/plats.cpp` | 1741 | `angles.y += 180; //LRC, FIXME: add a 'built facing' field.` | , FIXME: add a 'built facing' field. |
| `dlls/plats.cpp` | 1756 | `if (!(pev->spawnflags & SF_TRACKTRAIN_NOYAW)) //LRC` | LRC modification |
| `dlls/plats.cpp` | 1803 | `//LRC is "match angle" set to "Yes"? If so, set the angle exactly, because we've reached the corner.` | is "match angle" set to "Yes"? If so, set the angle exactly, because we've reached the corner. |
| `dlls/plats.cpp` | 1814 | `//LRC - one of { 1, 0.75, 0.5, 0.25, 0, ... -1 }` | one of { 1, 0.75, 0.5, 0.25, 0, ... -1 } |
| `dlls/plats.cpp` | 1816 | `//LRC` | LRC modification |
| `dlls/plats.cpp` | 1839 | `CPathTrack* pDest; //LRC - the path_track we're heading for, after pFire.` | the path_track we're heading for, after pFire. |
| `dlls/plats.cpp` | 1846 | `//LRC` | LRC modification |
| `dlls/plats.cpp` | 1884 | `//LRC` | LRC modification |
| `dlls/plats.cpp` | 1914 | `//LRC- FIXME: add support, here, for a Teleport flag.` | FIXME: add support, here, for a Teleport flag. |
| `dlls/plats.cpp` | 1925 | `Vector vecTemp; //LRC` | LRC modification |
| `dlls/plats.cpp` | 1927 | `vecTemp = (nextPos - pev->origin); //LRC` | LRC modification |
| `dlls/plats.cpp` | 1931 | `//		UTIL_SetVelocity( this, (nextPos - pev->origin) * 10 ); //LRC` | LRC modification |
| `dlls/plats.cpp` | 1933 | `if (!FBitSet(pev->spawnflags, SF_TRACKTRAIN_AVELOCITY)) //LRC` | LRC modification |
| `dlls/plats.cpp` | 1936 | `float distance = vecTemp.Length(); //LRC` | LRC modification |
| `dlls/plats.cpp` | 1950 | `UTIL_SetVelocity(this, vecTemp * (m_oldSpeed / distance)); //LRC` | LRC modification |
| `dlls/plats.cpp` | 1957 | `UTIL_SetVelocity(this, vecTemp); //LRC` | LRC modification |
| `dlls/plats.cpp` | 1999 | `UTIL_SetVelocity(this, g_vecZero); //LRC` | LRC modification |
| `dlls/plats.cpp` | 2001 | `if (!FBitSet(pev->spawnflags, SF_TRACKTRAIN_AVELOCITY)) //LRC` | LRC modification |
| `dlls/plats.cpp` | 2079 | `UTIL_SetAngles(this, vTemp); //LRC` | LRC modification |
| `dlls/plats.cpp` | 2081 | `UTIL_AssignOrigin(this, nextPos); //LRC` | LRC modification |
| `dlls/plats.cpp` | 2085 | `//	NextThink( 8, FALSE ); //LRC - What was this for?!` | What was this for?! |
| `dlls/plats.cpp` | 2155 | `//LRC` | LRC modification |
| `dlls/plats.cpp` | 2164 | `//LRC` | LRC modification |
| `dlls/plats.cpp` | 2933 | `//LRC - Scripted Train Sequence` | Scripted Train Sequence |
| `dlls/trains.h` | 23 | `#define SF_TRACKTRAIN_NOYAW			0x0010		//LRC` | LRC modification |
| `dlls/trains.h` | 24 | `#define SF_TRACKTRAIN_AVELOCITY		0x800000	//LRC - avelocity has been set manually, don't turn.` | avelocity has been set manually, don't turn. |
| `dlls/trains.h` | 25 | `#define SF_TRACKTRAIN_AVEL_GEARS	0x400000	//LRC - avelocity should be scaled up/down when the train changes gear.` | avelocity should be scaled up/down when the train changes gear. |
| `dlls/trains.h` | 33 | `#define SF_PATH_AVELOCITY		0x00080000 //LRC` | LRC modification |
| `dlls/trains.h` | 41 | `//LRC - values in 'armortype'` | values in 'armortype' |
| `dlls/trains.h` | 47 | `//LRC - values in 'frags'` | values in 'frags' |
| `dlls/trains.h` | 52 | `//LRC - values in 'armorvalue'` | values in 'armorvalue' |
| `dlls/trains.h` | 107 | `//LRC` | LRC modification |
| `dlls/trains.h` | 112 | `void DesiredAction(void); //LRC - used to be called Next!` | used to be called Next! |
| `dlls/trains.h` | 159 | `Vector m_vecMasterAvel; //LRC - masterAvel is to avelocity as m_speed is to speed.` | masterAvel is to avelocity as m_speed is to speed. |
| `dlls/trains.h` | 160 | `Vector m_vecBaseAvel; // LRC - the underlying avelocity, superceded by normal turning behaviour where applicable` | the underlying avelocity, superceded by normal turning behaviour where applicable |

## 13. Door Enhancements

SoHL modifies doors with direct-use capability, immediate-use flags, and MoveWith integration.

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/doors.cpp` | 411 | `//LRC` | LRC modification |
| `dlls/doors.cpp` | 669 | `//LRC- allow flags to override this` | allow flags to override this |
| `dlls/doors.cpp` | 694 | `//LRC:` | LRC modification |
| `dlls/doors.cpp` | 783 | `// LRC- if synched, we fire as soon as we start to go up` | if synched, we fire as soon as we start to go up |
| `dlls/doors.cpp` | 869 | `// LRC` | LRC modification |
| `dlls/doors.cpp` | 899 | `// LRC- if synched, we fire as soon as we start to go down` | if synched, we fire as soon as we start to go down |
| `dlls/doors.cpp` | 911 | `// LRC- if synched, we fire as soon as we start to go down` | if synched, we fire as soon as we start to go down |
| `dlls/doors.cpp` | 946 | `// LRC- 'message' is the open target` | 'message' is the open target |
| `dlls/doors.cpp` | 968 | `// LRC- if synched, don't fire now` | if synched, don't fire now |
| `dlls/doors.cpp` | 994 | `//LRC - thanks to [insert name here] for this` | thanks to [insert name here] for this |
| `dlls/doors.cpp` | 1008 | `//LRC - in immediate mode don't do this, doors are expected to do it themselves.` | in immediate mode don't do this, doors are expected to do it themselves. |
| `dlls/doors.cpp` | 1236 | `//LRC: FIXME, move to PostSpawn` | FIXME, move to PostSpawn |
| `dlls/doors.cpp` | 1339 | `//LRC- move at the given speed, if any.` | move at the given speed, if any. |
| `dlls/doors.cpp` | 1356 | `//LRC- nope, in a MoveWith world you can't rely on that. Check the state instead.` | nope, in a MoveWith world you can't rely on that. Check the state instead. |
| `dlls/doors.h` | 29 | `#define SF_DOOR_FORCETOUCHABLE		1024 //LRC- Opens when touched, even though it's named and/or "use only"` | Opens when touched, even though it's named and/or "use only" |
| `dlls/doors.h` | 30 | `//LRC - clashes with 'not in deathmatch'. Replaced with 'Target mode' and 'On/Off Mode' fields.` | clashes with 'not in deathmatch'. Replaced with 'Target mode' and 'On/Off Mode' fields. |
| `dlls/doors.h` | 31 | `//#define SF_DOOR_SYNCHED				2048 //LRC- sends USE_ON/OFF when it starts to open/close (instead of sending` | sends USE_ON/OFF when it starts to open/close (instead of sending |

## 14. Button Enhancements

SoHL enhances buttons with locking/unlocking, master control, and MoveWith support.

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/buttons.cpp` | 35 | `#define SF_BUTTON_ONLYDIRECT	16  //LRC - button can't be used through walls.` | button can't be used through walls. |
| `dlls/buttons.cpp` | 139 | `//LRC- a simple entity, just maintains a state` | a simple entity, just maintains a state |
| `dlls/buttons.cpp` | 335 | `//LRC- the evil multisource...` | the evil multisource... |
| `dlls/buttons.cpp` | 408 | `// LRC- On could be meaningful. Off, sadly, can't work in the obvious manner.` | On could be meaningful. Off, sadly, can't work in the obvious manner. |
| `dlls/buttons.cpp` | 409 | `// LRC (09/06/01)- er... why not?` | (09/06/01)- er... why not? |
| `dlls/buttons.cpp` | 410 | `// LRC (28/04/02)- that depends what the "obvious" manner is.` | (28/04/02)- that depends what the "obvious" manner is. |
| `dlls/buttons.cpp` | 544 | `//LRC - moved here from cbase.h to use the spawnflags defined in this file` | moved here from cbase.h to use the spawnflags defined in this file |
| `dlls/buttons.cpp` | 785 | `//LRC` | LRC modification |
| `dlls/buttons.cpp` | 838 | `//LRC` | LRC modification |
| `dlls/buttons.cpp` | 975 | `//LRC - they had it set up so that a touch-only button couldn't even be triggered!?` | they had it set up so that a touch-only button couldn't even be triggered!? |
| `dlls/buttons.cpp` | 1062 | `//LRC - unhelpfully, SF_BUTTON_DONTMOVE is the same value as` | unhelpfully, SF_BUTTON_DONTMOVE is the same value as |
| `dlls/buttons.cpp` | 1092 | `//LRC` | LRC modification |
| `dlls/buttons.cpp` | 1137 | `//LRC` | LRC modification |
| `dlls/buttons.cpp` | 1185 | `// LRC- hmm... I see. On returning, a button will only turn off multisources.` | hmm... I see. On returning, a button will only turn off multisources. |
| `dlls/buttons.cpp` | 1434 | `if (IsLockedByMaster()) return; //LRC` | LRC modification |
| `dlls/buttons.cpp` | 1481 | `//LRC check if we're outside the boundaries` | check if we're outside the boundaries |
| `dlls/buttons.cpp` | 1500 | `//LRC- that is to say: our avelocity will get us to the target point in 0.1 secs.` | that is to say: our avelocity will get us to the target point in 0.1 secs. |
| `dlls/buttons.cpp` | 1591 | `STATE m_iState; //LRC` | LRC modification |
| `dlls/buttons.cpp` | 1599 | `DEFINE_FIELD(CEnvSpark, m_iState, FIELD_INTEGER), //LRC` | LRC modification |

## 15. Breakable Enhancements

SoHL enhances breakable entities with custom model support and improved breaking behavior.

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/func_break.cpp` | 118 | `else if (FStrEq(pkvd->szKeyName, "respawn")) //LRC` | LRC modification |
| `dlls/func_break.cpp` | 123 | `else if (FStrEq(pkvd->szKeyName, "whenhit")) //LRC` | LRC modification |
| `dlls/func_break.cpp` | 156 | `//LRC- time until respawn` | time until respawn |
| `dlls/func_break.cpp` | 158 | `//LRC- health to set on respawn` | health to set on respawn |
| `dlls/func_break.cpp` | 177 | `if (m_iClass) //LRC - might these additions cause problems?` | might these additions cause problems? |
| `dlls/func_break.cpp` | 183 | `if (m_iszWhenHit) //LRC - locus trigger` | locus trigger |
| `dlls/func_break.cpp` | 553 | `//LRC` | LRC modification |
| `dlls/func_break.cpp` | 572 | `//LRC` | LRC modification |
| `dlls/func_break.cpp` | 656 | `//LRC` | LRC modification |
| `dlls/func_break.cpp` | 711 | `// LRC - Die() does everything necessary` | Die() does everything necessary |
| `dlls/func_break.cpp` | 1090 | `if (pev->spawnflags & SF_PUSH_NOPULL) return; //LRC: a non-pullable pushable.` | a non-pullable pushable. |
| `dlls/func_break.h` | 92 | `//LRC` | LRC modification |

## 16. Light Enhancements

SoHL enhances light entities with pattern support and style modifications.

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/lights.cpp` | 27 | `//LRC` | LRC modification |
| `dlls/lights.cpp` | 113 | `int GetStyle(void) { return m_iszCurrentStyle; }; //LRC` | LRC modification |
| `dlls/lights.cpp` | 114 | `void SetStyle(int iszPattern); //LRC` | LRC modification |
| `dlls/lights.cpp` | 116 | `void SetCorrectStyle(void); //LRC` | LRC modification |
| `dlls/lights.cpp` | 493 | `//LRC- the CLightDynamic entity - works like the flashlight.` | the CLightDynamic entity - works like the flashlight. |
| `dlls/lights.cpp` | 571 | `//LRC- the CTriggerLightstyle entity - changes the style of a light temporarily.` | the CTriggerLightstyle entity - changes the style of a light temporarily. |

## 17. Sound Enhancements

SoHL modifies ambient_generic and sound playback with improved control and MoveWith support.

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/sound.cpp` | 141 | `edict_t* m_pPlayFrom; //LRC - the entity to play from` | the entity to play from |
| `dlls/sound.cpp` | 142 | `int m_iChannel; //LRC - the channel to play from, for "play from X" sounds` | the channel to play from, for "play from X" sounds |
| `dlls/sound.cpp` | 151 | `DEFINE_FIELD(CAmbientGeneric, m_iChannel, FIELD_INTEGER), //LRC` | LRC modification |
| `dlls/sound.cpp` | 152 | `DEFINE_FIELD(CAmbientGeneric, m_pPlayFrom, FIELD_EDICT), //LRC` | LRC modification |
| `dlls/sound.cpp` | 269 | `SetThink(&CAmbientGeneric ::StartPlayFrom); //LRC` | LRC modification |
| `dlls/sound.cpp` | 270 | `//			EMIT_SOUND_DYN( m_pPlayFrom, m_iChannel, szSoundFile, //LRC` | LRC modification |
| `dlls/sound.cpp` | 284 | `//LRC - for some reason, I can't get other entities to start playing sounds during Activate;` | for some reason, I can't get other entities to start playing sounds during Activate; |
| `dlls/sound.cpp` | 290 | `EMIT_SOUND_DYN(m_pPlayFrom, m_iChannel, szSoundFile, //LRC` | LRC modification |
| `dlls/sound.cpp` | 342 | `STOP_SOUND(m_pPlayFrom, m_iChannel, szSoundFile); //LRC` | LRC modification |
| `dlls/sound.cpp` | 391 | `STOP_SOUND(m_pPlayFrom, m_iChannel, szSoundFile); //LRC` | LRC modification |
| `dlls/sound.cpp` | 498 | `EMIT_SOUND_DYN(m_pPlayFrom, m_iChannel, szSoundFile, (vol * 0.01), //LRC` | LRC modification |
| `dlls/sound.cpp` | 710 | `STOP_SOUND(m_pPlayFrom, m_iChannel, szSoundFile); //LRC` | LRC modification |
| `dlls/sound.cpp` | 725 | `EMIT_SOUND_DYN(m_pPlayFrom, m_iChannel, szSoundFile, //LRC` | LRC modification |
| `dlls/sound.cpp` | 1096 | `//LRC - trigger_sound` | trigger_sound |

## 18. Effects Enhancements

SoHL extensively modifies beam, laser, sprite, and other effect entities with new features and MoveWith support.

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/bmodels.cpp` | 46 | `return (pevBModel->absmin + pevBModel->absmax) * 0.5; //LRC - bug fix for rotating ents` | bug fix for rotating ents |
| `dlls/bmodels.cpp` | 79 | `if (!m_pMoveWith) //LRC` | LRC modification |
| `dlls/bmodels.cpp` | 82 | `//LRC` | LRC modification |
| `dlls/bmodels.cpp` | 379 | `void EXPORT WaitForStart(); //LRC - get round 1.1.0.8's bizarre behaviour on startup` | get round 1.1.0.8's bizarre behaviour on startup |
| `dlls/bmodels.cpp` | 395 | `float m_fCurSpeed; //LRC - during spin-up and spin-down, this is` | during spin-up and spin-down, this is |
| `dlls/bmodels.cpp` | 400 | `STATE m_iState; //LRC` | LRC modification |
| `dlls/bmodels.cpp` | 475 | `m_fCurSpeed = 0; //LRC` | LRC modification |
| `dlls/bmodels.cpp` | 503 | `if (m_flFanFriction <= 0) //LRC - ensure it's not negative` | ensure it's not negative |
| `dlls/bmodels.cpp` | 548 | `//LRC - start immediately if unnamed, too.` | start immediately if unnamed, too. |
| `dlls/bmodels.cpp` | 656 | `pev->dmg = m_fCurSpeed / 10; //LRC` | LRC modification |
| `dlls/bmodels.cpp` | 795 | `if (m_fCurSpeed != 0) //LRC` | LRC modification |
| `dlls/bmodels.cpp` | 814 | `//LRC` | LRC modification |
| `dlls/bmodels.cpp` | 958 | `UTIL_SetAvelocity(this, m_maxSpeed * pev->movedir); //LRC` | LRC modification |
| `dlls/bmodels.cpp` | 967 | `UTIL_SetAvelocity(this, g_vecZero); //LRC` | LRC modification |
| `dlls/bmodels.cpp` | 982 | `UTIL_SetAngles(this, m_start); //LRC` | LRC modification |
| `dlls/bmodels.cpp` | 986 | `UTIL_SetAvelocity(this, g_vecZero); //LRC` | LRC modification |
| `dlls/bmodels.cpp` | 1016 | `UTIL_SetAvelocity(this, pev->speed * pev->movedir); //LRC` | LRC modification |
| `dlls/bmodels.cpp` | 1034 | `UTIL_SetAngles(this, m_center); //LRC` | LRC modification |
| `dlls/bmodels.cpp` | 1039 | `UTIL_SetAvelocity(this, g_vecZero); //LRC` | LRC modification |
| `dlls/effects.cpp` | 25 | `#include "player.h" //LRC - footstep stuff` | footstep stuff |
| `dlls/effects.cpp` | 26 | `#include "locus.h" //LRC - locus utilities` | locus utilities |
| `dlls/effects.cpp` | 27 | `#include "movewith.h" //LRC - the DesiredThink system` | the DesiredThink system |
| `dlls/effects.cpp` | 35 | `//LRC - make info_target an entity class in its own right` | make info_target an entity class in its own right |
| `dlls/effects.cpp` | 45 | `//LRC- force an info_target to use the sprite null.spr` | force an info_target to use the sprite null.spr |
| `dlls/effects.cpp` | 433 | `void BeamUpdatePoints(void); //LRC` | LRC modification |
| `dlls/effects.cpp` | 508 | `//LRC- a convenience for mappers. Will this mess anything up?` | a convenience for mappers. Will this mess anything up? |
| `dlls/effects.cpp` | 639 | `SUB_UseTargets(this, USE_OFF, 0); //LRC` | LRC modification |
| `dlls/effects.cpp` | 646 | `SUB_UseTargets(this, USE_ON, 0); //LRC` | LRC modification |
| `dlls/effects.cpp` | 684 | `//LRC- follow (almost) any entity that has a model` | follow (almost) any entity that has a model |
| `dlls/effects.cpp` | 695 | `if (m_life != 0 && m_restrike != -1) //LRC non-restriking beams! what an idea!` | non-restriking beams! what an idea! |
| `dlls/effects.cpp` | 731 | `//LRC- FIXME: tell the user there's a problem.` | FIXME: tell the user there's a problem. |
| `dlls/effects.cpp` | 795 | `//LRC - tripbeams` | tripbeams |
| `dlls/effects.cpp` | 852 | `//LRC - beams that heal people` | beams that heal people |
| `dlls/effects.cpp` | 860 | `//LRC - used to be DamageThink, but now it's more general.` | used to be DamageThink, but now it's more general. |
| `dlls/effects.cpp` | 874 | `//LRC - tripbeams` | tripbeams |
| `dlls/effects.cpp` | 1006 | `// LRC: Called whenever the beam gets turned on, in case an alias changed or one of the points has moved.` | Called whenever the beam gets turned on, in case an alias changed or one of the points has moved. |
| `dlls/effects.cpp` | 1065 | `BeamUpdatePoints(); //LRC` | LRC modification |
| `dlls/effects.cpp` | 1114 | `//LRC: allow the spritename to be the name of an env_sprite` | allow the spritename to be the name of an env_sprite |
| `dlls/effects.cpp` | 1174 | `//LRC` | LRC modification |
| `dlls/effects.cpp` | 1300 | `UTIL_SetVelocity(m_pStartSprite, g_vecZero); //LRC` | LRC modification |
| `dlls/effects.cpp` | 1305 | `UTIL_SetVelocity(m_pEndSprite, g_vecZero); //LRC` | LRC modification |
| `dlls/effects.cpp` | 1391 | `//LRC` | LRC modification |
| `dlls/effects.cpp` | 1419 | `//LRC - tripbeams` | tripbeams |
| `dlls/effects.cpp` | 1675 | `SUB_UseTargets(this, USE_OFF, 0); //LRC` | LRC modification |
| `dlls/effects.cpp` | 1680 | `SUB_UseTargets(this, USE_ON, 0); //LRC` | LRC modification |
| `dlls/effects.cpp` | 1763 | `UTIL_SetSize(pev, Vector(-10, -10, -10), Vector(10, 10, 10)); //LRCT` | T |
| `dlls/effects.cpp` | 1892 | `#define SF_GIBSHOOTER_DEBUG			4 //LRC` | LRC modification |
| `dlls/effects.cpp` | 2104 | `if (m_flDelay == 0) // LRC - delay is 0, fire them all at once.` | delay is 0, fire them all at once. |
| `dlls/effects.cpp` | 2547 | `Vector Direction(CBaseEntity* pActivator); //LRC - added pActivator, for locus system` | added pActivator, for locus system |
| `dlls/effects.cpp` | 2675 | `STATE m_iState; //LRC` | LRC modification |
| `dlls/effects.cpp` | 2740 | `SetNextThink(Duration()); //LRC` | LRC modification |
| `dlls/effects.cpp` | 2752 | `void Think(void); //LRC` | LRC modification |
| `dlls/effects.cpp` | 2760 | `STATE m_iState; // LRC. Don't saverestore this value, it's not worth it.` | . Don't saverestore this value, it's not worth it. |
| `dlls/effects.cpp` | 2771 | `#define SF_FADE_PERMANENT		0x0008		//LRC - hold permanently` | hold permanently |
| `dlls/effects.cpp` | 2806 | `SetNextThink(Duration()); //LRC` | LRC modification |
| `dlls/effects.cpp` | 2814 | `if (pev->spawnflags & SF_FADE_PERMANENT) //LRC` | LRC modification |
| `dlls/effects.cpp` | 2815 | `fadeFlags \|= FFADE_STAYOUT; //LRC` | LRC modification |
| `dlls/effects.cpp` | 2852 | `//LRC: a bolt-on state!` | a bolt-on state! |
| `dlls/effects.cpp` | 2984 | `//LRC` | LRC modification |
| `dlls/effects.cpp` | 2995 | `//LRC` | LRC modification |
| `dlls/effects.cpp` | 3037 | `// LRC -  All the particle effects from Quake 1` | All the particle effects from Quake 1 |
| `dlls/effects.cpp` | 3086 | `// LRC - Beam Trail effect` | Beam Trail effect |
| `dlls/effects.cpp` | 3199 | `// LRC -  custom footstep sounds` | custom footstep sounds |
| `dlls/effects.cpp` | 3361 | `//LRC- the long-awaited effect. (Rain, in the desert? :)` | the long-awaited effect. (Rain, in the desert? :) |
| `dlls/effects.cpp` | 3667 | `//LRC- Xen monsters' warp-in effect, for those too lazy to build it. :)` | Xen monsters' warp-in effect, for those too lazy to build it. :) |
| `dlls/effects.cpp` | 3738 | `//LRC- Shockwave effect, like when a Houndeye attacks.` | Shockwave effect, like when a Houndeye attacks. |
| `dlls/effects.cpp` | 3805 | `m_iHeight = atoi(pkvd->szValue) / 2; //LRC- the actual height is doubled when drawn` | the actual height is doubled when drawn |
| `dlls/effects.cpp` | 4022 | `//LRC- env_elight; Dynamic Entity Light creator` | env_elight; Dynamic Entity Light creator |
| `dlls/effects.cpp` | 4090 | `// LRC - Decal effect` | Decal effect |
| `dlls/effects.cpp` | 4603 | `// LRC - env_particle, uses the aurora particle system` | env_particle, uses the aurora particle system |
| `dlls/effects.h` | 29 | `//LRC - tripbeams` | tripbeams |
| `dlls/effects.h` | 31 | `//LRC - smoother lasers` | smoother lasers |
| `dlls/effects.h` | 185 | `CBaseEntity* GetTripEntity(TraceResult* ptr); //LRC` | LRC modification |

## 19. Model/Animation

SoHL adds model scaling support and animation helper functions.

| File | Line | Code | Description |
|------|------|------|-------------|
| `cl_dll/StudioModelRenderer.cpp` | 559 | `//LRC - apply scale to models!` | apply scale to models! |
| `cl_dll/StudioModelRenderer.cpp` | 1284 | `//LRC` | LRC modification |
| `dlls/animation.cpp` | 527 | `//LRC` | LRC modification |
| `dlls/animation.cpp` | 546 | `//LRC` | LRC modification |
| `dlls/animation.h` | 40 | `//LRC` | LRC modification |
| `dlls/animation.h` | 43 | `int GetSequenceFrames(void* pmodel, entvars_t* pev); //LRC` | LRC modification |

## 20. Particle System

SoHL adds a client-side particle system manager with time-based updates.

| File | Line | Code | Description |
|------|------|------|-------------|
| `cl_dll/particlemgr.cpp` | 107 | `void ParticleSystemManager::UpdateSystems(float frametime) //LRC - now with added time!` | now with added time! |

## 21. Scripted Sequence Enhancements

SoHL significantly enhances scripted_sequence with new turn types, trigger targets, idle/play interrupt control, and MoveWith integration.

| File | Line | Code | Description |
|------|------|------|-------------|
| `dlls/scripted.cpp` | 106 | `// LRC	else if (FStrEq(pkvd->szKeyName, "m_flRepeat"))` | else if (FStrEq(pkvd->szKeyName, "m_flRepeat")) |
| `dlls/scripted.cpp` | 145 | `DEFINE_FIELD(CCineMonster, m_iState, FIELD_INTEGER), //LRC` | LRC modification |
| `dlls/scripted.cpp` | 149 | `DEFINE_FIELD(CCineMonster, m_iszAttack, FIELD_STRING), //LRC` | LRC modification |
| `dlls/scripted.cpp` | 150 | `DEFINE_FIELD(CCineMonster, m_iszMoveTarget, FIELD_STRING), //LRC` | LRC modification |
| `dlls/scripted.cpp` | 155 | `//LRC- this is unused	DEFINE_FIELD( CCineMonster, m_flRepeat, FIELD_FLOAT ),` | this is unused	DEFINE_FIELD( CCineMonster, m_flRepeat, FIELD_FLOAT ), |
| `dlls/scripted.cpp` | 167 | `//LRC` | LRC modification |
| `dlls/scripted.cpp` | 178 | `LINK_ENTITY_TO_CLASS(scripted_action, CCineMonster); //LRC` | LRC modification |
| `dlls/scripted.cpp` | 180 | `LINK_ENTITY_TO_CLASS(aiscripted_sequence, CCineMonster); //LRC - aiscripted sequences don't need to be seperate` | aiscripted sequences don't need to be seperate |
| `dlls/scripted.cpp` | 206 | `//LRC - the only difference between AI and normal sequences` | the only difference between AI and normal sequences |
| `dlls/scripted.cpp` | 241 | `CineThink(); //LRC` | LRC modification |
| `dlls/scripted.cpp` | 308 | `//LRC: now redefined... find a viable entity with the given name, and return it (or NULL if not found).` | now redefined... find a viable entity with the given name, and return it (or NULL if not found). |
| `dlls/scripted.cpp` | 545 | `m_iRepeatsLeft = m_iRepeats; //LRC - reset the repeater count` | reset the repeater count |
| `dlls/scripted.cpp` | 574 | `// or select a specific AMBUSH schedule, regardless of state. //LRC` | LRC modification |
| `dlls/scripted.cpp` | 668 | `//LRC - clean up so that if another script is starting immediately, the monster will notice it.` | clean up so that if another script is starting immediately, the monster will notice it. |
| `dlls/scripted.cpp` | 719 | `FireTargets(STRING(m_iszFireOnBegin), this, this, USE_TOGGLE, 0); //LRC` | LRC modification |
| `dlls/scripted.cpp` | 788 | `// LRC - why mess around with this? Solidity isn't changed by sequences!` | why mess around with this? Solidity isn't changed by sequences! |
| `dlls/scripted.cpp` | 900 | `//LRC- removed, was never implemented. ClearBits(pev->spawnflags, SF_MONSTER_WAIT_FOR_SCRIPT );` | removed, was never implemented. ClearBits(pev->spawnflags, SF_MONSTER_WAIT_FOR_SCRIPT ); |
| `dlls/scripted.cpp` | 1029 | `m_playing = FALSE; //LRC` | LRC modification |
| `dlls/scripted.cpp` | 1066 | `if (!m_iszEntity) //LRC- no target monster given: speak through HEV` | no target monster given: speak through HEV |
| `dlls/scripted.cpp` | 1106 | `//LRC` | LRC modification |
| `dlls/scripted.cpp` | 1191 | `//LRC: Er... if the "concurrent" flag is NOT set, we make bConcurrent true!?` | Er... if the "concurrent" flag is NOT set, we make bConcurrent true!? |
| `dlls/scripted.h` | 30 | `#define SF_SCRIPT_STAYDEAD			256 // LRC- signifies that the animation kills the monster` | signifies that the animation kills the monster |
| `dlls/scripted.h` | 35 | `//LRC - rearranged into flags` | rearranged into flags |
| `dlls/scripted.h` | 63 | `//LRC: states for script entities` | states for script entities |
| `dlls/scripted.h` | 69 | `void EXPORT InitIdleThink(void); //LRC` | LRC modification |
| `dlls/scripted.h` | 76 | `inline BOOL IsAction(void) { return FClassnameIs(pev, "scripted_action"); }; //LRC` | LRC modification |
| `dlls/scripted.h` | 78 | `//LRC: Should the monster do a precise attack for this scripted_action?` | Should the monster do a precise attack for this scripted_action? |
| `dlls/scripted.h` | 111 | `//LRC- this does nothing!!	float m_flRepeat;	// repeat rate` | this does nothing!!	float m_flRepeat;	// repeat rate |
| `dlls/scripted.h` | 112 | `int m_iRepeats; //LRC - number of times to repeat the animation` | number of times to repeat the animation |
| `dlls/scripted.h` | 113 | `int m_iRepeatsLeft; //LRC` | LRC modification |
| `dlls/scripted.h` | 114 | `float m_fRepeatFrame; //LRC` | LRC modification |
| `dlls/scripted.h` | 115 | `int m_iPriority; //LRC` | LRC modification |
| `dlls/scripted.h` | 127 | `//LRC - removed CCineAI, obsolete` | removed CCineAI, obsolete |

## 22. New Entity Definitions

SoHL adds new capability flags, classification constants, and content type definitions.

| File | Line | Code | Description |
|------|------|------|-------------|
| `common/const.h` | 328 | `//LRC - ignore this, they're lying // byte (brightness)` | ignore this, they're lying // byte (brightness) |
| `common/const.h` | 602 | `//LRC- New (long overdue) content types for Spirit` | New (long overdue) content types for Spirit |
| `common/const.h` | 606 | `#define CONTENT_SPECIAL1			-20 //LRC - used by the particle systems` | used by the particle systems |
| `dlls/cbase.h` | 50 | `#define		FCAP_ONLYDIRECT_USE			0x00000100		//LRC - can't use this entity through a wall.` | can't use this entity through a wall. |
| `dlls/cbase.h` | 138 | `#define CLASS_FACTION_A			14 //LRC - very simple new classes, for use with Behaves As` | very simple new classes, for use with Behaves As |

## 23. Other/Miscellaneous Changes

Additional modifications that span various subsystems.

| File | Line | Code | Description |
|------|------|------|-------------|
| `cl_dll/ammo.cpp` | 541 | `//LRCT - experiment to allow a custom crosshair.` | T - experiment to allow a custom crosshair. |
| `cl_dll/ammo.cpp` | 591 | `m_pWeapon = NULL; //LRC` | LRC modification |
| `cl_dll/ammo.cpp` | 623 | `//LRCT - probably not the right way to do this...` | T - probably not the right way to do this... |
| `cl_dll/cl_dll.h` | 43 | `#define CONPRINT (gEngfuncs.Con_Printf) //LRC - I can't live without printf!` | I can't live without printf! |
| `cl_dll/health.cpp` | 228 | `//LRC FillRGBA(x, y, iWidth, iHeight, 255, 160, 0, a);` | FillRGBA(x, y, iWidth, iHeight, 255, 160, 0, a); |
| `cl_dll/health.cpp` | 229 | `FillRGBA(x, y, iWidth, iHeight, r, g, b, a); //LRC` | LRC modification |
| `cl_dll/hl/hl_baseentity.cpp` | 82 | `} //LRC` | LRC modification |
| `cl_dll/hl/hl_baseentity.cpp` | 85 | `} //LRC` | LRC modification |
| `cl_dll/hl/hl_baseentity.cpp` | 88 | `} //LRC` | LRC modification |
| `cl_dll/hl/hl_baseentity.cpp` | 91 | `} //LRC` | LRC modification |
| `cl_dll/hl/hl_baseentity.cpp` | 94 | `} //LRC` | LRC modification |
| `cl_dll/hl/hl_baseentity.cpp` | 471 | `int CBaseMonster::CanPlaySequence(int interruptLevel) { return FALSE; } //LRC - prototype changed` | prototype changed |
| `cl_dll/hl/hl_weapons.cpp` | 253 | `//LRC` | LRC modification |
| `cl_dll/hud.cpp` | 97 | `//LRC` | LRC modification |
| `cl_dll/hud.cpp` | 103 | `//LRC` | LRC modification |
| `cl_dll/hud.cpp` | 110 | `//LRC` | LRC modification |
| `cl_dll/hud.cpp` | 117 | `//LRC` | LRC modification |
| `cl_dll/hud.cpp` | 124 | `//LRC` | LRC modification |
| `cl_dll/hud.cpp` | 612 | `//LRC` | LRC modification |
| `cl_dll/hud.h` | 562 | `//LRC` | LRC modification |
| `cl_dll/hud_msg.cpp` | 125 | `//LRC` | LRC modification |
| `cl_dll/hud_msg.cpp` | 158 | `//LRC` | LRC modification |
| `cl_dll/hud_msg.cpp` | 193 | `//LRC` | LRC modification |
| `cl_dll/hud_msg.cpp` | 215 | `//LRC` | LRC modification |
| `cl_dll/tri.cpp` | 128 | `//LRCT` | T |
| `cl_dll/tri.cpp` | 157 | `//LRCT		sprintf( sz, "sprites/cursor.spr" );` | T		sprintf( sz, "sprites/cursor.spr" ); |
| `cl_dll/tri.cpp` | 158 | `sprintf( sz, "sprites/bubble.spr" ); //LRCT` | T |
| `cl_dll/tri.cpp` | 249 | `// LRC: find out the time elapsed since the last redraw` | find out the time elapsed since the last redraw |
| `cl_dll/tri.cpp` | 254 | `// LRC: draw and update particle systems` | draw and update particle systems |
| `cl_dll/view.cpp` | 507 | `//LRC - if this is the second pass through, then we've just drawn the sky, and now we're setting up the normal view.` | if this is the second pass through, then we've just drawn the sky, and now we're setting up the normal view. |
| `cl_dll/view.cpp` | 537 | `//LRC - don't show weapon models when we're drawing the sky.` | don't show weapon models when we're drawing the sky. |
| `cl_dll/view.cpp` | 860 | `//LRC` | LRC modification |
| `cl_dll/view.cpp` | 863 | `// LRC - override the view position if we're drawing a sky, rather than the player's view` | override the view position if we're drawing a sky, rather than the player's view |
| `dlls/cbase.cpp` | 156 | `return -1; //LRC` | LRC modification |
| `dlls/cbase.cpp` | 158 | `return -1; //LRC` | LRC modification |
| `dlls/cbase.cpp` | 160 | `return -1; //LRC` | LRC modification |
| `dlls/cbase.cpp` | 275 | `//LRC - rearranged so that we can correct m_fNextThink too.` | rearranged so that we can correct m_fNextThink too. |
| `dlls/cbase.cpp` | 507 | `//LRC` | LRC modification |
| `dlls/cbase.cpp` | 510 | `//LRC - rebuild the new assistlist as the game starts` | rebuild the new assistlist as the game starts |
| `dlls/cbase.cpp` | 516 | `//LRC - and the aliaslist too` | and the aliaslist too |
| `dlls/cbase.cpp` | 528 | `//LRC- called by activate() to support movewith` | called by activate() to support movewith |
| `dlls/cbase.cpp` | 586 | `//LRC` | LRC modification |
| `dlls/cbase.cpp` | 599 | `//LRC` | LRC modification |
| `dlls/cbase.cpp` | 618 | `//LRC - for getting round the engine's preconceptions.` | for getting round the engine's preconceptions. |
| `dlls/cbase.cpp` | 654 | `//LRC` | LRC modification |
| `dlls/cbase.cpp` | 672 | `//LRC - check in case the engine has changed our nextthink. (which it does` | check in case the engine has changed our nextthink. (which it does |
| `dlls/cbase.cpp` | 784 | `DEFINE_FIELD(CBaseEntity, m_MoveWith, FIELD_STRING), //LRC` | LRC modification |
| `dlls/cbase.cpp` | 785 | `DEFINE_FIELD(CBaseEntity, m_pMoveWith, FIELD_CLASSPTR), //LRC` | LRC modification |
| `dlls/cbase.cpp` | 786 | `DEFINE_FIELD(CBaseEntity, m_pChildMoveWith, FIELD_CLASSPTR), //LRC` | LRC modification |
| `dlls/cbase.cpp` | 787 | `DEFINE_FIELD(CBaseEntity, m_pSiblingMoveWith, FIELD_CLASSPTR), //LRC` | LRC modification |
| `dlls/cbase.cpp` | 789 | `DEFINE_FIELD(CBaseEntity, m_iLFlags, FIELD_INTEGER), //LRC` | LRC modification |
| `dlls/cbase.cpp` | 790 | `DEFINE_FIELD(CBaseEntity, m_iStyle, FIELD_INTEGER), //LRC` | LRC modification |
| `dlls/cbase.cpp` | 791 | `DEFINE_FIELD(CBaseEntity, m_vecMoveWithOffset, FIELD_VECTOR), //LRC` | LRC modification |
| `dlls/cbase.cpp` | 792 | `DEFINE_FIELD(CBaseEntity, m_vecRotWithOffset, FIELD_VECTOR), //LRC` | LRC modification |
| `dlls/cbase.cpp` | 793 | `DEFINE_FIELD(CBaseEntity, m_activated, FIELD_BOOLEAN), //LRC` | LRC modification |
| `dlls/cbase.cpp` | 794 | `DEFINE_FIELD(CBaseEntity, m_fNextThink, FIELD_TIME), //LRC` | LRC modification |
| `dlls/cbase.cpp` | 795 | `DEFINE_FIELD(CBaseEntity, m_fPevNextThink, FIELD_TIME), //LRC` | LRC modification |
| `dlls/cbase.cpp` | 796 | `//	DEFINE_FIELD( CBaseEntity, m_pAssistLink, FIELD_CLASSPTR ), //LRC - don't save this, we'll just rebuild the list o...` | don't save this, we'll just rebuild the list on restore |
| `dlls/cbase.cpp` | 797 | `DEFINE_FIELD(CBaseEntity, m_vecPostAssistVel, FIELD_VECTOR), //LRC` | LRC modification |
| `dlls/cbase.cpp` | 798 | `DEFINE_FIELD(CBaseEntity, m_vecPostAssistAVel, FIELD_VECTOR), //LRC` | LRC modification |
| `dlls/cbase.cpp` | 809 | `ThinkCorrection(); //LRC` | LRC modification |
| `dlls/cbase.h` | 49 | `// LRC: no longer used` | no longer used |
| `dlls/cbase.h` | 99 | `//extern CBaseEntity *g_pDesiredList; //LRC- handles DesiredVel, for movewith` | handles DesiredVel, for movewith |
| `dlls/cbase.h` | 101 | `//LRC- added USE_SAME, USE_NOT, and USE_KILL` | added USE_SAME, USE_NOT, and USE_KILL |
| `dlls/cbase.h` | 196 | `CBaseEntity* m_pMoveWith; // LRC- the entity I move with.` | the entity I move with. |
| `dlls/cbase.h` | 197 | `int m_MoveWith; //LRC- Name of that entity` | Name of that entity |
| `dlls/cbase.h` | 198 | `CBaseEntity* m_pChildMoveWith; //LRC- one of the entities that's moving with me.` | one of the entities that's moving with me. |
| `dlls/cbase.h` | 199 | `CBaseEntity* m_pSiblingMoveWith; //LRC- another entity that's Moving With the same ent as me. (linked list.)` | another entity that's Moving With the same ent as me. (linked list.) |
| `dlls/cbase.h` | 200 | `Vector m_vecMoveWithOffset; // LRC- Position I should be in relative to m_pMoveWith->pev->origin.` | Position I should be in relative to m_pMoveWith->pev->origin. |
| `dlls/cbase.h` | 201 | `Vector m_vecRotWithOffset; // LRC- Angles I should be facing relative to m_pMoveWith->pev->angles.` | Angles I should be facing relative to m_pMoveWith->pev->angles. |
| `dlls/cbase.h` | 202 | `CBaseEntity* m_pAssistLink; // LRC- link to the next entity which needs to be Assisted before physics are applied.` | link to the next entity which needs to be Assisted before physics are applied. |
| `dlls/cbase.h` | 203 | `Vector m_vecPostAssistVel; // LRC` | LRC modification |
| `dlls/cbase.h` | 204 | `Vector m_vecPostAssistAVel; // LRC` | LRC modification |
| `dlls/cbase.h` | 206 | `// LRC - for SetNextThink and SetPhysThink. Marks the time when a think will be performed - not necessarily the same ...` | for SetNextThink and SetPhysThink. Marks the time when a think will be performed - not necessarily the same as pev->nextthink! |
| `dlls/cbase.h` | 208 | `// LRC - always set equal to pev->nextthink, so that we can tell when the latter gets changed by the @#$^! engine.` | always set equal to pev->nextthink, so that we can tell when the latter gets changed by the @#$^! engine. |
| `dlls/cbase.h` | 209 | `int m_iLFlags; // LRC- a new set of flags. (pev->spawnflags and pev->flags are full...)` | a new set of flags. (pev->spawnflags and pev->flags are full...) |
| `dlls/cbase.h` | 212 | `}; // LRC - for postponing stuff until PostThink time, not as a think.` | for postponing stuff until PostThink time, not as a think. |
| `dlls/cbase.h` | 213 | `int m_iStyle; // LRC - almost anything can have a lightstyle these days...` | almost anything can have a lightstyle these days... |
| `dlls/cbase.h` | 215 | `Vector m_vecSpawnOffset; // LRC- To fix things which (for example) MoveWith a door which Starts Open.` | To fix things which (for example) MoveWith a door which Starts Open. |
| `dlls/cbase.h` | 216 | `BOOL m_activated; // LRC- moved here from func_train. Signifies that an entity has already been` | moved here from func_train. Signifies that an entity has already been |
| `dlls/cbase.h` | 219 | `//LRC - decent mechanisms for setting think times!` | decent mechanisms for setting think times! |
| `dlls/cbase.h` | 241 | `//LRC use this instead of "SetThink( NULL )" or "pev->nextthink = -1".` | use this instead of "SetThink( NULL )" or "pev->nextthink = -1". |
| `dlls/cbase.h` | 243 | `//LRC similar, but called by the parent when a think needs to be aborted.` | similar, but called by the parent when a think needs to be aborted. |
| `dlls/cbase.h` | 248 | `//LRC - loci` | loci |
| `dlls/cbase.h` | 253 | `//LRC - aliases` | aliases |
| `dlls/cbase.h` | 262 | `//LRC - MoveWith for all!` | MoveWith for all! |
| `dlls/cbase.h` | 283 | `//LRC - if I MoveWith something, then only cross transitions if the MoveWith entity does too.` | if I MoveWith something, then only cross transitions if the MoveWith entity does too. |
| `dlls/cbase.h` | 289 | `virtual void Activate(void); //LRC` | LRC modification |
| `dlls/cbase.h` | 290 | `void InitMoveWith(void); //LRC - called by Activate() to set up moveWith values` | called by Activate() to set up moveWith values |
| `dlls/cbase.h` | 293 | `} //LRC - called by Activate() to handle entity-specific initialisation.` | called by Activate() to handle entity-specific initialisation. |
| `dlls/cbase.h` | 308 | `// LRC- this supports a global concept of "entities with states", so that state_watchers and` | this supports a global concept of "entities with states", so that state_watchers and |
| `dlls/cbase.h` | 554 | `//LRC- moved here from player.cpp. I'd put it in util.h with its friends, but it needs CBaseEntity to be declared.` | moved here from player.cpp. I'd put it in util.h with its friends, but it needs CBaseEntity to be declared. |
| `dlls/cbase.h` | 641 | `EHANDLE m_hActivator; //LRC - moved here from CBaseToggle` | moved here from CBaseToggle |
| `dlls/cbase.h` | 682 | `//LRC` | LRC modification |
| `dlls/cbase.h` | 725 | `float m_flLinearMoveSpeed; // LRC- allows a LinearMove to be delayed until a think.` | allows a LinearMove to be delayed until a think. |
| `dlls/cbase.h` | 726 | `float m_flAngularMoveSpeed; // LRC` | LRC modification |
| `dlls/cbase.h` | 738 | `// LRC- overridden because toggling entities have general rules governing their states.` | overridden because toggling entities have general rules governing their states. |
| `dlls/cbase.h` | 746 | `void EXPORT LinearMoveNow(void); //LRC- think function that lets us guarantee a LinearMove gets done as a think.` | think function that lets us guarantee a LinearMove gets done as a think. |
| `dlls/cbase.h` | 748 | `void EXPORT LinearMoveDoneNow(void); //LRC` | LRC modification |
| `dlls/cbase.h` | 751 | `void EXPORT AngularMoveNow(void); //LRC- think function that lets us guarantee an AngularMove gets done as a think.` | think function that lets us guarantee an AngularMove gets done as a think. |
| `dlls/cbase.h` | 892 | `virtual void PostSpawn(void); //LRC` | LRC modification |
| `dlls/cbase.h` | 998 | `//LRC- much as I hate to add new globals, I can't see how to read data from the World entity.` | much as I hate to add new globals, I can't see how to read data from the World entity. |
| `dlls/cbase.h` | 1001 | `//LRC- moved here from alias.cpp so that util functions can use these defs.` | moved here from alias.cpp so that util functions can use these defs. |
| `dlls/cdll_dll.h` | 34 | `#define HIDEHUD_CUSTOMCROSSHAIR ( 1<<4 ) //LRC - probably not the right way to do this, but it's just an experiment.` | probably not the right way to do this, but it's just an experiment. |
| `dlls/client.cpp` | 529 | `else if (FStrEq(pcmd, "fire")) //LRC - trigger entities manually` | trigger entities manually |
| `dlls/client.cpp` | 829 | `//LRC - moved to here from CBasePlayer::PostThink, so that` | moved to here from CBasePlayer::PostThink, so that |
| `dlls/client.cpp` | 864 | `//	CheckDesiredList(); //LRC` | LRC modification |
| `dlls/client.cpp` | 865 | `CheckAssistList(); //LRC` | LRC modification |
| `dlls/crowbar.cpp` | 278 | `m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.25; //LRC: corrected half-life bug` | corrected half-life bug |
| `dlls/explode.cpp` | 160 | `//LRC` | LRC modification |
| `dlls/explode.cpp` | 179 | `pev->origin = vecSpot; //LRC` | LRC modification |
| `dlls/game.cpp` | 44 | `cvar_t impulsetarget = {"sohl_impulsetarget", "0", FCVAR_SERVER}; //LRC - trigger ents manually` | trigger ents manually |
| `dlls/game.cpp` | 46 | `//LRC - debug info. for MoveWith. (probably not useful for most people.)` | debug info. for MoveWith. (probably not useful for most people.) |
| `dlls/game.cpp` | 487 | `CVAR_REGISTER(&impulsetarget); //LRC` | LRC modification |
| `dlls/game.cpp` | 488 | `CVAR_REGISTER(&mw_debug); //LRC` | LRC modification |
| `dlls/gamerules.cpp` | 74 | `//LRC` | LRC modification |
| `dlls/gamerules.h` | 19 | `//LRC` | LRC modification |
| `dlls/h_battery.cpp` | 98 | `//LRC` | LRC modification |
| `dlls/h_battery.cpp` | 123 | `//LRC` | LRC modification |
| `dlls/h_battery.cpp` | 193 | `//LRC` | LRC modification |
| `dlls/healthkit.cpp` | 170 | `//LRC` | LRC modification |
| `dlls/healthkit.cpp` | 198 | `//LRC` | LRC modification |
| `dlls/healthkit.cpp` | 254 | `//LRC` | LRC modification |
| `dlls/items.cpp` | 219 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/items.cpp` | 228 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/items.cpp` | 233 | `PRECACHE_SOUND((char*)STRING(pev->noise)); //LRC` | LRC modification |
| `dlls/items.cpp` | 259 | `EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, STRING(pev->noise), 1, ATTN_NORM); //LRC` | LRC modification |
| `dlls/maprules.cpp` | 359 | `//LRC` | LRC modification |
| `dlls/maprules.cpp` | 608 | `if (pev->origin == g_vecZero) //LRC - to support movewith` | to support movewith |
| `dlls/movewith.cpp` | 8 | `CWorld* g_pWorld = NULL; //LRC` | LRC modification |
| `dlls/movewith.cpp` | 10 | `BOOL g_doingDesired = FALSE; //LRC - marks whether the Desired functions are currently` | marks whether the Desired functions are currently |
| `dlls/movewith.cpp` | 423 | `// LRC- change the origin to the given position, and bring any movewiths along too.` | change the origin to the given position, and bring any movewiths along too. |
| `dlls/movewith.cpp` | 429 | `// LRC- bInitiator is true if this is being called directly, rather than because pEntity is moving with something else.` | bInitiator is true if this is being called directly, rather than because pEntity is moving with something else. |
| `dlls/movewith.cpp` | 528 | `//LRC- an arbitrary limit. If this number is exceeded we assume there's an infinite loop, and abort.` | an arbitrary limit. If this number is exceeded we assume there's an infinite loop, and abort. |
| `dlls/movewith.cpp` | 531 | `//LRC- for use in supporting movewith. Tell the entity that whatever it's moving with is about to change velocity.` | for use in supporting movewith. Tell the entity that whatever it's moving with is about to change velocity. |
| `dlls/movewith.cpp` | 567 | `//LRC- velocity is ignored on entities that aren't thinking! (Aargh...)` | velocity is ignored on entities that aren't thinking! (Aargh...) |
| `dlls/movewith.cpp` | 601 | `//LRC` | LRC modification |
| `dlls/movewith.cpp` | 619 | `int sloopbreaker = MAX_MOVEWITH_DEPTH; // LRC - to save us from infinite loops` | to save us from infinite loops |
| `dlls/movewith.cpp` | 636 | `//LRC - one more MoveWith utility. This one's for the simple version of RotWith.` | one more MoveWith utility. This one's for the simple version of RotWith. |
| `dlls/movewith.cpp` | 689 | `int sloopbreaker = MAX_MOVEWITH_DEPTH; // LRC - to save us from infinite loops` | to save us from infinite loops |
| `dlls/pathcorner.cpp` | 61 | `else if (FStrEq(pkvd->szKeyName, "turnspeed")) //LRC` | LRC modification |
| `dlls/pathcorner.cpp` | 151 | `else if (FStrEq(pkvd->szKeyName, "turnspeed")) //LRC` | LRC modification |
| `dlls/player.cpp` | 39 | `#include "effects.h" //LRC` | LRC modification |
| `dlls/player.cpp` | 40 | `#include "movewith.h" //LRC` | LRC modification |
| `dlls/player.cpp` | 48 | `BOOL g_markFrameBounds = 0; //LRC` | LRC modification |
| `dlls/player.cpp` | 168 | `int gmsgParticle = 0; // LRC` | LRC modification |
| `dlls/player.cpp` | 197 | `int gmsgStatusIcon = 0; //LRC` | LRC modification |
| `dlls/player.cpp` | 233 | `gmsgParticle = REG_USER_MSG("Particle", -1); //LRC` | LRC modification |
| `dlls/player.cpp` | 389 | `//LRC- if no suit, then no flatline sound. (unless it's a deathmatch.)` | if no suit, then no flatline sound. (unless it's a deathmatch.) |
| `dlls/player.cpp` | 882 | `//LRC` | LRC modification |
| `dlls/player.cpp` | 912 | `//LRC` | LRC modification |
| `dlls/player.cpp` | 1710 | `//LRC- try to get an exact entity to use.` | try to get an exact entity to use. |
| `dlls/player.cpp` | 1724 | `if (!pObject) //LRC- couldn't find a direct solid object to use, try the normal method` | couldn't find a direct solid object to use, try the normal method |
| `dlls/player.cpp` | 1730 | `//LRC - we can't see 'direct use' entities in this section` | we can't see 'direct use' entities in this section |
| `dlls/player.cpp` | 1737 | `//				ALERT(at_console, "absmin %f %f %f, absmax %f %f %f, mins %f %f %f, maxs %f %f %f, size %f %f %f\n", pObject->p...` | TEMP |
| `dlls/player.cpp` | 1744 | `// LRC - if the player is standing inside this entity, it's also ok to use it.` | if the player is standing inside this entity, it's also ok to use it. |
| `dlls/player.cpp` | 2774 | `//LRC - This is now handled with the Think function, by TrackTarget` | This is now handled with the Think function, by TrackTarget |
| `dlls/player.cpp` | 2952 | `//LRC- moved to cbase.h` | moved to cbase.h |
| `dlls/player.cpp` | 3740 | `case 90: //LRC - send USE_TOGGLE` | send USE_TOGGLE |
| `dlls/player.cpp` | 3747 | `case 91: //LRC - send USE_ON` | send USE_ON |
| `dlls/player.cpp` | 3754 | `case 92: //LRC - send USE_OFF` | send USE_OFF |
| `dlls/player.h` | 351 | `extern int gmsgParticle; // LRC` | LRC modification |
| `dlls/rpg.cpp` | 99 | `//LRC: -1 means suspend indefinitely` | -1 means suspend indefinitely |
| `dlls/schedule.h` | 174 | `//LRC` | LRC modification |
| `dlls/singleplay_gamerules.cpp` | 127 | `//LRC- support the new "start with HEV" flag...` | support the new "start with HEV" flag... |
| `dlls/singleplay_gamerules.cpp` | 131 | `// LRC what's wrong with allowing "game_player_equip" entities in single player? (The` | what's wrong with allowing "game_player_equip" entities in single player? (The |
| `dlls/subs.cpp` | 106 | `//LRC - remove this from the AssistList.` | remove this from the AssistList. |
| `dlls/subs.cpp` | 118 | `//LRC` | LRC modification |
| `dlls/subs.cpp` | 142 | `//LRC - do the same thing if another entity is moving with _me_.` | do the same thing if another entity is moving with _me_. |
| `dlls/subs.cpp` | 214 | `DEFINE_FIELD(CBaseDelay, m_hActivator, FIELD_EHANDLE), //LRC` | LRC modification |
| `dlls/subs.cpp` | 278 | `//LRC - allow changing of usetype` | allow changing of usetype |
| `dlls/subs.cpp` | 352 | `//LRC- Firing has finished, aliases can now reflect their new values.` | Firing has finished, aliases can now reflect their new values. |
| `dlls/subs.cpp` | 399 | `//LRC - Valve had a hacked thing here to avoid breaking` | Valve had a hacked thing here to avoid breaking |
| `dlls/subs.cpp` | 416 | `//LRC- now just USE_KILLs its killtarget, for consistency.` | now just USE_KILLs its killtarget, for consistency. |
| `dlls/subs.cpp` | 470 | `//LRC - now using m_hActivator.` | now using m_hActivator. |
| `dlls/subs.cpp` | 498 | `DEFINE_FIELD(CBaseToggle, m_flAngularMoveSpeed, FIELD_FLOAT), //LRC` | LRC modification |
| `dlls/subs.cpp` | 664 | `DontThink(); //LRC` | LRC modification |
| `dlls/subs.cpp` | 677 | `//LRC- mapping toggle-states to global states` | mapping toggle-states to global states |
| `dlls/subs.cpp` | 795 | `if (pev->movedir != g_vecZero) //LRC` | LRC modification |
| `dlls/subs.cpp` | 848 | `// LRC - info_movewith, the first entity I've made which` | info_movewith, the first entity I've made which |
| `dlls/triggers.cpp` | 31 | `#include "weapons.h" //LRC, for trigger_hevcharge` | , for trigger_hevcharge |
| `dlls/triggers.cpp` | 32 | `#include "movewith.h" //LRC` | LRC modification |
| `dlls/triggers.cpp` | 33 | `#include "locus.h" //LRC` | LRC modification |
| `dlls/triggers.cpp` | 176 | `UTIL_DesiredAction(this); //LRC - don't think until the player has spawned.` | don't think until the player has spawned. |
| `dlls/triggers.cpp` | 376 | `//LRC - trigger_rottest, temporary new entity` | trigger_rottest, temporary new entity |
| `dlls/triggers.cpp` | 502 | `float m_flWait; //LRC- minimum length of time to wait` | minimum length of time to wait |
| `dlls/triggers.cpp` | 503 | `float m_flMaxWait; //LRC- random, maximum length of time to wait` | random, maximum length of time to wait |
| `dlls/triggers.cpp` | 504 | `string_t m_sMaster; //LRC- master` | master |
| `dlls/triggers.cpp` | 505 | `int m_iMode; //LRC- 0 = timed, 1 = pick random, 2 = each random` | 0 = timed, 1 = pick random, 2 = each random |
| `dlls/triggers.cpp` | 506 | `int m_iszThreadName; //LRC` | LRC modification |
| `dlls/triggers.cpp` | 507 | `int m_iszLocusThread; //LRC` | LRC modification |
| `dlls/triggers.cpp` | 511 | `USE_TYPE m_triggerType; //LRC` | LRC modification |
| `dlls/triggers.cpp` | 532 | `DEFINE_FIELD(CMultiManager, m_iState, FIELD_INTEGER), //LRC` | LRC modification |
| `dlls/triggers.cpp` | 533 | `DEFINE_FIELD(CMultiManager, m_iMode, FIELD_INTEGER), //LRC` | LRC modification |
| `dlls/triggers.cpp` | 535 | `DEFINE_FIELD(CMultiManager, m_triggerType, FIELD_INTEGER), //LRC` | LRC modification |
| `dlls/triggers.cpp` | 538 | `DEFINE_FIELD(CMultiManager, m_sMaster, FIELD_STRING), //LRC` | LRC modification |
| `dlls/triggers.cpp` | 540 | `DEFINE_FIELD(CMultiManager, m_flWait, FIELD_FLOAT), //LRC` | LRC modification |
| `dlls/triggers.cpp` | 541 | `DEFINE_FIELD(CMultiManager, m_flMaxWait, FIELD_FLOAT), //LRC` | LRC modification |
| `dlls/triggers.cpp` | 542 | `DEFINE_FIELD(CMultiManager, m_iszThreadName, FIELD_STRING), //LRC` | LRC modification |
| `dlls/triggers.cpp` | 543 | `DEFINE_FIELD(CMultiManager, m_iszLocusThread, FIELD_STRING), //LRC` | LRC modification |
| `dlls/triggers.cpp` | 555 | `//LRC- that would support Delay, Killtarget, Lip, Distance, Wait and Master.` | that would support Delay, Killtarget, Lip, Distance, Wait and Master. |
| `dlls/triggers.cpp` | 568 | `else if (FStrEq(pkvd->szKeyName, "master")) //LRC` | LRC modification |
| `dlls/triggers.cpp` | 573 | `else if (FStrEq(pkvd->szKeyName, "m_iszThreadName")) //LRC` | LRC modification |
| `dlls/triggers.cpp` | 578 | `else if (FStrEq(pkvd->szKeyName, "m_iszLocusThread")) //LRC` | LRC modification |
| `dlls/triggers.cpp` | 583 | `else if (FStrEq(pkvd->szKeyName, "mode")) //LRC` | LRC modification |
| `dlls/triggers.cpp` | 588 | `else if (FStrEq(pkvd->szKeyName, "triggerstate")) //LRC` | LRC modification |
| `dlls/triggers.cpp` | 595 | `break; //LRC- yes, this algorithm is different` | yes, this algorithm is different |
| `dlls/triggers.cpp` | 619 | `else //LRC - keep a count of how many targets, for the error message` | keep a count of how many targets, for the error message |
| `dlls/triggers.cpp` | 635 | `//LRC` | LRC modification |
| `dlls/triggers.cpp` | 698 | `//LRC- different manager modes` | different manager modes |
| `dlls/triggers.cpp` | 706 | `if (m_flMaxWait) //LRC- random time to wait?` | random time to wait? |
| `dlls/triggers.cpp` | 708 | `else if (m_flWait) //LRC- constant time to wait?` | constant time to wait? |
| `dlls/triggers.cpp` | 710 | `else //LRC- just start immediately.` | just start immediately. |
| `dlls/triggers.cpp` | 818 | `if (m_flMaxWait) //LRC- random time to wait?` | random time to wait? |
| `dlls/triggers.cpp` | 823 | `else if (m_flWait) //LRC- constant time to wait?` | constant time to wait? |
| `dlls/triggers.cpp` | 828 | `else //LRC- just start immediately.` | just start immediately. |
| `dlls/triggers.cpp` | 899 | `if (m_iszThreadName) pMulti->pev->targetname = m_iszThreadName; //LRC` | LRC modification |
| `dlls/triggers.cpp` | 900 | `pMulti->m_triggerType = m_triggerType; //LRC` | LRC modification |
| `dlls/triggers.cpp` | 901 | `pMulti->m_iMode = m_iMode; //LRC` | LRC modification |
| `dlls/triggers.cpp` | 902 | `pMulti->m_flWait = m_flWait; //LRC` | LRC modification |
| `dlls/triggers.cpp` | 903 | `pMulti->m_flMaxWait = m_flMaxWait; //LRC` | LRC modification |
| `dlls/triggers.cpp` | 959 | `//LRC- "master" support` | "master" support |
| `dlls/triggers.cpp` | 985 | `if (m_flMaxWait) //LRC- random time to wait?` | random time to wait? |
| `dlls/triggers.cpp` | 990 | `else if (m_flWait) //LRC- constant time to wait?` | constant time to wait? |
| `dlls/triggers.cpp` | 995 | `else //LRC- just start immediately.` | just start immediately. |
| `dlls/triggers.cpp` | 1010 | `if (pev->spawnflags & SF_MULTIMAN_SAMETRIG) //LRC` | LRC modification |
| `dlls/triggers.cpp` | 1045 | `//LRC- multi_watcher entity: useful? Well, I think it is. And I'm worth it. :)` | multi_watcher entity: useful? Well, I think it is. And I'm worth it. :) |
| `dlls/triggers.cpp` | 1389 | `//LRC` | LRC modification |
| `dlls/triggers.cpp` | 1394 | `//LRC-  RenderFxFader, a subsidiary entity for RenderFxManager` | RenderFxFader, a subsidiary entity for RenderFxManager |
| `dlls/triggers.cpp` | 1544 | `//LRC - amt is often 0 when mode is normal. Set it to be fully visible, for fade purposes.` | amt is often 0 when mode is normal. Set it to be fully visible, for fade purposes. |
| `dlls/triggers.cpp` | 1563 | `//LRC - fade the entity in/out!` | fade the entity in/out! |
| `dlls/triggers.cpp` | 1839 | `//LRCT` | T |
| `dlls/triggers.cpp` | 2187 | `//LRC - this was very bloated. I moved lots of methods into the` | this was very bloated. I moved lots of methods into the |
| `dlls/triggers.cpp` | 2527 | `//LRC` | LRC modification |
| `dlls/triggers.cpp` | 2978 | `//LRC - trigger_inout, a trigger which fires _only_ when` | trigger_inout, a trigger which fires _only_ when |
| `dlls/triggers.cpp` | 3072 | `//		ALERT(at_console, "adding; max %.2f %.2f %.2f, min %.2f %.2f %.2f is inside max %.2f %.2f %.2f, min %.2f %.2f %.2...` | T |
| `dlls/triggers.cpp` | 3092 | `//			ALERT(at_console, "removing; max %.2f %.2f %.2f, min %.2f %.2f %.2f is outside max %.2f %.2f %.2f, min %.2f %.2f...` | T |
| `dlls/triggers.cpp` | 3385 | `//LRC -- don't allow changelevels to contain capital letters; it causes problems` | - don't allow changelevels to contain capital letters; it causes problems |
| `dlls/triggers.cpp` | 3790 | `//LRC- NODRAW is a better-performance way to stop things being drawn.` | NODRAW is a better-performance way to stop things being drawn. |
| `dlls/triggers.cpp` | 3929 | `//LRC- trigger_bounce` | trigger_bounce |
| `dlls/triggers.cpp` | 3969 | `//LRC- trigger_onsight` | trigger_onsight |
| `dlls/triggers.cpp` | 4185 | `//LRC - landmark based teleports` | landmark based teleports |
| `dlls/triggers.cpp` | 4251 | `pOther->pev->v_angle = pTarget->pev->angles; //LRC` | LRC modification |
| `dlls/triggers.cpp` | 4401 | `//LRC- trigger_startpatrol` | trigger_startpatrol |
| `dlls/triggers.cpp` | 4459 | `//LRC- trigger_motion` | trigger_motion |
| `dlls/triggers.cpp` | 4700 | `//LRC- motion_manager` | motion_manager |
| `dlls/triggers.cpp` | 5049 | `//LRC - you thought _that_ was a bad idea? Check this baby out...` | you thought _that_ was a bad idea? Check this baby out... |
| `dlls/triggers.cpp` | 5374 | `//LRC` | LRC modification |
| `dlls/triggers.cpp` | 5453 | `if (pev->velocity.Length() < 10.0) //LRC- whyyyyyy???` | whyyyyyy??? |
| `dlls/turret.cpp` | 322 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/turret.cpp` | 352 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/turret.cpp` | 362 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/turret.cpp` | 386 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/turret.cpp` | 1203 | `PRECACHE_MODEL((char*)STRING(pev->model)); //LRC` | LRC modification |
| `dlls/turret.cpp` | 1212 | `SET_MODEL(ENT(pev), STRING(pev->model)); //LRC` | LRC modification |
| `dlls/turret.cpp` | 1215 | `if (!pev->health) //LRC` | LRC modification |
| `dlls/util.cpp` | 395 | `//LRC - pass in a normalised axis vector and a number of degrees, and this returns the corresponding` | pass in a normalised axis vector and a number of degrees, and this returns the corresponding |
| `dlls/util.cpp` | 415 | `//LRC - as above, but returns the position of point 1 0 0 under the given rotation` | as above, but returns the position of point 1 0 0 under the given rotation |
| `dlls/util.cpp` | 598 | `//LRC - things get messed up if aliases change in the middle of an entity traversal.` | things get messed up if aliases change in the middle of an entity traversal. |
| `dlls/util.cpp` | 960 | `//LRC UNDONE: Work during trigger_camera?` | UNDONE: Work during trigger_camera? |
| `dlls/util.cpp` | 1711 | `//LRC - moved here from barney.cpp` | moved here from barney.cpp |
| `dlls/util.cpp` | 1760 | `//LRC - randomized vectors of the form "0 0 0 .. 1 0 0"` | randomized vectors of the form "0 0 0 .. 1 0 0" |
| `dlls/util.h` | 187 | `//LRC- the values used for the new "global states" mechanism.` | the values used for the new "global states" mechanism. |
| `dlls/util.h` | 239 | `extern Vector UTIL_AxisRotationToAngles(const Vector& vec, float angle); //LRC` | LRC modification |
| `dlls/util.h` | 240 | `extern Vector UTIL_AxisRotationToVec(const Vector& vec, float angle); //LRC` | LRC modification |
| `dlls/util.h` | 242 | `//LRC` | LRC modification |
| `dlls/util.h` | 252 | `//LRC - for $locus references` | for $locus references |
| `dlls/util.h` | 320 | `extern void UTIL_StringToRandomVector(float* pVector, const char* pString); //LRC` | LRC modification |
| `dlls/util.h` | 339 | `extern BOOL UTIL_IsFacing(entvars_t* pevTest, const Vector& reference); //LRC` | LRC modification |
| `dlls/util.h` | 507 | `#define SF_BREAK_FADE_RESPAWN	8// LRC- fades in gradually when respawned` | fades in gradually when respawned |
| `dlls/util.h` | 512 | `#define SF_PUSH_NOPULL			512//LRC` | LRC modification |
| `dlls/util.h` | 513 | `#define SF_PUSH_USECUSTOMSIZE	0x800000 //LRC, not yet used` | , not yet used |
| `dlls/util.h` | 616 | `// LRC- for aliases and groups` | for aliases and groups |
| `dlls/weapons.cpp` | 797 | `//LRC` | LRC modification |
| `dlls/weapons.cpp` | 1276 | `// LRC - remove the specified ammo from this gun` | remove the specified ammo from this gun |
| `dlls/weapons.h` | 310 | `virtual void SetNextThink(float delay); //LRC` | LRC modification |
| `dlls/weapons.h` | 362 | `//LRC - used by weaponstrip` | used by weaponstrip |
| `dlls/world.cpp` | 36 | `#include "movewith.h" //LRC` | LRC modification |
| `dlls/world.cpp` | 469 | `//#define SF_WORLD_STARTSUIT	0x0008		// LRC- Start this level with an HEV suit!` | Start this level with an HEV suit! |
| `dlls/world.cpp` | 474 | `BOOL g_startSuit; //LRC` | LRC modification |
| `dlls/world.cpp` | 485 | `//LRC - set up the world lists` | set up the world lists |
| `dlls/world.cpp` | 568 | `PRECACHE_MODEL("sprites/null.spr"); //LRC` | LRC modification |
| `dlls/world.cpp` | 722 | `//LRC- let map designers start the player with his suit already on` | let map designers start the player with his suit already on |
| `dlls/world.cpp` | 733 | `//LRC- ends` | ends |
| `engine/eiface.h` | 59 | `//LRC- identifies the ALERT statements which don't need removing before release` | identifies the ALERT statements which don't need removing before release |

## 24. New Files

The following source files are entirely new to Spirit of Half-Life (not present in the vanilla Half-Life SDK):

| File | Purpose |
|------|---------|
| `dlls/alias.cpp` | Alias entity system — allows entity references by alias names |
| `dlls/alias.h` | Header for alias system |
| `dlls/locus.cpp` | Locus system — calculates positions/velocities from entity references |
| `dlls/locus.h` | Header for locus system |
| `dlls/movewith.cpp` | MoveWith core logic — parent/child entity movement coordination |
| `dlls/movewith.h` | Header for MoveWith system |
| `dlls/weapons_shared.cpp` | Shared weapons code |
| `cl_dll/particlemgr.cpp` | Client-side particle system manager |
| `cl_dll/particlemgr.h` | Header for particle manager |


## 25. Summary Statistics

- **Total LRC-commented lines**: 795
- **Unique source files modified**: 96
- **Major feature categories**: 23

### Changes per Category

| Category | Count |
|----------|-------|
| State System | 22 |
| Fog System | 13 |
| Sky System | 9 |
| Custom HUD Color | 7 |
| Shiny/Reflective Surfaces | 10 |
| Dynamic Lighting | 6 |
| Monster/NPC | 143 |
| Func_Tank | 45 |
| Platform/Train | 69 |
| Door | 17 |
| Button | 19 |
| Breakable | 12 |
| Light | 6 |
| Sound | 14 |
| Effects | 73 |
| Model/Animation | 6 |
| Particle System | 1 |
| Scripted Sequences | 33 |
| New Entity Definitions | 5 |
| Other/Misc | 285 |