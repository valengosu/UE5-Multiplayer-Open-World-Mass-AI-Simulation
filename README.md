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

Gameplay events cross the Mass processing boundary through a **double-buffered pipeline**, with lifecycle operations synchronized before execution on the Game Thread.
