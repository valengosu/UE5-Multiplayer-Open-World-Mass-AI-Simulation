# UE5 Multiplayer Open-World Mass AI Simulation

A scalable multiplayer NPC simulation system combining dynamic population management, Mass simulation, and full gameplay interaction.

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
