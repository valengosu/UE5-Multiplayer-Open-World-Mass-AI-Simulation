A scalable multiplayer NPC simulation system built with Unreal Engine 5, designed to support large open-world populations while allowing lightweight NPCs to dynamically enter and leave full gameplay.

<!-- Demo GIF / Video -->

## Overview

Large open-world NPC systems need to balance three competing requirements: **large populations, limited runtime budgets, and full multiplayer gameplay**.

Keeping every NPC as a fully simulated Character with AI, animation, collision, and network replication does not scale well as population size increases. However, lightweight simulation alone is not sufficient when NPCs need to dynamically participate in full gameplay interactions.

This project addresses that problem with a hybrid architecture: NPCs normally run as lightweight **Mass Entities**, then dynamically transition to fully replicated **RealActors** when gameplay requires full AI and interaction. After the interaction ends, they can return to Mass simulation.

**Population → Mass Simulation → Gameplay Event → RealActor → Full AI / Gameplay → Mass**

The world is built on **World Partition** and **HLOD** to keep large environments streamable, while NPC population is managed independently from world size.

Instead of maintaining a fixed number of NPCs across the entire map, the population system evaluates the **ZoneGraph lane space around active players** and uses configurable population density to determine how many NPCs should exist in each relevant area.

As players move through the world, the system continuously compares the **desired population** against the current population and performs **Spawn, Recycle, and Rebalance** operations to maintain local density.

This keeps the active NPC workload tied primarily to **player-relevant areas and local density**, rather than directly scaling with the total size of the open world.
