// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "mmd_pmx.h"
#include "kaitai/exceptions.h"

mmd_pmx_t::mmd_pmx_t(kaitai::kstream *p__io, kaitai::kstruct *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = this;
    m_header = nullptr;
    m_vertices = nullptr;
    m_faces = nullptr;
    m_textures = nullptr;
    m_materials = nullptr;
    m_bones = nullptr;
    m_morphs = nullptr;
    m_frames = nullptr;
    m_rigid_bodies = nullptr;
    m_joints = nullptr;
    _read();
}

void mmd_pmx_t::_read()
{
    m_header = std::make_unique<header_t>(m__io, this, m__root);
    m_num_vertices = m__io->read_u4le();
    m_vertices = std::make_unique<std::vector<std::unique_ptr<vertex_t>>>();
    const int l_vertices = num_vertices();
    for (int i = 0; i < l_vertices; i++)
    {
        m_vertices->push_back(std::make_unique<vertex_t>(m__io, this, m__root));
    }
    m_face_num_vertices = m__io->read_u4le();
    m_faces = std::make_unique<std::vector<std::unique_ptr<face_t>>>();
    const int l_faces = (face_num_vertices() / 3);
    for (int i = 0; i < l_faces; i++)
    {
        m_faces->push_back(std::make_unique<face_t>(m__io, this, m__root));
    }
    m_num_textures = m__io->read_u4le();
    m_textures = std::make_unique<std::vector<std::unique_ptr<texture_t>>>();
    const int l_textures = num_textures();
    for (int i = 0; i < l_textures; i++)
    {
        m_textures->push_back(std::make_unique<texture_t>(m__io, this, m__root));
    }
    m_num_materials = m__io->read_u4le();
    m_materials = std::make_unique<std::vector<std::unique_ptr<material_t>>>();
    const int l_materials = num_materials();
    for (int i = 0; i < l_materials; i++)
    {
        m_materials->push_back(std::make_unique<material_t>(m__io, this, m__root));
    }
    m_num_bones = m__io->read_u4le();
    m_bones = std::make_unique<std::vector<std::unique_ptr<bone_t>>>();
    const int l_bones = num_bones();
    for (int i = 0; i < l_bones; i++)
    {
        m_bones->push_back(std::make_unique<bone_t>(m__io, this, m__root));
    }
    m_num_morphs = m__io->read_u4le();
    m_morphs = std::make_unique<std::vector<std::unique_ptr<morph_t>>>();
    const int l_morphs = num_morphs();
    for (int i = 0; i < l_morphs; i++)
    {
        m_morphs->push_back(std::make_unique<morph_t>(m__io, this, m__root));
    }
    m_num_frames = m__io->read_u4le();
    m_frames = std::make_unique<std::vector<std::unique_ptr<frame_t>>>();
    const int l_frames = num_frames();
    for (int i = 0; i < l_frames; i++)
    {
        m_frames->push_back(std::make_unique<frame_t>(m__io, this, m__root));
    }
    m_num_rigid_bodies = m__io->read_u4le();
    m_rigid_bodies = std::make_unique<std::vector<std::unique_ptr<rigid_body_t>>>();
    const int l_rigid_bodies = num_rigid_bodies();
    for (int i = 0; i < l_rigid_bodies; i++)
    {
        m_rigid_bodies->push_back(std::make_unique<rigid_body_t>(m__io, this, m__root));
    }
    m_num_joints = m__io->read_u4le();
    m_joints = std::make_unique<std::vector<std::unique_ptr<joint_t>>>();
    const int l_joints = num_joints();
    for (int i = 0; i < l_joints; i++)
    {
        m_joints->push_back(std::make_unique<joint_t>(m__io, this, m__root));
    }
}

mmd_pmx_t::~mmd_pmx_t()
{
    _clean_up();
}

void mmd_pmx_t::_clean_up()
{
}

mmd_pmx_t::bone_ik_link_t::bone_ik_link_t(kaitai::kstream *p__io, mmd_pmx_t::bone_ik_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_index = nullptr;
    m_lower_limitation_angle = nullptr;
    m_upper_limitation_angle = nullptr;
    _read();
}

void mmd_pmx_t::bone_ik_link_t::_read()
{
    m_index = std::make_unique<sized_index_t>(_root()->header()->bone_index_size(), m__io, this, m__root);
    m_angle_limitation = m__io->read_u1();
    n_lower_limitation_angle = true;
    if (angle_limitation() == 1)
    {
        n_lower_limitation_angle = false;
        m_lower_limitation_angle = std::make_unique<vec3_t>(m__io, this, m__root);
    }
    n_upper_limitation_angle = true;
    if (angle_limitation() == 1)
    {
        n_upper_limitation_angle = false;
        m_upper_limitation_angle = std::make_unique<vec3_t>(m__io, this, m__root);
    }
}

mmd_pmx_t::bone_ik_link_t::~bone_ik_link_t()
{
    _clean_up();
}

void mmd_pmx_t::bone_ik_link_t::_clean_up()
{
    if (!n_lower_limitation_angle)
    {
    }
    if (!n_upper_limitation_angle)
    {
    }
}

