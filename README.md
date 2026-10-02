# UE5 Multiplayer Open-World Mass AI Simulation

A scalable multiplayer NPC simulation system combining dynamic population management, Mass simulation, and full gameplay interaction.

Problem
Large multiplayer open worlds need many persistent NPCs, but running every NPC as a full gameplay Actor with Character movement, collision, animation, AI, and replication does not scale.

This demo solves that problem by keeping the wider population in lightweight Mass simulation and dynamically promoting only gameplay-relevant NPCs to full replicated Actors.

## System Architecture

### Open-World Population

**World Partition and HLOD** handle world streaming, while a custom population system uses **ZoneGraph lane space and configurable density** to maintain the desired NPC population around active players through **Spawn, Recycle, and Rebalance**.

### Mass Simulation

NPCs remain as lightweight **Mass Entities**, with **Simulation LOD** and **Representation LOD** controlling runtime cost. The server remains authoritative while Mass replication maintains corresponding client entities and representations.

### Gameplay-Driven Mass ↔ RealActor Lifecycle

Gameplay events dynamically upgrade relevant Mass NPCs into replicated **RealActors** with full movement, collision, animation, and AI, then return them to Mass simulation when the interaction ends.

**Population → Mass → Gameplay Event → RealActor → AI / Gameplay → Mass**

**Events generated during parallel Mass processing cross to the **Game Thread** through a double-buffered pipeline after the Mass processing phase.**

## UE5.7 Mass Framework Fixes

Building this architecture for multiplayer required several framework-level fixes and extensions:

- **Client Entity Removal** — Fixed replicated Mass Entities remaining alive on remote clients after server-side destruction.
- **Multiplayer ZoneGraph Navigation** — Extended the default ZoneGraph navigation path to support server-authoritative multiplayer NPC movement.
- **Representation Release** — Fixed an internal issue in `ReleaseTemplateActorOrCancelSpawning()` and implemented a safe release path for Mass → RealActor takeover.

## Seamless Mass ↔ RealActor Integration

Making the lifecycle functionally correct was not enough—the transition also had to remain visually continuous across multiplayer replication and independent animation instances.

### Spatial Continuity

During Mass → RealActor takeover, the client-side Mass representation may continue moving before the replicated RealActor reaches the client, creating a visible position mismatch. I preserve the visual offset during takeover and let Character Movement network correction converge smoothly toward the server-authoritative state instead of snapping immediately.

### Animation Continuity

Mass representations and RealActors use independent animation instances, so a direct handoff can cause visible animation discontinuity. I transfer locomotion parameters and semantic animation state across the transition, allowing the destination animation instance to resume from a matching gameplay state.
