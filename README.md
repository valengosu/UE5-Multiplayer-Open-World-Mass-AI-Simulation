# UE5 Multiplayer Open-World Mass AI Simulation

A scalable multiplayer NPC simulation system for large open worlds, combining dynamic population management, Mass simulation, and full gameplay interaction.

## Overview

Large open-world NPC systems must balance **population scale, runtime cost, and multiplayer gameplay**. This project uses a hybrid architecture where large NPC populations remain in lightweight simulation and dynamically transition into full gameplay actors when required.

## Open-World Population

**World Partition and HLOD** provide large-world streaming, while a custom population system uses **ZoneGraph lane space and configurable density** to determine the desired NPC population around active players.

As players move, the system performs **Spawn, Recycle, and Rebalance**, keeping the active NPC workload primarily tied to player-relevant areas rather than total world size.

## Mass Simulation

NPCs are maintained as lightweight **Mass Entities**, with **Simulation LOD** and **Representation LOD** adjusting simulation and visual cost according to player relevance.

In multiplayer, the **server remains authoritative**, while Mass replication maintains corresponding client-side entities and representations.

## Gameplay-Driven Mass ↔ RealActor Lifecycle

Gameplay events can dynamically upgrade relevant Mass NPCs into replicated **RealActors** with full Character movement, collision, animation, and AI. Once the interaction ends, they return to Mass simulation.

**Population → Mass → Gameplay Event → RealActor → AI / Gameplay → Mass**

Gameplay events generated during parallel Mass processing are collected through a **double-buffered pipeline** and consumed on the **Game Thread after the Mass processing phase**, avoiding unsafe cross-thread Actor operations.

## Mass Framework Multiplayer Fixes & Extensions

Making this architecture work reliably in multiplayer exposed several limitations and bugs in the UE5.7 Mass framework that required framework-level fixes and extensions.

### Client-Side Entity Removal

Destroying a replicated Mass Entity on the server could leave the corresponding entity alive on remote clients. I added a server-side removal observer that explicitly cleans the agent from each client's replication bubble and cached replication data, allowing removal to propagate correctly.

### Multiplayer ZoneGraph Navigation

The default ZoneGraph navigation workflow did not support the multiplayer behavior required by this system. I extended the navigation path so Mass NPCs can correctly use ZoneGraph movement in a server-authoritative multiplayer environment.

### Mass Representation Release Bug

While implementing Mass → RealActor takeover, I traced an inconsistent representation state to an internal bug in UE5.7's `ReleaseTemplateActorOrCancelSpawning()`.

I replaced the affected path with a corrected implementation that safely handles pending spawn requests, actor-template mismatches, representation release, and fragment reacquisition after archetype changes.
