"""
Blender Python script — Auto-rig a mesh and generate skiing pose animations.

Usage:
    /Applications/Blender.app/Contents/MacOS/Blender --background --python generate_ski_poses.py

Imports the unrigged skier FBX, creates a UE5-convention skeleton,
skins with automatic weights, creates 5 pose actions, and exports FBXs.
"""

import bpy
import os
import math
from mathutils import Vector

# --- Configuration ---

FBX_SOURCE = "/Users/toto/Downloads/stylized human 3d model/StylizedSkierBlocky.fbx"
OUTPUT_DIR = "/Users/toto/Documents/Unreal Projects/PowderRush/Content/Skier/Animations"
RIGGED_OUTPUT = "/Users/toto/Documents/Unreal Projects/PowderRush/Content/Skier/Animations/SkierRigged.fbx"

def rad(deg):
    return math.radians(deg)


# --- Skeleton Definition ---
# Bone positions derived from mesh vertex analysis.
# Mesh orientation: faces +X, arms along ±Y, Z up.
# Format: (bone_name, head_pos, tail_pos, parent_name)

BONES = [
    ("root",        (0, 0, 0),           (0, 0, 0.05),        None),
    ("pelvis",      (0, 0, 0.38),        (0, 0, 0.42),        "root"),
    ("spine_01",    (0, 0, 0.42),        (0, 0, 0.69),        "pelvis"),
    ("head",        (0, 0, 0.69),        (0, 0, 0.95),        "spine_01"),
    # Left arm (+Y side)
    ("upperarm_l",  (0, 0.09, 0.69),     (0, 0.26, 0.69),     "spine_01"),
    ("lowerarm_l",  (0, 0.26, 0.69),     (0, 0.39, 0.69),     "upperarm_l"),
    ("hand_l",      (0, 0.39, 0.69),     (0, 0.47, 0.69),     "lowerarm_l"),
    # Right arm (-Y side)
    ("upperarm_r",  (0, -0.09, 0.69),    (0, -0.26, 0.69),    "spine_01"),
    ("lowerarm_r",  (0, -0.26, 0.69),    (0, -0.39, 0.69),    "upperarm_r"),
    ("hand_r",      (0, -0.39, 0.69),    (0, -0.47, 0.69),    "lowerarm_r"),
    # Left leg (+Y side)
    ("thigh_l",     (0, 0.05, 0.38),     (0, 0.05, 0.22),     "pelvis"),
    ("calf_l",      (0, 0.05, 0.22),     (0, 0.05, 0.04),     "thigh_l"),
    ("foot_l",      (0, 0.05, 0.04),     (0.06, 0.05, 0.0),   "calf_l"),
    # Right leg (-Y side)
    ("thigh_r",     (0, -0.05, 0.38),    (0, -0.05, 0.22),    "pelvis"),
    ("calf_r",      (0, -0.05, 0.22),    (0, -0.05, 0.04),    "thigh_r"),
    ("foot_r",      (0, -0.05, 0.04),    (0.06, -0.05, 0.0),  "calf_r"),
]


# --- Pose Definitions ---
# bone_name -> (euler_x, euler_y, euler_z) in degrees, local-space delta rotations.