mmd_pmx_t::vec4_t::vec4_t(kaitai::kstream *p__io, kaitai::kstruct *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void mmd_pmx_t::vec4_t::_read()
{
    m_x = m__io->read_f4le();
    m_y = m__io->read_f4le();
    m_z = m__io->read_f4le();
    m_w = m__io->read_f4le();
}

mmd_pmx_t::vec4_t::~vec4_t()
{
    _clean_up();
}

void mmd_pmx_t::vec4_t::_clean_up()
{
}

mmd_pmx_t::bdef1_weights_t::bdef1_weights_t(kaitai::kstream *p__io, mmd_pmx_t::vertex_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_bone_index = nullptr;
    _read();
}

void mmd_pmx_t::bdef1_weights_t::_read()
{
    m_bone_index = std::make_unique<sized_index_t>(_root()->header()->bone_index_size(), m__io, this, m__root);
}

mmd_pmx_t::bdef1_weights_t::~bdef1_weights_t()
{
    _clean_up();
}

void mmd_pmx_t::bdef1_weights_t::_clean_up()
{
}

mmd_pmx_t::vertex_t::vertex_t(kaitai::kstream *p__io, mmd_pmx_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_position = nullptr;
    m_normal = nullptr;
    m_uv = nullptr;
    m_additional_uvs = nullptr;
    _read();
}

void mmd_pmx_t::vertex_t::_read()
{
    m_position = std::make_unique<vec3_t>(m__io, this, m__root);
    m_normal = std::make_unique<vec3_t>(m__io, this, m__root);
    m_uv = std::make_unique<vec2_t>(m__io, this, m__root);
    m_additional_uvs = std::make_unique<std::vector<std::unique_ptr<vec4_t>>>();
    const int l_additional_uvs = _root()->header()->additional_uv_count();
    for (int i = 0; i < l_additional_uvs; i++)
    {
        m_additional_uvs->push_back(std::make_unique<vec4_t>(m__io, this, m__root));
    }
    m_type = static_cast<mmd_pmx_t::bone_type_t>(m__io->read_u1());
    n_skin_weights = true;
    switch (type())
    {
    case mmd_pmx_t::BONE_TYPE_BDEF4:
    {
        n_skin_weights = false;
        m_skin_weights = std::make_unique<bdef4_weights_t>(m__io, this, m__root);
        break;
    }
    case mmd_pmx_t::BONE_TYPE_BDEF1:
    {
        n_skin_weights = false;
        m_skin_weights = std::make_unique<bdef1_weights_t>(m__io, this, m__root);
        break;
    }
    case mmd_pmx_t::BONE_TYPE_BDEF2:
    {
        n_skin_weights = false;
        m_skin_weights = std::make_unique<bdef2_weights_t>(m__io, this, m__root);
        break;
    }
    case mmd_pmx_t::BONE_TYPE_QDEF:
    {
        n_skin_weights = false;
        m_skin_weights = std::make_unique<qdef_weights_t>(m__io, this, m__root);
        break;
    }
    case mmd_pmx_t::BONE_TYPE_SDEF:
    {
        n_skin_weights = false;
        m_skin_weights = std::make_unique<sdef_weights_t>(m__io, this, m__root);
        break;
    }
    }
    m_edge_ratio = m__io->read_f4le();
}

mmd_pmx_t::vertex_t::~vertex_t()
{
    _clean_up();
}

void mmd_pmx_t::vertex_t::_clean_up()
{
    if (!n_skin_weights)
    {
    }
}

mmd_pmx_t::material_morph_element_t::material_morph_element_t(kaitai::kstream *p__io, mmd_pmx_t::morph_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_index = nullptr;
    m_diffuse = nullptr;
    m_specular = nullptr;
    m_ambient = nullptr;
    m_edge_color = nullptr;
    m_texture_color = nullptr;
    m_sphere_texture_color = nullptr;
    m_toon_color = nullptr;
    _read();
}

void mmd_pmx_t::material_morph_element_t::_read()
{
    m_index = std::make_unique<sized_index_t>(_root()->header()->material_index_size(), m__io, this, m__root);
    m_type = m__io->read_u1();
    m_diffuse = std::make_unique<color4_t>(m__io, this, m__root);
    m_specular = std::make_unique<color3_t>(m__io, this, m__root);
    m_shininess = m__io->read_f4le();
    m_ambient = std::make_unique<color3_t>(m__io, this, m__root);
    m_edge_color = std::make_unique<color4_t>(m__io, this, m__root);
    m_edge_size = m__io->read_f4le();
    m_texture_color = std::make_unique<color4_t>(m__io, this, m__root);
    m_sphere_texture_color = std::make_unique<color4_t>(m__io, this, m__root);
    m_toon_color = std::make_unique<color4_t>(m__io, this, m__root);
}

mmd_pmx_t::material_morph_element_t::~material_morph_element_t()
{
    _clean_up();
}

void mmd_pmx_t::material_morph_element_t::_clean_up()
{
}

mmd_pmx_t::bone_morph_element_t::bone_morph_element_t(kaitai::kstream *p__io, mmd_pmx_t::morph_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_index = nullptr;
    m_position = nullptr;
    m_rotation = nullptr;
    _read();
}

void mmd_pmx_t::bone_morph_element_t::_read()
{
    m_index = std::make_unique<sized_index_t>(_root()->header()->bone_index_size(), m__io, this, m__root);
    m_position = std::make_unique<vec3_t>(m__io, this, m__root);
    m_rotation = std::make_unique<vec4_t>(m__io, this, m__root);
}

mmd_pmx_t::bone_morph_element_t::~bone_morph_element_t()
{
    _clean_up();
}

void mmd_pmx_t::bone_morph_element_t::_clean_up()
{
}

mmd_pmx_t::color4_t::color4_t(kaitai::kstream *p__io, kaitai::kstruct *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void mmd_pmx_t::color4_t::_read()
{
    m_r = m__io->read_f4le();
    m_g = m__io->read_f4le();
    m_b = m__io->read_f4le();
    m_a = m__io->read_f4le();
}

mmd_pmx_t::color4_t::~color4_t()
{
    _clean_up();
}

void mmd_pmx_t::color4_t::_clean_up()
{
}

mmd_pmx_t::bone_grant_t::bone_grant_t(kaitai::kstream *p__io, mmd_pmx_t::bone_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_parent_index = nullptr;
    f_local = false;
    f_affect_rotation = false;
    f_affect_position = false;
    _read();
}

void mmd_pmx_t::bone_grant_t::_read()
{
    m_parent_index = std::make_unique<sized_index_t>(_root()->header()->bone_index_size(), m__io, this, m__root);
    m_ratio = m__io->read_f4le();
}

mmd_pmx_t::bone_grant_t::~bone_grant_t()
{
    _clean_up();
}

void mmd_pmx_t::bone_grant_t::_clean_up()
{
}

bool mmd_pmx_t::bone_grant_t::local()
{
    if (f_local)
        return m_local;
    m_local = _parent()->add_local_deform();
    f_local = true;
    return m_local;
}

bool mmd_pmx_t::bone_grant_t::affect_rotation()
{
    if (f_affect_rotation)
        return m_affect_rotation;
    m_affect_rotation = _parent()->inherit_rotation();
    f_affect_rotation = true;
    return m_affect_rotation;
}

bool mmd_pmx_t::bone_grant_t::affect_position()
{
    if (f_affect_position)
        return m_affect_position;
    m_affect_position = _parent()->inherit_translation();
    f_affect_position = true;
    return m_affect_position;
}

mmd_pmx_t::vertex_morph_element_t::vertex_morph_element_t(kaitai::kstream *p__io, mmd_pmx_t::morph_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_index = nullptr;
    m_position = nullptr;
    _read();
}

void mmd_pmx_t::vertex_morph_element_t::_read()
{
    m_index = std::make_unique<sized_index_t>(_root()->header()->vertex_index_size(), m__io, this, m__root);
    m_position = std::make_unique<vec3_t>(m__io, this, m__root);
}

mmd_pmx_t::vertex_morph_element_t::~vertex_morph_element_t()
{
    _clean_up();
}

void mmd_pmx_t::vertex_morph_element_t::_clean_up()
{
}

mmd_pmx_t::sdef_weights_t::sdef_weights_t(kaitai::kstream *p__io, mmd_pmx_t::vertex_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_bone_indices = nullptr;
    m_c = nullptr;
    m_r0 = nullptr;
    m_r1 = nullptr;
    f_weights = false;
    _read();
}

void mmd_pmx_t::sdef_weights_t::_read()
{
    m_bone_indices = std::make_unique<std::vector<std::unique_ptr<sized_index_t>>>();
    const int l_bone_indices = 2;
    for (int i = 0; i < l_bone_indices; i++)
    {
        m_bone_indices->push_back(std::make_unique<sized_index_t>(_root()->header()->bone_index_size(), m__io, this, m__root));
    }
    m_weight1 = m__io->read_f4le();
    m_c = std::make_unique<vec3_t>(m__io, this, m__root);
    m_r0 = std::make_unique<vec3_t>(m__io, this, m__root);
    m_r1 = std::make_unique<vec3_t>(m__io, this, m__root);
}

mmd_pmx_t::sdef_weights_t::~sdef_weights_t()
{
    _clean_up();
}

void mmd_pmx_t::sdef_weights_t::_clean_up()
{
}

std::vector<double> *mmd_pmx_t::sdef_weights_t::weights()
{
    if (f_weights)
        return m_weights.get();
    m_weights = std::make_unique<std::vector<double>>(std::initializer_list<double>{weight1(), (1.0 - weight1())});
    f_weights = true;
    return m_weights.get();
}

mmd_pmx_t::text_t::text_t(kaitai::kstream *p__io, kaitai::kstruct *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void mmd_pmx_t::text_t::_read()
{
    n_value = true;
    switch (_root()->header()->encoding())
    {
    case 0:
    {
        n_value = false;
        m_value = std::make_unique<text_utf16_t>(m__io, this, m__root);
        break;
    }
    case 1:
    {
        n_value = false;
        m_value = std::make_unique<text_utf8_t>(m__io, this, m__root);
        break;
    }
    }
}

mmd_pmx_t::text_t::~text_t()
{
    _clean_up();
}

void mmd_pmx_t::text_t::_clean_up()
{
    if (!n_value)
    {
    }
}

mmd_pmx_t::group_morph_element_t::group_morph_element_t(kaitai::kstream *p__io, mmd_pmx_t::morph_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_index = nullptr;
    _read();
}

void mmd_pmx_t::group_morph_element_t::_read()
{
    m_index = std::make_unique<sized_index_t>(_root()->header()->morph_index_size(), m__io, this, m__root);
    m_ratio = m__io->read_f4le();
}

mmd_pmx_t::group_morph_element_t::~group_morph_element_t()
{
    _clean_up();
}

void mmd_pmx_t::group_morph_element_t::_clean_up()
{
}

mmd_pmx_t::joint_t::joint_t(kaitai::kstream *p__io, mmd_pmx_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_name = nullptr;
    m_english_name = nullptr;
    m_rigid_body_indices = nullptr;
    m_position = nullptr;
    m_rotation = nullptr;
    m_position_constraint_lower = nullptr;
    m_position_constraint_upper = nullptr;
    m_rotation_constraint_lower = nullptr;
    m_rotation_constraint_upper = nullptr;
    m_spring_position = nullptr;
    m_spring_rotation = nullptr;
    _read();
}

void mmd_pmx_t::joint_t::_read()
{
    m_name = std::make_unique<text_t>(m__io, this, m__root);
    m_english_name = std::make_unique<text_t>(m__io, this, m__root);
    m_type = static_cast<mmd_pmx_t::joint_type_t>(m__io->read_u1());
    m_rigid_body_indices = std::make_unique<std::vector<std::unique_ptr<sized_index_t>>>();
    const int l_rigid_body_indices = 2;
    for (int i = 0; i < l_rigid_body_indices; i++)
    {
        m_rigid_body_indices->push_back(std::make_unique<sized_index_t>(_root()->header()->rigid_body_index_size(), m__io, this, m__root));
    }
    m_position = std::make_unique<vec3_t>(m__io, this, m__root);
    m_rotation = std::make_unique<vec3_t>(m__io, this, m__root);
    m_position_constraint_lower = std::make_unique<vec3_t>(m__io, this, m__root);
    m_position_constraint_upper = std::make_unique<vec3_t>(m__io, this, m__root);
    m_rotation_constraint_lower = std::make_unique<vec3_t>(m__io, this, m__root);
    m_rotation_constraint_upper = std::make_unique<vec3_t>(m__io, this, m__root);
    m_spring_position = std::make_unique<vec3_t>(m__io, this, m__root);
    m_spring_rotation = std::make_unique<vec3_t>(m__io, this, m__root);
}

mmd_pmx_t::joint_t::~joint_t()
{
    _clean_up();
}

void mmd_pmx_t::joint_t::_clean_up()
{
}

mmd_pmx_t::bone_t::bone_t(kaitai::kstream *p__io, mmd_pmx_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_name = nullptr;
    m_english_name = nullptr;
    m_position = nullptr;
    m_parent_index = nullptr;
    m_connect_index = nullptr;
    m_offset_position = nullptr;
    m_grant = nullptr;
    m_fixed_axis = nullptr;
    m_local_x_vector = nullptr;
    m_local_z_vector = nullptr;
    m_ik = nullptr;
    _read();
}

void mmd_pmx_t::bone_t::_read()
{
    m_name = std::make_unique<text_t>(m__io, this, m__root);
    m_english_name = std::make_unique<text_t>(m__io, this, m__root);
    m_position = std::make_unique<vec3_t>(m__io, this, m__root);
    m_parent_index = std::make_unique<sized_index_t>(_root()->header()->bone_index_size(), m__io, this, m__root);
    m_transformation_class = m__io->read_u4le();
    m_indexed_tail_position = m__io->read_bits_int_le(1);
    m_rotatable = m__io->read_bits_int_le(1);
    m_translatable = m__io->read_bits_int_le(1);
    m_visible = m__io->read_bits_int_le(1);
    m_enabled = m__io->read_bits_int_le(1);
    m_has_ik = m__io->read_bits_int_le(1);
    m_unknown6 = m__io->read_bits_int_le(1);
    m_add_local_deform = m__io->read_bits_int_le(1);
    m_inherit_rotation = m__io->read_bits_int_le(1);
    m_inherit_translation = m__io->read_bits_int_le(1);
    m_has_fixed_axis = m__io->read_bits_int_le(1);
    m_has_local_axes = m__io->read_bits_int_le(1);
    m_physics_after_deform = m__io->read_bits_int_le(1);
    m_external_parent_deform = m__io->read_bits_int_le(1);
    m_reserved = m__io->read_bits_int_le(2);
    m__io->align_to_byte();
    n_connect_index = true;
    if (indexed_tail_position())
    {
        n_connect_index = false;
        m_connect_index = std::make_unique<sized_index_t>(_root()->header()->bone_index_size(), m__io, this, m__root);
    }
    n_offset_position = true;
    if (!(indexed_tail_position()))
    {
        n_offset_position = false;
        m_offset_position = std::make_unique<vec3_t>(m__io, this, m__root);
    }
    n_grant = true;
    if (((inherit_rotation()) || (inherit_translation())))
    {
        n_grant = false;
        m_grant = std::make_unique<bone_grant_t>(m__io, this, m__root);
    }
    n_fixed_axis = true;
    if (has_fixed_axis())
    {
        n_fixed_axis = false;
        m_fixed_axis = std::make_unique<vec3_t>(m__io, this, m__root);
    }
    n_local_x_vector = true;
    if (has_local_axes())
    {
        n_local_x_vector = false;
        m_local_x_vector = std::make_unique<vec3_t>(m__io, this, m__root);
    }
    n_local_z_vector = true;
    if (has_local_axes())
    {
        n_local_z_vector = false;
        m_local_z_vector = std::make_unique<vec3_t>(m__io, this, m__root);
    }
    n_key = true;
    if (external_parent_deform())
    {
        n_key = false;
        m_key = m__io->read_u4le();
    }
    n_ik = true;
    if (has_ik())
    {
        n_ik = false;
        m_ik = std::make_unique<bone_ik_t>(m__io, this, m__root);
    }
}

mmd_pmx_t::bone_t::~bone_t()
{
    _clean_up();
}

void mmd_pmx_t::bone_t::_clean_up()
{
    if (!n_connect_index)
    {
    }
    if (!n_offset_position)
    {
    }
    if (!n_grant)
    {
    }
    if (!n_fixed_axis)
    {
    }
    if (!n_local_x_vector)
    {
    }
    if (!n_local_z_vector)
    {
    }
    if (!n_key)
    {
    }
    if (!n_ik)
    {
    }
}

mmd_pmx_t::frame_t::frame_t(kaitai::kstream *p__io, mmd_pmx_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_name = nullptr;
    m_english_name = nullptr;
    m_elements = nullptr;
    _read();
}

void mmd_pmx_t::frame_t::_read()
{
    m_name = std::make_unique<text_t>(m__io, this, m__root);
    m_english_name = std::make_unique<text_t>(m__io, this, m__root);
    m_type = m__io->read_u1();
    m_num_elements = m__io->read_u4le();
    m_elements = std::make_unique<std::vector<std::unique_ptr<frame_element_t>>>();
    const int l_elements = num_elements();
    for (int i = 0; i < l_elements; i++)
    {
        m_elements->push_back(std::make_unique<frame_element_t>(m__io, this, m__root));
    }
}

mmd_pmx_t::frame_t::~frame_t()
{
    _clean_up();
}

void mmd_pmx_t::frame_t::_clean_up()
{
}

mmd_pmx_t::qdef_weights_t::qdef_weights_t(kaitai::kstream *p__io, mmd_pmx_t::vertex_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_bone_indices = nullptr;
    m_weights = nullptr;
    _read();
}

void mmd_pmx_t::qdef_weights_t::_read()
{
    m_bone_indices = std::make_unique<std::vector<std::unique_ptr<sized_index_t>>>();
    const int l_bone_indices = 4;
    for (int i = 0; i < l_bone_indices; i++)
    {
        m_bone_indices->push_back(std::make_unique<sized_index_t>(_root()->header()->bone_index_size(), m__io, this, m__root));
    }
    m_weights = std::make_unique<std::vector<float>>();
    const int l_weights = 4;
    for (int i = 0; i < l_weights; i++)
    {
        m_weights->push_back(std::move(m__io->read_f4le()));
    }
}

mmd_pmx_t::qdef_weights_t::~qdef_weights_t()
{
    _clean_up();
}

void mmd_pmx_t::qdef_weights_t::_clean_up()
{
}

mmd_pmx_t::face_t::face_t(kaitai::kstream *p__io, mmd_pmx_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_indices = nullptr;
    _read();
}

void mmd_pmx_t::face_t::_read()
{
    m_indices = std::make_unique<std::vector<std::unique_ptr<sized_index_t>>>();
    const int l_indices = 3;
    for (int i = 0; i < l_indices; i++)
    {
        m_indices->push_back(std::make_unique<sized_index_t>(_root()->header()->vertex_index_size(), m__io, this, m__root));
    }
}

mmd_pmx_t::face_t::~face_t()
{
    _clean_up();
}

void mmd_pmx_t::face_t::_clean_up()
{
}

mmd_pmx_t::color3_t::color3_t(kaitai::kstream *p__io, kaitai::kstruct *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void mmd_pmx_t::color3_t::_read()
{
    m_r = m__io->read_f4le();
    m_g = m__io->read_f4le();
    m_b = m__io->read_f4le();
}

mmd_pmx_t::color3_t::~color3_t()
{
    _clean_up();
}

void mmd_pmx_t::color3_t::_clean_up()
{
}

mmd_pmx_t::morph_t::morph_t(kaitai::kstream *p__io, mmd_pmx_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_name = nullptr;
    m_english_name = nullptr;
    m_elements = nullptr;
    _read();
}

void mmd_pmx_t::morph_t::_read()
{
    m_name = std::make_unique<text_t>(m__io, this, m__root);
    m_english_name = std::make_unique<text_t>(m__io, this, m__root);
    m_panel = m__io->read_u1();
    m_type = static_cast<mmd_pmx_t::morph_type_t>(m__io->read_u1());
    m_num_elements = m__io->read_u4le();
    m_elements = std::make_unique<std::vector<std::unique_ptr<kaitai::kstruct>>>();
    const int l_elements = num_elements();
    for (int i = 0; i < l_elements; i++)
    {
        switch (type())
        {
        case mmd_pmx_t::MORPH_TYPE_ADDITIONAL_UV4:
        {
            m_elements->push_back(std::make_unique<uv_morph_element_t>(m__io, this, m__root));
            break;
        }
        case mmd_pmx_t::MORPH_TYPE_ADDITIONAL_UV1:
        {
            m_elements->push_back(std::make_unique<uv_morph_element_t>(m__io, this, m__root));
            break;
        }
        case mmd_pmx_t::MORPH_TYPE_VERTEX:
        {
            m_elements->push_back(std::make_unique<vertex_morph_element_t>(m__io, this, m__root));
            break;
        }
        case mmd_pmx_t::MORPH_TYPE_MATERIAL:
        {
            m_elements->push_back(std::make_unique<material_morph_element_t>(m__io, this, m__root));
            break;
        }
        case mmd_pmx_t::MORPH_TYPE_FLIP:
        {
            m_elements->push_back(std::make_unique<group_morph_element_t>(m__io, this, m__root));
            break;
        }
        case mmd_pmx_t::MORPH_TYPE_ADDITIONAL_UV3:
        {
            m_elements->push_back(std::make_unique<uv_morph_element_t>(m__io, this, m__root));
            break;
        }
        case mmd_pmx_t::MORPH_TYPE_ADDITIONAL_UV2:
        {
            m_elements->push_back(std::make_unique<uv_morph_element_t>(m__io, this, m__root));
            break;
        }
        case mmd_pmx_t::MORPH_TYPE_UV:
        {
            m_elements->push_back(std::make_unique<uv_morph_element_t>(m__io, this, m__root));
            break;
        }
        case mmd_pmx_t::MORPH_TYPE_GROUP:
        {
            m_elements->push_back(std::make_unique<group_morph_element_t>(m__io, this, m__root));
            break;
        }
        case mmd_pmx_t::MORPH_TYPE_IMPULSE:
        {
            m_elements->push_back(std::make_unique<impulse_morph_element_t>(m__io, this, m__root));
            break;
        }
        case mmd_pmx_t::MORPH_TYPE_BONE:
        {
            m_elements->push_back(std::make_unique<bone_morph_element_t>(m__io, this, m__root));
            break;
        }
        }
    }
}

mmd_pmx_t::morph_t::~morph_t()
{
    _clean_up();
}

void mmd_pmx_t::morph_t::_clean_up()
{
}

mmd_pmx_t::vec2_t::vec2_t(kaitai::kstream *p__io, mmd_pmx_t::vertex_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void mmd_pmx_t::vec2_t::_read()
{
    m_x = m__io->read_f4le();
    m_y = m__io->read_f4le();
}

mmd_pmx_t::vec2_t::~vec2_t()
{
    _clean_up();
}

void mmd_pmx_t::vec2_t::_clean_up()
{
}

mmd_pmx_t::text_utf16_t::text_utf16_t(kaitai::kstream *p__io, mmd_pmx_t::text_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void mmd_pmx_t::text_utf16_t::_read()
{
    m_length = m__io->read_u4le();
    m_value = kaitai::kstream::bytes_to_str(m__io->read_bytes(length()), std::string("UTF-16LE"));
}

mmd_pmx_t::text_utf16_t::~text_utf16_t()
{
    _clean_up();
}

void mmd_pmx_t::text_utf16_t::_clean_up()
{
}

mmd_pmx_t::frame_element_t::frame_element_t(kaitai::kstream *p__io, mmd_pmx_t::frame_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void mmd_pmx_t::frame_element_t::_read()
{
    m_target = m__io->read_u1();
    switch (target())
    {
    case 0:
    {
        m_index = std::make_unique<sized_index_t>(_root()->header()->bone_index_size(), m__io, this, m__root);
        break;
    }
    default:
    {
        m_index = std::make_unique<sized_index_t>(_root()->header()->morph_index_size(), m__io, this, m__root);
        break;
    }
    }
}

mmd_pmx_t::frame_element_t::~frame_element_t()
{
    _clean_up();
}

void mmd_pmx_t::frame_element_t::_clean_up()
{
}

mmd_pmx_t::impulse_morph_element_t::impulse_morph_element_t(kaitai::kstream *p__io, mmd_pmx_t::morph_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_rigid_body_index = nullptr;
    m_translational_velocity = nullptr;
    m_angular_velocity = nullptr;
    _read();
}

void mmd_pmx_t::impulse_morph_element_t::_read()
{
    m_rigid_body_index = std::make_unique<sized_index_t>(_root()->header()->rigid_body_index_size(), m__io, this, m__root);
    m_local = m__io->read_u1();
    m_translational_velocity = std::make_unique<vec3_t>(m__io, this, m__root);
    m_angular_velocity = std::make_unique<vec3_t>(m__io, this, m__root);
}

mmd_pmx_t::impulse_morph_element_t::~impulse_morph_element_t()
{
    _clean_up();
}

void mmd_pmx_t::impulse_morph_element_t::_clean_up()
{
}

mmd_pmx_t::bone_ik_t::bone_ik_t(kaitai::kstream *p__io, mmd_pmx_t::bone_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_effector = nullptr;
    m_links = nullptr;
    _read();
}

void mmd_pmx_t::bone_ik_t::_read()
{
    m_effector = std::make_unique<sized_index_t>(_root()->header()->bone_index_size(), m__io, this, m__root);
    m_iteration = m__io->read_u4le();
    m_max_angle = m__io->read_f4le();
    m_num_links = m__io->read_u4le();
    m_links = std::make_unique<std::vector<std::unique_ptr<bone_ik_link_t>>>();
    const int l_links = num_links();
    for (int i = 0; i < l_links; i++)
    {
        m_links->push_back(std::make_unique<bone_ik_link_t>(m__io, this, m__root));
    }
}

mmd_pmx_t::bone_ik_t::~bone_ik_t()
{
    _clean_up();
}

void mmd_pmx_t::bone_ik_t::_clean_up()
{
}

mmd_pmx_t::text_utf8_t::text_utf8_t(kaitai::kstream *p__io, mmd_pmx_t::text_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void mmd_pmx_t::text_utf8_t::_read()
{
    m_length = m__io->read_u4le();
    m_value = kaitai::kstream::bytes_to_str(m__io->read_bytes(length()), std::string("UTF-8"));
}

mmd_pmx_t::text_utf8_t::~text_utf8_t()
{
    _clean_up();
}

void mmd_pmx_t::text_utf8_t::_clean_up()
{
}

mmd_pmx_t::uv_morph_element_t::uv_morph_element_t(kaitai::kstream *p__io, mmd_pmx_t::morph_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_index = nullptr;
    m_uv = nullptr;
    _read();
}

void mmd_pmx_t::uv_morph_element_t::_read()
{
    m_index = std::make_unique<sized_index_t>(_root()->header()->vertex_index_size(), m__io, this, m__root);
    m_uv = std::make_unique<vec4_t>(m__io, this, m__root);
}

mmd_pmx_t::uv_morph_element_t::~uv_morph_element_t()
{
    _clean_up();
}

void mmd_pmx_t::uv_morph_element_t::_clean_up()
{
}

mmd_pmx_t::bdef2_weights_t::bdef2_weights_t(kaitai::kstream *p__io, mmd_pmx_t::vertex_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_bone_indices = nullptr;
    f_weights = false;
    _read();
}

void mmd_pmx_t::bdef2_weights_t::_read()
{
    m_bone_indices = std::make_unique<std::vector<std::unique_ptr<sized_index_t>>>();
    const int l_bone_indices = 2;
    for (int i = 0; i < l_bone_indices; i++)
    {
        m_bone_indices->push_back(std::make_unique<sized_index_t>(_root()->header()->bone_index_size(), m__io, this, m__root));
    }
    m_weight1 = m__io->read_f4le();
}

mmd_pmx_t::bdef2_weights_t::~bdef2_weights_t()
{
    _clean_up();
}

void mmd_pmx_t::bdef2_weights_t::_clean_up()
{
}

std::vector<double> *mmd_pmx_t::bdef2_weights_t::weights()
{
    if (f_weights)
        return m_weights.get();
    m_weights = std::make_unique<std::vector<double>>(std::initializer_list<double>{weight1(), (1.0 - weight1())});
    f_weights = true;
    return m_weights.get();
}

mmd_pmx_t::header_t::header_t(kaitai::kstream *p__io, mmd_pmx_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_model_name = nullptr;
    m_english_model_name = nullptr;
    m_comment = nullptr;
    m_english_comment = nullptr;
    _read();
}

void mmd_pmx_t::header_t::_read()
{
    m_magic = m__io->read_bytes(4);
    if (!(magic() == std::string("\x50\x4D\x58\x20", 4)))
    {
        throw kaitai::validation_not_equal_error<std::string>(std::string("\x50\x4D\x58\x20", 4), magic(), _io(), std::string("/types/header/seq/0"));
    }
    m_version = m__io->read_f4le();
    m_header_size = m__io->read_u1();
    m_encoding = m__io->read_u1();
    m_additional_uv_count = m__io->read_u1();
    m_vertex_index_size = m__io->read_u1();
    m_texture_index_size = m__io->read_u1();
    m_material_index_size = m__io->read_u1();
    m_bone_index_size = m__io->read_u1();
    m_morph_index_size = m__io->read_u1();
    m_rigid_body_index_size = m__io->read_u1();
    m_model_name = std::make_unique<text_t>(m__io, this, m__root);
    m_english_model_name = std::make_unique<text_t>(m__io, this, m__root);
    m_comment = std::make_unique<text_t>(m__io, this, m__root);
    m_english_comment = std::make_unique<text_t>(m__io, this, m__root);
}

mmd_pmx_t::header_t::~header_t()
{
    _clean_up();
}

void mmd_pmx_t::header_t::_clean_up()
{
}

mmd_pmx_t::common_toon_index_t::common_toon_index_t(kaitai::kstream *p__io, mmd_pmx_t::material_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void mmd_pmx_t::common_toon_index_t::_read()
{
    m_value = m__io->read_u1();
}

mmd_pmx_t::common_toon_index_t::~common_toon_index_t()
{
    _clean_up();
}

void mmd_pmx_t::common_toon_index_t::_clean_up()
{
}

mmd_pmx_t::material_t::material_t(kaitai::kstream *p__io, mmd_pmx_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_name = nullptr;
    m_english_name = nullptr;
    m_diffuse = nullptr;
    m_specular = nullptr;
    m_ambient = nullptr;
    m_edge_color = nullptr;
    m_texture_index = nullptr;
    m_sphere_texture_index = nullptr;
    m_comment = nullptr;
    _read();
}

void mmd_pmx_t::material_t::_read()
{
    m_name = std::make_unique<text_t>(m__io, this, m__root);
    m_english_name = std::make_unique<text_t>(m__io, this, m__root);
    m_diffuse = std::make_unique<color4_t>(m__io, this, m__root);
    m_specular = std::make_unique<color3_t>(m__io, this, m__root);
    m_shininess = m__io->read_f4le();
    m_ambient = std::make_unique<color3_t>(m__io, this, m__root);
    m_no_cull = m__io->read_bits_int_le(1);
    m_ground_shadow = m__io->read_bits_int_le(1);
    m_cast_shadow = m__io->read_bits_int_le(1);
    m_receive_shadow = m__io->read_bits_int_le(1);
    m_outlined = m__io->read_bits_int_le(1);
    m_uses_vertex_color = m__io->read_bits_int_le(1);
    m_render_points = m__io->read_bits_int_le(1);
    m_render_lines = m__io->read_bits_int_le(1);
    m__io->align_to_byte();
    m_edge_color = std::make_unique<color4_t>(m__io, this, m__root);
    m_edge_size = m__io->read_f4le();
    m_texture_index = std::make_unique<sized_index_t>(_root()->header()->texture_index_size(), m__io, this, m__root);
    m_sphere_texture_index = std::make_unique<sized_index_t>(_root()->header()->texture_index_size(), m__io, this, m__root);
    m_sphere_op_mode = static_cast<mmd_pmx_t::sphere_op_mode_t>(m__io->read_u1());
    m_is_common_toon = m__io->read_u1();
    n_toon_index = true;
    switch (is_common_toon())
    {
    case 0:
    {
        n_toon_index = false;
        m_toon_index = std::make_unique<sized_index_t>(_root()->header()->texture_index_size(), m__io, this, m__root);
        break;
    }
    case 1:
    {
        n_toon_index = false;
        m_toon_index = std::make_unique<common_toon_index_t>(m__io, this, m__root);
        break;
    }
    }
    m_comment = std::make_unique<text_t>(m__io, this, m__root);
    m_face_num_vertices = m__io->read_u4le();
}

mmd_pmx_t::material_t::~material_t()
{
    _clean_up();
}

void mmd_pmx_t::material_t::_clean_up()
{
    if (!n_toon_index)
    {
    }
}

mmd_pmx_t::vec3_t::vec3_t(kaitai::kstream *p__io, kaitai::kstruct *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void mmd_pmx_t::vec3_t::_read()
{
    m_x = m__io->read_f4le();
    m_y = m__io->read_f4le();
    m_z = m__io->read_f4le();
}

mmd_pmx_t::vec3_t::~vec3_t()
{
    _clean_up();
}

void mmd_pmx_t::vec3_t::_clean_up()
{
}

mmd_pmx_t::sized_index_t::sized_index_t(uint8_t p_size, kaitai::kstream *p__io, kaitai::kstruct *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_size = p_size;
    _read();
}

void mmd_pmx_t::sized_index_t::_read()
{
    n_value = true;
    switch (size())
    {
    case 1:
    {
        n_value = false;
        m_value = m__io->read_u1();
        break;
    }
    case 2:
    {
        n_value = false;
        m_value = m__io->read_u2le();
        break;
    }
    case 4:
    {
        n_value = false;
        m_value = m__io->read_u4le();
        break;
    }
    }
}

mmd_pmx_t::sized_index_t::~sized_index_t()
{
    _clean_up();
}

void mmd_pmx_t::sized_index_t::_clean_up()
{
    if (!n_value)
    {
    }
}

mmd_pmx_t::bdef4_weights_t::bdef4_weights_t(kaitai::kstream *p__io, mmd_pmx_t::vertex_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_bone_indices = nullptr;
    m_weights = nullptr;
    _read();
}

void mmd_pmx_t::bdef4_weights_t::_read()
{
    m_bone_indices = std::make_unique<std::vector<std::unique_ptr<sized_index_t>>>();
    const int l_bone_indices = 4;
    for (int i = 0; i < l_bone_indices; i++)
    {
        m_bone_indices->push_back(std::make_unique<sized_index_t>(_root()->header()->bone_index_size(), m__io, this, m__root));
    }
    m_weights = std::make_unique<std::vector<float>>();
    const int l_weights = 4;
    for (int i = 0; i < l_weights; i++)
    {
        m_weights->push_back(std::move(m__io->read_f4le()));
    }
}

mmd_pmx_t::bdef4_weights_t::~bdef4_weights_t()
{
    _clean_up();
}

void mmd_pmx_t::bdef4_weights_t::_clean_up()
{
}

mmd_pmx_t::rigid_body_t::rigid_body_t(kaitai::kstream *p__io, mmd_pmx_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_name = nullptr;
    m_english_name = nullptr;
    m_bone_index = nullptr;
    m_position = nullptr;
    m_rotation = nullptr;
    _read();
}

void mmd_pmx_t::rigid_body_t::_read()
{
    m_name = std::make_unique<text_t>(m__io, this, m__root);
    m_english_name = std::make_unique<text_t>(m__io, this, m__root);
    m_bone_index = std::make_unique<sized_index_t>(_root()->header()->bone_index_size(), m__io, this, m__root);
    m_group_index = m__io->read_u1();
    m_group_target = m__io->read_u2le();
    m_shape_type = m__io->read_u1();
    m_width = m__io->read_f4le();
    m_height = m__io->read_f4le();
    m_depth = m__io->read_f4le();
    m_position = std::make_unique<vec3_t>(m__io, this, m__root);
    m_rotation = std::make_unique<vec3_t>(m__io, this, m__root);
    m_weight = m__io->read_f4le();
    m_position_damping = m__io->read_f4le();
    m_rotation_damping = m__io->read_f4le();
    m_restitution = m__io->read_f4le();
    m_friction = m__io->read_f4le();
    m_type = m__io->read_u1();
}

mmd_pmx_t::rigid_body_t::~rigid_body_t()
{
    _clean_up();
}

void mmd_pmx_t::rigid_body_t::_clean_up()
{
}

mmd_pmx_t::texture_t::texture_t(kaitai::kstream *p__io, mmd_pmx_t *p__parent, mmd_pmx_t *p__root) : kaitai::kstruct(p__io)
{
    m__parent = p__parent;
    m__root = p__root;
    m_name = nullptr;
    _read();
}

void mmd_pmx_t::texture_t::_read()
{
    m_name = std::make_unique<text_t>(m__io, this, m__root);
}

mmd_pmx_t::texture_t::~texture_t()
{
    _clean_up();
}

void mmd_pmx_t::texture_t::_clean_up()
{
}
