# UE5 Multiplayer Open-World Mass AI Simulation

A scalable multiplayer NPC simulation system for large open worlds, combining dynamic population management with lightweight simulation and full gameplay interaction.

## Overview

Large open-world NPC systems must support **large populations, limited runtime budgets, and multiplayer gameplay**. Keeping every NPC as a full Character with AI, animation, collision, and networking is too expensive, while lightweight simulation alone cannot support NPCs that need to enter full gameplay dynamically.

This project uses a hybrid architecture that maintains large NPC populations through lightweight simulation and dynamically converts relevant NPCs into full gameplay actors when needed.

## Open-World Population

The environment uses **World Partition and HLOD** for large-world streaming, while NPC population is managed dynamically around active players.

The population system evaluates valid **ZoneGraph lane space** and configurable density to determine the desired NPC population, then performs **Spawn, Recycle, and Rebalance** as players move through the world.

This keeps the active NPC workload primarily tied to **player-relevant areas and local population density**, rather than the total size of the world.
