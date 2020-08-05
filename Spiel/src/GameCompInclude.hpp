#define GAME_COMPONENT_SEGMENT \
ComponentStorage<Health, sparse_set>, \
ComponentStorage<Age, sparse_set>, \
ComponentStorage<Player, sparse_set>, \
ComponentStorage<Bullet, sparse_set>, \
ComponentStorage<Enemy, sparse_set>, \
ComponentStorage<ParticleScriptComp, sparse_set>, \
ComponentStorage<Dummy, sparse_set>, \
ComponentStorage<SpawnerComp, sparse_set>, \
ComponentStorage<SuckerComp, sparse_set>