POSES = {
    "Idle_Tuck": {
        "pelvis":      (0, 0, 0),
        "spine_01":    (15, 0, 0),
        "head":        (-10, 0, 0),
        "thigh_l":     (-35, 0, 0),
        "thigh_r":     (-35, 0, 0),
        "calf_l":      (40, 0, 0),
        "calf_r":      (40, 0, 0),
        "foot_l":      (-5, 0, 0),
        "foot_r":      (-5, 0, 0),
        "upperarm_l":  (10, 0, 40),
        "upperarm_r":  (10, 0, -40),
        "lowerarm_l":  (0, -25, 0),
        "lowerarm_r":  (0, 25, 0),
        "hand_l":      (0, 0, 0),
        "hand_r":      (0, 0, 0),
    },
    "Carve_Left": {
        "pelvis":      (0, 0, -12),
        "spine_01":    (20, 8, 0),
        "head":        (-15, -5, 0),
        "thigh_l":     (-40, -8, 0),
        "thigh_r":     (-25, 5, 0),
        "calf_l":      (50, 0, 0),
        "calf_r":      (30, 0, 0),
        "foot_l":      (-5, -10, 0),
        "foot_r":      (-5, -8, 0),
        "upperarm_l":  (15, 0, 30),
        "upperarm_r":  (5, 0, -50),
        "lowerarm_l":  (0, -30, 0),
        "lowerarm_r":  (0, 20, 0),
        "hand_l":      (0, 0, 0),
        "hand_r":      (0, 0, 0),
    },
    "Carve_Right": {
        "pelvis":      (0, 0, 12),
        "spine_01":    (20, -8, 0),
        "head":        (-15, 5, 0),
        "thigh_l":     (-25, -5, 0),
        "thigh_r":     (-40, 8, 0),
        "calf_l":      (30, 0, 0),
        "calf_r":      (50, 0, 0),
        "foot_l":      (-5, 8, 0),
        "foot_r":      (-5, 10, 0),
        "upperarm_l":  (5, 0, 50),
        "upperarm_r":  (15, 0, -30),
        "lowerarm_l":  (0, -20, 0),
        "lowerarm_r":  (0, 30, 0),
        "hand_l":      (0, 0, 0),
        "hand_r":      (0, 0, 0),
    },
    "Airborne": {
        "pelvis":      (0, 0, 0),
        "spine_01":    (5, 0, 0),
        "head":        (-5, 0, 0),
        "thigh_l":     (-15, 0, -5),
        "thigh_r":     (-15, 0, 5),
        "calf_l":      (15, 0, 0),
        "calf_r":      (15, 0, 0),
        "foot_l":      (10, 0, 0),
        "foot_r":      (10, 0, 0),
        "upperarm_l":  (0, 0, 60),
        "upperarm_r":  (0, 0, -60),
        "lowerarm_l":  (0, -15, 0),
        "lowerarm_r":  (0, 15, 0),
        "hand_l":      (0, 0, 0),
        "hand_r":      (0, 0, 0),
    },
    "Ollie_Prep": {
        "pelvis":      (0, 0, 0),
        "spine_01":    (30, 0, 0),
        "head":        (-20, 0, 0),
        "thigh_l":     (-55, 0, 0),
        "thigh_r":     (-55, 0, 0),
        "calf_l":      (65, 0, 0),
        "calf_r":      (65, 0, 0),
        "foot_l":      (-10, 0, 0),
        "foot_r":      (-10, 0, 0),
        "upperarm_l":  (20, 0, 25),
        "upperarm_r":  (20, 0, -25),
        "lowerarm_l":  (0, -40, 0),
        "lowerarm_r":  (0, 40, 0),
        "hand_l":      (0, 0, 0),
        "hand_r":      (0, 0, 0),
    },
}


def clean_scene():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=True)
    for block in bpy.data.meshes:
        if block.users == 0:
            bpy.data.meshes.remove(block)
    for block in bpy.data.armatures:
        if block.users == 0:
            bpy.data.armatures.remove(block)
    for block in bpy.data.actions:
        if block.users == 0:
            bpy.data.actions.remove(block)


def import_mesh(filepath):
    """Import FBX and return the mesh object."""
    bpy.ops.import_scene.fbx(filepath=filepath)
    for obj in bpy.context.scene.objects:
        if obj.type == 'MESH':
            return obj
    raise RuntimeError("No mesh found in FBX!")


def create_armature(bone_defs):
    """Create an armature from bone definitions. Returns the armature object."""
    bpy.ops.object.armature_add(enter_editmode=True, location=(0, 0, 0))
    armature_obj = bpy.context.active_object
    armature_obj.name = "Armature"
    armature = armature_obj.data
    armature.name = "Skeleton"

    # Remove the default bone
    for bone in armature.edit_bones:
        armature.edit_bones.remove(bone)

    # Create all bones
    bone_map = {}
    for bone_name, head, tail, parent_name in bone_defs:
        bone = armature.edit_bones.new(bone_name)
        bone.head = Vector(head)
        bone.tail = Vector(tail)
        bone.use_connect = False
        bone_map[bone_name] = bone

    # Set parents
    for bone_name, _, _, parent_name in bone_defs:
        if parent_name and parent_name in bone_map:
            bone_map[bone_name].parent = bone_map[parent_name]

    bpy.ops.object.mode_set(mode='OBJECT')
    return armature_obj


def skin_mesh_to_armature(mesh_obj, armature_obj):
    """Parent mesh to armature with automatic weights."""
    # Deselect all, then select mesh and armature
    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)
    armature_obj.select_set(True)
    bpy.context.view_layer.objects.active = armature_obj

    bpy.ops.object.parent_set(type='ARMATURE_AUTO')
    print(f"  Skinned '{mesh_obj.name}' to '{armature_obj.name}' with automatic weights")

    # Verify vertex groups were created
    vg_names = [vg.name for vg in mesh_obj.vertex_groups]
    print(f"  Vertex groups: {vg_names}")


