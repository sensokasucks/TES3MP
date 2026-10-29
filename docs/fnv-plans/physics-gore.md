# Physics, ragdolls, scaling and generic gore

Planned with Sen on 2026-10-29 (thread "P0 Plan the modern AI option"). This covers engine work that both games share. Basic physics and ragdolls belong to the faithful path, because the original FNV has them. The extras are opt-in, like the modern AI.

## Where things stand (openmw-fnv integration, 2026-10-29)

- No ragdolls. Death plays a creature's Death.KF, a get-up animation run backwards, or `deathpose1/2.kf`. Placed corpses ignore their saved XRGD/XRGB poses.
- ESM4 NPCs and creatures have no physics body. esm4actors moves them along the navmesh.
- Static collision uses the render mesh. The bhk* Havok blocks are parsed in `components/nif/physics.*` but nifbullet never reads them.
- Gore (P2-15.0) uses FNV's own BPTD body part data and vanilla gib meshes. Console: `gib`, `gib head`, `sever head`.
- Source: the animation and physics audit `render-anim-physics.md` in the project files, sections 1.7 and 2.

## Faithful physics (default)

| Step | Content | Size |
|------|---------|------|
| F1 Collision | nifbullet reads bhkCollisionObject, bhkRigidBody, box, sphere, capsule, convex, list, MOPP and packed tri-strip shapes. Havok units scaled to game units. Render mesh only when no bhk data exists. | M |
| F2 Actor bodies | A capsule per ESM4 actor (height from a race and creature table), and esm4actors movement through MWPhysics. Falls, pushes, blocking by heavy bodies (`fMoveLimitMass` 95). | M |
| F3 Ragdolls | Bullet constraints built from each skeleton's bhkRagdollSystem and RGDL/BPTD records. Keyframed while alive, dynamic on death and knockdown with the death impulse. Pose-matched get-up. Placed corpses take their XRGD pose. | L |
| F4 Clutter | Dynamic rigid bodies with mass, friction and restitution from bhkRigidBody. Grab with Z (`fGrabMaxWeightWalking` 100, running 50). Physics damage tiers from `fPhysicsDamage*`. | L |

## Modern extras (opt-in)

- Active ragdolls: hits make NPCs stagger and stumble through a blend of animation and physics, instead of a flinch in place.
- Wounded movement: limping on crippled legs, bracing against walls.
- Better explosion knockdown, and companions who catch their balance.

## Scaling to many ragdolls and AIs

The GPU helps most with drawing many characters. Ragdolls and AI scale better through level of detail and threads.

1. **GPU skinning.** Check how the fork skins meshes today. If it is still on the CPU, move skinning to a shader. This speeds up every character on screen. First item.
2. **Ragdoll sleep and budget.** Only the nearest 5 to 10 ragdolls simulate fully. A body at rest freezes into a static pose and costs nothing after that. Physics runs on its own thread (OpenMW already supports async physics). First item, alongside F3.
3. **AI level of detail.** Far actors think about once a second. Long path searches go to worker threads or the server (see [server-ai.md](server-ai.md)). Towns use flow fields instead of a path per NPC. Flow fields could move to a compute shader later if they show up in profiles. OpenMW's older OpenGL target limits compute shaders.
4. **Vertex animation textures (VAT).** Walk and idle loops for the rendered-lite civilian tier are baked into textures and drawn instanced, so hundreds of far people cost a few draw calls. This does not fit close NPCs, which need equipment swaps, head tracking and blending. Comes with B5 in [modern-ai.md](modern-ai.md).

## Generic gore that also works in Morrowind

Morrowind ships no gib or severed-limb meshes, so the engine generates them from the character's own mesh:

- **Morrowind NPCs are already split by limb.** Bodies are built from separate body parts (head, hair, chest, upper arm, forearm, hand, thigh, knee, ankle, foot), and armor and clothing cover the same slots. Dismemberment hides the parts below the cut and spawns those parts as a loose physics piece. No new art is needed for the cut itself.
- **Creatures and single-mesh bodies are cut by bone weights.** At load time, sort each skinned vertex by its strongest bone and group bones into limbs. A cut splits the mesh at a limb root into two meshes. The stump and the severed piece both get a generic cap.
- **One small shared gore pack** of our own: a few wound caps, meat chunks, bone splinters, blood decals and particles. The blood color comes from the actor (Morrowind's blood texture index, FNV's impact data), so skeletons and ghosts give dust or ectoplasm instead of red blood. Our own art, so it ships with the engine.
- **Explosion gibbing.** The body becomes chunks from the shared pack plus the cut limbs, thrown with the explosion impulse. This needs F3-style rigid bodies for the pieces.
- **Rules.** Morrowind gore is a setting, off by default. It never fires on essential NPCs, and a cut corpse can still be looted. The gore state is saved, and it is sent in multiplayer as one small event (actor, limb, impulse seed).
- **FNV keeps its own data first.** When BPTD gore data and vanilla gib meshes exist, those are used. The generic path is the fallback for actors with no gore data.

Order: the body-part hide and the Morrowind NPC limb drop first, since they need no physics. Then the bone-weight cutter. Explosion gibbing comes after F3.

## Bleeding

Added 2026-10-29. Opt-in in both games, like the other gore settings.

- **Health over time.** A bleed is a timed damage-health effect on the existing active effects system (FNV I.FNVEffects, Morrowind magic effects). Each wound adds its own stack. Strength comes from the hit: damage, weapon type (blades and big guns bleed more), and the hit body part. Stimpaks, bandages and doctor's bags stop it in FNV, and healing spells and potions in Morrowind. Bleeding never fires on essential NPCs and never kills during a quest scene.
- **AI reaction.** Under the modern brain, a badly bleeding NPC retreats to heal, slows down or limps (ties into B3 and B6 in [modern-ai.md](modern-ai.md)). The original brain only gets the health loss.
- **Drips.** Particles fall from the wound bone. Where one lands (a short raycast), it leaves a small splat.
- **Trails.** A moving bleeder drops a splat every few steps, so a wounded animal can be tracked by its blood.
- **Spray and pools.** A hit sprays onto walls and floors behind the target along the shot direction. A body leaves a pool that grows for a few seconds.
- **Decal system.** OpenMW has none for this today. New work: projected decals (or terrain-hugging quads) on terrain and statics, with a budget of about 200 to 300 splats. The oldest fade out first, and a cell's splats are dropped when it unloads.
- **Textures.** FNV uses its own blood decals and impact data. Morrowind uses the actor's blood type, so skeletons drop dust and ghosts ectoplasm.
- **Multiplayer.** The bleed effect is game state and replicates with effects. The splats are cosmetic: each machine places them from the wound event plus a seed, so they are never sent.

Order: the bleed effect first (small, effects already exist), then drips, trails and pools once the decal system is in.
