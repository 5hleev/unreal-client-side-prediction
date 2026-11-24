"""Run inside Unreal Editor via -ExecutePythonScript, once after C++ build."""
import unreal

MAP = '/Game/Maps/PredictionArena'
if unreal.EditorAssetLibrary.does_asset_exist(MAP):
    unreal.log('PredictionArena already exists; keeping the existing map.')
else:
    level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level.new_level(MAP):
        raise RuntimeError('Cannot create ' + MAP)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    cube = unreal.load_asset('/Engine/BasicShapes/Cube')

    def block(label, location, scale):
        actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*location))
        actor.set_actor_label(label)
        actor.static_mesh_component.set_static_mesh(cube)
        actor.set_actor_scale3d(unreal.Vector(*scale))
        actor.static_mesh_component.set_collision_profile_name('BlockAll')
        return actor

    block('ArenaFloor', (0, 0, -50), (80, 80, 1))
    block('Obstacle', (1000, 0, 100), (1, 12, 2))
    block('LowStep', (0, 1000, 20), (5, 5, 0.4))
    for label, pos, scale in [
        ('North', (0, 4000, 150), (80, 1, 3)),
        ('South', (0, -4000, 150), (80, 1, 3)),
        ('East', (4000, 0, 150), (1, 80, 3)),
        ('West', (-4000, 0, 150), (1, 80, 3)),
    ]:
        block(label, pos, scale)
    for x, y in [(-1500, -300), (-1500, 300), (-2000, -300), (-2000, 300)]:
        actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(x, y, 110))
    sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(-55, -30, 0))
    sun.light_component.set_editor_property('intensity', 4.0)
    sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 600))
    sky.light_component.set_editor_property('intensity', 1.0)
    if not level.save_current_level():
        raise RuntimeError('Cannot save arena')
    unreal.log('Created ' + MAP)
