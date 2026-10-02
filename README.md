A scalable multiplayer NPC simulation system built with Unreal Engine 5, designed to support large open-world populations while allowing lightweight NPCs to dynamically enter and leave full gameplay.

<!-- Demo GIF / Video -->

## Overview

Large open-world NPC systems need to balance three competing requirements: **large populations, limited runtime budgets, and full multiplayer gameplay**.

Keeping every NPC as a fully simulated Character with AI, animation, collision, and network replication does not scale well as population size increases. However, lightweight simulation alone is not sufficient when NPCs need to dynamically participate in full gameplay interactions.

This project addresses that problem with a hybrid architecture: NPCs normally run as lightweight **Mass Entities**, then dynamically transition to fully replicated **RealActors** when gameplay requires full AI and interaction. After the interaction ends, they can return to Mass simulation.

**Population → Mass Simulation → Gameplay Event → RealActor → Full AI / Gameplay → Mass**