def create_pose_action(armature_obj, pose_name, bone_rotations):
    """Create a single-frame action with the given pose."""
    bpy.context.view_layer.objects.active = armature_obj
    bpy.ops.object.mode_set(mode='POSE')

    # Reset all pose bones
    for pbone in armature_obj.pose.bones:
        pbone.rotation_mode = 'XYZ'
        pbone.rotation_euler = (0, 0, 0)
        pbone.location = (0, 0, 0)
        pbone.scale = (1, 1, 1)

    # Create action
    action = bpy.data.actions.new(name=pose_name)
    if not armature_obj.animation_data:
        armature_obj.animation_data_create()
    armature_obj.animation_data.action = action

    # Apply rotations and keyframe
    for bone_name, (rx, ry, rz) in bone_rotations.items():
        pbone = armature_obj.pose.bones.get(bone_name)
        if pbone is None:
            print(f"  WARNING: Bone '{bone_name}' not found, skipping")
            continue
        pbone.rotation_mode = 'XYZ'
        pbone.rotation_euler = (rad(rx), rad(ry), rad(rz))
        pbone.keyframe_insert(data_path="rotation_euler", frame=0)

    # Keyframe rest bones too
    for pbone in armature_obj.pose.bones:
        if pbone.name not in bone_rotations:
            pbone.rotation_mode = 'XYZ'
            pbone.keyframe_insert(data_path="rotation_euler", frame=0)

    bpy.ops.object.mode_set(mode='OBJECT')
    return action


def export_fbx(armature_obj, output_path, bake_all_actions=False):
    """Export armature + mesh children as FBX."""
    bpy.ops.object.select_all(action='DESELECT')
    armature_obj.select_set(True)
    for child in armature_obj.children:
        child.select_set(True)
    bpy.context.view_layer.objects.active = armature_obj

    bpy.ops.export_scene.fbx(
        filepath=output_path,
        use_selection=True,
        object_types={'ARMATURE', 'MESH'},
        add_leaf_bones=False,
        bake_anim=True,
        bake_anim_use_all_actions=bake_all_actions,
        bake_anim_use_nla_strips=bake_all_actions,
        bake_anim_force_startend_keying=True,
        bake_anim_simplify_factor=0.0,
        mesh_smooth_type='OFF',
        use_mesh_modifiers=True,
        axis_forward='-Y',
        axis_up='Z',
        global_scale=1.0,
        apply_scale_options='FBX_SCALE_ALL',
    )


def main():
    print("=" * 60)
    print("PowderRush Skier Auto-Rig + Pose Generator")
    print("=" * 60)

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # Step 1: Clean and import mesh
    clean_scene()
    print(f"\n[1/5] Importing mesh: {FBX_SOURCE}")
    mesh_obj = import_mesh(FBX_SOURCE)
    print(f"  Mesh: {mesh_obj.name} ({len(mesh_obj.data.vertices)} verts, {len(mesh_obj.data.polygons)} polys)")

    # Step 2: Create armature
    print(f"\n[2/5] Creating armature ({len(BONES)} bones)")
    armature_obj = create_armature(BONES)
    bone_names = [b[0] for b in BONES]
    print(f"  Bones: {bone_names}")

    # Step 3: Skin mesh to armature
    print(f"\n[3/5] Skinning mesh with automatic weights")
    skin_mesh_to_armature(mesh_obj, armature_obj)

    # Step 4: Export rigged mesh (rest pose)
    print(f"\n[4/5] Exporting rigged mesh")
    export_fbx(armature_obj, RIGGED_OUTPUT)
    print(f"  Exported: {RIGGED_OUTPUT}")

    # Step 5: Generate poses and export
    print(f"\n[5/5] Generating {len(POSES)} poses")
    actions = {}
    for pose_name, bone_rotations in POSES.items():
        print(f"\n  Creating pose: {pose_name}")
        action = create_pose_action(armature_obj, pose_name, bone_rotations)
        actions[pose_name] = action

        output_path = os.path.join(OUTPUT_DIR, f"{pose_name}.fbx")
        armature_obj.animation_data.action = action
        export_fbx(armature_obj, output_path)
        print(f"  Exported: {output_path}")

    # Export combined FBX with all actions via NLA
    print(f"\n  Exporting combined FBX with all actions...")
    armature_obj.animation_data.action = None
    for action_name, action in actions.items():
        track = armature_obj.animation_data.nla_tracks.new()
        track.name = action_name
        strip = track.strips.new(action_name, int(action.frame_range[0]), action)
        strip.name = action_name

    combined_path = os.path.join(OUTPUT_DIR, "SkierPoses_All.fbx")
    export_fbx(armature_obj, combined_path, bake_all_actions=True)
    print(f"  Exported: {combined_path}")

    print("\n" + "=" * 60)
    print("Done!")
    print(f"  Rigged mesh: {RIGGED_OUTPUT}")
    print(f"  {len(POSES)} poses in: {OUTPUT_DIR}")
    print("=" * 60)


if __name__ == "__main__":
    main()
