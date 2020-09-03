#define GAME_COMPONENT_SEGMENT \
ComponentStorage<Health, paged_set>, \
ComponentStorage<Age, paged_set>, \
ComponentStorage<Player, paged_set>, \
ComponentStorage<Bullet, paged_set>, \
ComponentStorage<Enemy, paged_set>, \
ComponentStorage<ParticleScriptComp, paged_set>, \
ComponentStorage<Dummy, paged_set>, \
ComponentStorage<SpawnerComp, paged_set>, \
ComponentStorage<SuckerComp, paged_set>, \
ComponentStorage<Tester, paged_set>
