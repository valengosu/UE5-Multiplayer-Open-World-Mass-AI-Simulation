# UE5 Multiplayer Open-World Mass AI Simulation

A scalable multiplayer NPC simulation system for large open worlds, combining dynamic population management with lightweight simulation and full gameplay interaction.

## Overview

Large open-world NPC systems must support **large populations, limited runtime budgets, and multiplayer gameplay**. Keeping every NPC as a full Character with AI, animation, collision, and networking is too expensive, while lightweight simulation alone cannot support NPCs that need to enter full gameplay dynamically.

This project uses a hybrid architecture that maintains large NPC populations through lightweight simulation and dynamically converts relevant NPCs into full gameplay actors when needed.

## Open-World Population

The environment uses **World Partition and HLOD** for large-world streaming, while NPC population is managed dynamically around active players.

The population system evaluates valid **ZoneGraph lane space** and configurable density to determine the desired NPC population, then performs **Spawn, Recycle, and Rebalance** as players move through the world.

This keeps the active NPC workload primarily tied to **player-relevant areas and local population density**, rather than the total size of the world.

## Mass Simulation

Active NPCs are simulated as lightweight **Mass Entities**, avoiding the cost of maintaining full Characters, Actor-based AI, animation, collision, and networking for the entire population.

**Simulation LOD** and **Representation LOD** dynamically adjust simulation and visual fidelity based on player relevance, allowing nearby NPCs to use higher-fidelity representations while distant NPCs fall back to cheaper representations or no visualization.

In multiplayer, the **server remains authoritative over NPC simulation**, while Mass replication maintains corresponding client-side entities and representations.

## Gameplay-Driven Mass ↔ RealActor Lifecycle

Mass NPCs remain lightweight until gameplay requires full interaction. Gameplay events such as gunshots can trigger relevant NPCs to transition into replicated **RealActors**, enabling full Character movement, collision, animation, and Actor-based AI.

Once the interaction ends, the NPC returns to valid ZoneGraph space and transitions back into Mass simulation, completing the lifecycle:

**Population → Mass Simulation → Gameplay Event → RealActor → AI / Gameplay → Mass**

Gameplay events generated during Mass processing are transferred through a **double-buffered event pipeline**, using the Mass phase boundary as a synchronization point before lifecycle operations are executed on the Game Thread.
