# UE5 Multiplayer Open-World Mass AI Simulation

A scalable multiplayer NPC simulation system combining dynamic population management, Mass simulation, and full gameplay interaction.

**Problem:** Large multiplayer open worlds need many persistent NPCs, but running every NPC as a full gameplay Actor with Character movement, collision, animation, AI, and replication does not scale.

This demo solves that problem by keeping the wider population in lightweight Mass simulation and dynamically promoting only gameplay-relevant NPCs to full replicated Actors.

## System Architecture

### Open-World Population

**World Partition and HLOD** handle world streaming, while a custom population system uses **ZoneGraph lane space and configurable density** to maintain the desired NPC population around active players through **Spawn, Recycle, and Rebalance**.

### Mass Simulation

NPCs remain as lightweight **Mass Entities**, with **Simulation LOD** and **Representation LOD** controlling runtime cost. The server remains authoritative while Mass replication maintains corresponding client entities and representations.

I extended the Mass replication strategy with **configurable synchronization thresholds**, allowing replication granularity and network traffic to be dynamically balanced for large NPC populations.

### Gameplay-Driven Mass ↔ RealActor Lifecycle

Gameplay events dynamically upgrade relevant Mass NPCs into replicated RealActors with full movement, collision, animation, and AI. When the interaction ends, the RealActor hands control back to the corresponding Mass Entity, restoring its transform and lightweight representation.

**Population → Mass → Gameplay Event → RealActor → AI / Gameplay → Mass**

**Gameplay events can be generated concurrently during multi-threaded Mass processing. A lock-free double-buffered pipeline collects them without blocking worker threads, then transfers them to the Game Thread after the Mass processing phase for Actor lifecycle operations.**

## UE5.7 Mass Framework Fixes

Building this architecture for multiplayer required several framework-level fixes and extensions:

- **Client Entity Removal** — Fixed replicated Mass Entities remaining alive on remote clients after server-side destruction by correcting the client-bubble removal path.
- **Multiplayer ZoneGraph Navigation** — Extended the default ZoneGraph navigation path to support multiple player-driven navigation contexts in a server-authoritative multiplayer world.
- **Representation Release** — Traced a takeover failure to an internal issue in UE5.7's `ReleaseTemplateActorOrCancelSpawning()` and implemented a corrected release path for Mass → RealActor transitions.

## Seamless Mass ↔ RealActor Integration

Making the lifecycle functionally correct was not enough—the transition also had to remain visually continuous across multiplayer replication and independent animation instances.

### Spatial Continuity

When a Mass NPC becomes a replicated RealActor, the server-authoritative Actor position can differ from the client's still-moving Mass representation, causing visible jitter if the replicated position is applied directly.

I customized the Character Movement Component (CMC) network synchronization path to preserve visual continuity during takeover: small positional differences are temporarily absorbed while rotation continues to synchronize, allowing the client to transition without repeatedly snapping to the server position.

### Animation Continuity

Mass representations and RealActors use independent animation instances, so a direct handoff can cause visible animation discontinuity. I transfer locomotion parameters and semantic animation state across the transition, allowing the destination animation instance to enter a matching gameplay state.

## Tech Stack

Unreal Engine 5.7 · C++ · Mass Entity · ZoneGraph · World Partition · HLOD · Multiplayer Replication · Behavior Tree
