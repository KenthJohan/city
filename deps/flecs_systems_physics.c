#define FLECS_SYSTEMS_PHYSICS_IMPL
#include "flecs_systems_physics.h"

ECS_CTOR(EcsSpatialQuery, ptr, {
    ptr->query = NULL;
})

ECS_DTOR(EcsSpatialQuery, ptr, {
    if (ptr->query) {
        ecs_squery_free(ptr->query);
    }
})

ECS_CTOR(EcsSpatialQueryResult, ptr, {
    ptr->results = NULL;
})

ECS_DTOR(EcsSpatialQueryResult, ptr, {
    if (ptr->results) {
        ecs_vector_free(ptr->results);
    }
})

static
void EcsMove2(ecs_iter_t *it) {
    EcsPosition2 *p = ecs_term(it, EcsPosition2, 1);
    EcsVelocity2 *v = ecs_term(it, EcsVelocity2, 1);

    int i;
    for (i = 0; i < it->count; i ++) {
        p[i].x += v[i].x * it->delta_time;
        p[i].y += v[i].y * it->delta_time;
    }
}

static
void EcsMove3(ecs_iter_t *it) {
    EcsPosition3 *p = ecs_term(it, EcsPosition3, 1);
    EcsVelocity3 *v = ecs_term(it, EcsVelocity3, 2);

    int i;
    for (i = 0; i < it->count; i ++) {
        p[i].x += v[i].x * it->delta_time;
        p[i].y += v[i].y * it->delta_time;
        p[i].z += v[i].z * it->delta_time;
    }
}

static
void EcsRotate3(ecs_iter_t *it) {
    EcsRotation3 *r = ecs_term(it, EcsRotation3, 1);
    EcsAngularVelocity *a = ecs_term(it, EcsAngularVelocity, 2);

    int i;
    for (i = 0; i < it->count; i ++) {
        r[i].x += a[i].x * it->delta_time;
        r[i].y += a[i].y * it->delta_time;
        r[i].z += a[i].z * it->delta_time;
    }
}

static
void EcsAddBoxCollider(ecs_iter_t *it) {
    EcsBox *box = ecs_term(it, EcsBox, 2);
    ecs_entity_t C = ecs_term_id(it, 1);
    ecs_entity_t B = ecs_term_id(it, 2);

    int i;
    if (ecs_term_is_owned(it, 2)) {
        for (i = 0; i < it->count; i ++) {
            ecs_entity_t pair = ecs_pair(C, B);
            EcsBox *collider = ecs_get_mut_id(
                it->world, it->entities[i], pair, NULL);
            ecs_os_memcpy_t(collider, &box[i], EcsBox);
        }
    } else {
        for (i = 0; i < it->count; i ++) {
            ecs_entity_t pair = ecs_pair(C, B);
            EcsBox *collider = ecs_get_mut_id(
                it->world, it->entities[i], pair, NULL);
            ecs_os_memcpy_t(collider, box, EcsBox);
        }
    }
}

static
void EcsUpdateSpatialQuery(ecs_iter_t *it) {
    EcsSpatialQuery *q = ecs_term(it, EcsSpatialQuery, 1);

    int i;
    for (i = 0; i < it->count; i ++) {
        ecs_squery_update(q->query);
    }
}

void FlecsSystemsPhysicsImport(
    ecs_world_t *world)
{
    ECS_MODULE(world, FlecsSystemsPhysics);
    ECS_IMPORT(world, FlecsComponentsTransform);
    ECS_IMPORT(world, FlecsComponentsPhysics);
    ECS_IMPORT(world, FlecsComponentsGeometry);

    ecs_set_name_prefix(world, "Ecs");

    ECS_COMPONENT_DEFINE(world, EcsSpatialQuery);
    ECS_COMPONENT_DEFINE(world, EcsSpatialQueryResult);

    ecs_set_component_actions(world, EcsSpatialQuery, {
        .ctor = ecs_ctor(EcsSpatialQuery),
        .dtor = ecs_dtor(EcsSpatialQuery)
    });

    ecs_set_component_actions(world, EcsSpatialQueryResult, {
        .ctor = ecs_ctor(EcsSpatialQueryResult),
        .dtor = ecs_dtor(EcsSpatialQueryResult)
    });    

    ECS_SYSTEM(world, EcsMove2, EcsOnUpdate, 
        [inout] flecs.components.transform.Position2,
        [in] flecs.components.physics.Velocity2);

    ECS_SYSTEM(world, EcsMove3, EcsOnUpdate, 
        [inout] flecs.components.transform.Position3,
        [in] flecs.components.physics.Velocity3);

    ECS_SYSTEM(world, EcsRotate3, EcsOnUpdate, 
        [inout] flecs.components.transform.Rotation3,
        [in] flecs.components.physics.AngularVelocity);

    ECS_SYSTEM(world, EcsAddBoxCollider, EcsPostLoad, 
        flecs.components.physics.Collider,
        flecs.components.geometry.Box(self|super),
        !(flecs.components.physics.Collider, flecs.components.geometry.Box));

    ECS_SYSTEM(world, EcsUpdateSpatialQuery, EcsPreUpdate, 
        (SpatialQuery, *), ?Prefab);
}


struct ecs_squery_t {
    ecs_query_t *q;
    ecs_octree_t *ot;
};

#define EXPR_PREFIX\
    "[in] flecs.components.transform.Position3,"\
    "[in] ANY:flecs.components.physics.Collider FOR flecs.components.geometry.Box || ANY:flecs.components.geometry.Box,"

ecs_squery_t* ecs_squery_new(
    ecs_world_t *world,
    const char *expr,
    vec3 center,
    float size)
{
    ecs_assert(world != NULL, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(expr != NULL, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(size > 0, ECS_INVALID_PARAMETER, NULL);

    ecs_squery_t *result = ecs_os_calloc(sizeof(ecs_squery_t));

    /* Prepend position to query so we have fast access to entity position when
     * updating the octree */
    char *full_expr = ecs_os_malloc(strlen(expr) + strlen(EXPR_PREFIX) + 1);
    sprintf(full_expr, EXPR_PREFIX "%s", expr);
    result->q = ecs_query_new(world, full_expr);
    result->ot = ecs_octree_new(center, size);

    ecs_assert(result->q != NULL, ECS_INTERNAL_ERROR, NULL);
    ecs_assert(result->ot != NULL, ECS_INTERNAL_ERROR, NULL);

    return result;
}

void ecs_squery_free(
    ecs_squery_t *sq)
{
    ecs_query_fini(sq->q);
    ecs_octree_free(sq->ot);
    ecs_os_free(sq);
}

void ecs_squery_update(
    ecs_squery_t *sq)
{
    ecs_assert(sq != NULL, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(sq->q != NULL, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(sq->ot != NULL, ECS_INVALID_PARAMETER, NULL);

    if (ecs_query_changed(sq->q, 0)) {
        ecs_octree_clear(sq->ot);

        const ecs_world_t *world = ecs_get_world(sq->q);
        ecs_iter_t it = ecs_query_iter(world, sq->q);
        while (ecs_query_next(&it)) {
            EcsPosition3 *p = ecs_term(&it, EcsPosition3, 1);
            EcsBox *b = ecs_term(&it, EcsBox, 2);

            if (ecs_term_is_owned(&it, 2)) {
                int i;
                for (i = 0; i < it.count; i ++) {
                    vec3 vp, vs;
                    vp[0] = p[i].x;
                    vp[1] = p[i].y;
                    vp[2] = p[i].z;

                    vs[0] = b[i].width;
                    vs[1] = b[i].height;
                    vs[2] = b[i].depth;

                    ecs_octree_insert(sq->ot, it.entities[i], vp, vs);
                }
            } else {
                int i;
                for (i = 0; i < it.count; i ++) {
                    vec3 vp, vs;
                    vp[0] = p[i].x;
                    vp[1] = p[i].y;
                    vp[2] = p[i].z;

                    vs[0] = b->width;
                    vs[1] = b->height;
                    vs[2] = b->depth;

                    ecs_octree_insert(sq->ot, it.entities[i], vp, vs);
                }
            }
        }
    }
}

void ecs_squery_findn(
    const ecs_squery_t *sq,
    vec3 position,
    float range,
    ecs_vector_t **result)
{
    ecs_assert(sq != NULL, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(sq->q != NULL, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(sq->ot != NULL, ECS_INVALID_PARAMETER, NULL);    

    ecs_octree_findn(sq->ot, position, range, result);
}


#define MAX_PER_OCTANT (8)

typedef struct cube_t {
    struct cube_t *parent;
    struct cube_t *nodes[8];
    ecs_vector_t *entities;
    int32_t id;
    bool is_leaf;
} cube_t;

struct ecs_octree_t {
    ecs_sparse_t *cubes;
    ecs_vector_t *free_cubes;
    cube_t root;
    vec3 center;
    float size;
    int32_t count;
};

static
void cube_split(
    ecs_octree_t *ot,
    cube_t *cube,
    vec3 center,
    float size);

static
cube_t *new_cube(
    ecs_octree_t *ot,
    cube_t *parent)
{
    cube_t *result;
    if (!ecs_vector_pop(ot->free_cubes, cube_t*, &result)) {
        result = ecs_sparse_add(ot->cubes, cube_t);
        result->id = (int32_t)ecs_sparse_last_id(ot->cubes);
    }
    
    result->parent = parent;
    result->is_leaf = true;

    return result;
}

static
bool is_inside_dim(
    float center,
    float size,
    float e_pos,
    float e_size)
{
    bool 
    result =  e_pos - e_size >= center - size;
    result &= e_pos + e_size <= center + size;
    return result;
}

static
bool is_inside(
    vec3 center,
    float size,
    vec3 e_pos,
    vec3 e_size)
{
    bool 
    result =  is_inside_dim(center[0], size, e_pos[0], e_size[0]);
    result &= is_inside_dim(center[1], size, e_pos[1], e_size[1]);
    result &= is_inside_dim(center[2], size, e_pos[2], e_size[2]);
    return result;
}

/* Returns 0 if no overlap, 2 if contains */
static
int overlap_dim(
    float center,
    float size,
    float pos,
    float range)
{
    float left = center - size;
    float right = center + size;
    float q_left = pos - range;
    float q_right = pos + range;

    bool left_outside = q_left < left;
    bool left_nomatch = q_left > right;

    bool right_outside = q_right > right;
    bool right_nomatch = q_right < left;

    return (!(left_nomatch | right_nomatch)) * (1 + (left_outside && right_outside));
}

/* Returns 0 if no overlap, 8 if contains, < 8 if overlaps */
static
int entity_overlaps(
    vec3 center,
    float size,
    vec3 e_pos,
    vec3 e_size)
{
    int result = overlap_dim(center[0], size, e_pos[0], e_size[0]);
    result *=    overlap_dim(center[1], size, e_pos[1], e_size[1]);
    result *=    overlap_dim(center[2], size, e_pos[2], e_size[2]);
    return result;
}

static
int cube_overlaps(
    vec3 center,
    float size,
    vec3 pos,
    float range)
{
    int result = overlap_dim(center[0], size, pos[0], range);
    result *=    overlap_dim(center[1], size, pos[1], range);
    result *=    overlap_dim(center[2], size, pos[2], range);
    return result;
}

/* Convenience macro for contains */
#define CONTAINS_CUBE (8)

static
int8_t get_side(
    float center,
    float coord)
{
    return coord > center;
}

static
int8_t next_cube_index(
    vec3 center,
    float size,
    vec3 pos)    
{
    int8_t left_right = get_side(center[0], pos[0]);
    int8_t top_bottom = get_side(center[1], pos[1]);
    int8_t front_back = get_side(center[2], pos[2]);
    center[0] += size * (left_right * 2 - 1);
    center[1] += size * (top_bottom * 2 - 1);
    center[2] += size * (front_back * 2 - 1);
    return left_right + top_bottom * 2 + front_back * 4;
}

static const vec3 cube_map[] = {
    {-1, -1, -1},
    { 1, -1, -1},
    {-1,  1, -1},
    { 1,  1, -1},
    {-1, -1,  1},
    { 1, -1,  1},
    {-1,  1,  1},
    { 1,  1,  1},
};

static
void get_cube_center(
    vec3 center,
    float size,
    int8_t index,
    vec3 result)
{
    result[0] = center[0] + cube_map[index][0] * size;
    result[1] = center[1] + cube_map[index][1] * size;
    result[2] = center[2] + cube_map[index][2] * size;
}

static
void cube_add_entity(
    cube_t *cube,
    ecs_oct_entity_t *ce)
{
    ecs_oct_entity_t *elem = ecs_vector_add(&cube->entities, ecs_oct_entity_t);
    *elem = *ce;
}

static
cube_t* cube_insert(
    ecs_octree_t *ot,
    ecs_oct_entity_t *ce,
    cube_t *cube,
    vec3 cube_center,
    float cube_size)
{
    /* create private variable that can be modified during iteration */
    vec3 center;
    glm_vec3_copy(cube_center, center);
    float size = cube_size / 2;
    cube_t *cur = cube;
    ecs_oct_entity_t e = *ce;

    if (!is_inside(center, size, ce->pos, ce->size)) {
        return NULL;
    }

    do {
        bool is_leaf = cur->is_leaf;

        /* If cube is a leaf and has space, insert */
        if (is_leaf && ecs_vector_count(cur->entities) < MAX_PER_OCTANT) {
            break;
        }

        /* Find next cube */
        vec3 child_center;
        glm_vec3_copy(center, child_center);
        float child_size = size / 2;
        int8_t cube_i = next_cube_index(child_center, child_size, e.pos);
        
        /* If entity does not fit in child cube, insert into current */
        if (!is_inside(child_center, child_size, e.pos, e.size)) {
            break;
        }

        /* Entity should not be inserted in current node. Check if node is a
         * leaf. If it is, split it up */
        if (is_leaf) {
            cube_split(ot, cur, center, size * 2);
        }

        cube_t *next = cur->nodes[cube_i];
        if (!next) {
            next = new_cube(ot, cur);
            ecs_assert(next != cur, ECS_INTERNAL_ERROR, NULL);

            cur->nodes[cube_i] = next;
            cur = next;

            /* Node is guaranteed to be empty, so stop looking */
            break;
        } else {
            ecs_assert(next != cur, ECS_INTERNAL_ERROR, NULL);
            cur = next;
        }

        size = child_size;
        glm_vec3_copy(child_center, center);
    } while (1);

    cube_add_entity(cur, &e);

    return cur;
}

static
void cube_split(
    ecs_octree_t *ot,
    cube_t *cube,
    vec3 center,
    float size)
{
    int32_t i, count = ecs_vector_count(cube->entities);

    /* This will force entities to be pushed to child nodes */
    cube->is_leaf = false; 

    for (i = 0; i < count; i ++) {
        ecs_oct_entity_t *entities = ecs_vector_first(cube->entities, ecs_oct_entity_t);
        cube_t *new_cube = cube_insert(ot, &entities[i], cube, center, size);
        ecs_assert(new_cube != NULL, ECS_INTERNAL_ERROR, NULL);

        if (new_cube != cube) {
            ecs_vector_remove(cube->entities, ecs_oct_entity_t, i);
            i --;
            count --;
            ecs_assert(count == ecs_vector_count(cube->entities), ECS_INTERNAL_ERROR, NULL);
        } else {
            ecs_vector_remove_last(cube->entities);
        }
    }

    ecs_assert(count == ecs_vector_count(cube->entities), ECS_INTERNAL_ERROR, NULL);
}

static
void result_add_entity(
    ecs_vector_t **result,
    ecs_oct_entity_t *e)
{
    ecs_oct_entity_t *elem = ecs_vector_add(result, ecs_oct_entity_t);
    *elem = *e;
}

static
void cube_find_all(
    cube_t *cube,
    ecs_vector_t **result)
{
    ecs_oct_entity_t *entities = ecs_vector_first(cube->entities, ecs_oct_entity_t);
    int32_t i, count = ecs_vector_count(cube->entities);
    for (i = 0; i < count; i ++) {
        result_add_entity(result, &entities[i]);
    }

    for (i = 0; i < 8; i ++) {
        cube_t *child = cube->nodes[i];
        if (!child) {
            continue;
        }

        cube_find_all(child, result);
    }
}

static
void cube_findn(
    cube_t *cube,
    vec3 center,
    float size,
    vec3 pos,
    float range,
    ecs_vector_t **result)
{
    size /= 2;

    ecs_oct_entity_t *entities = ecs_vector_first(cube->entities, ecs_oct_entity_t);
    int32_t i, count = ecs_vector_count(cube->entities);

    for (i = 0; i < count; i ++) {
        ecs_oct_entity_t *e = &entities[i];
        if (entity_overlaps(pos, range, e->pos, e->size)) {
            result_add_entity(result, e);
        }
    }

    for (i = 0; i < 8; i ++) {
        cube_t *child = cube->nodes[i];
        if (!child) {
            continue;
        }

        vec3 child_center;
        get_cube_center(center, size, i, child_center);
        int overlap = cube_overlaps(child_center, size, pos, range);

        if (overlap == CONTAINS_CUBE) {
            cube_find_all(child, result);
        } else if (overlap) {
            cube_findn(child, child_center, size, pos, range, result);
        }
    }
}

ecs_octree_t* ecs_octree_new(
    vec3 center,
    float size)
{
    ecs_octree_t *result = ecs_os_calloc(sizeof(ecs_octree_t));
    glm_vec3_copy(center, result->center);
    result->size = size;
    result->cubes = ecs_sparse_new(cube_t);
    return result;
}

void ecs_octree_clear(
    ecs_octree_t *ot)
{
    ecs_assert(ot != NULL, ECS_INVALID_PARAMETER, NULL);

    /* Keep existing cubes intact so that we can reuse them when the octree is
     * repopulated. This lets us keep the entity vectors, and should cause the
     * octree memory to stabilize eventually. */
    int32_t i, count = ecs_sparse_count(ot->cubes);
    for (i = 0; i < count; i ++) {
        cube_t *cube = ecs_sparse_get_dense(ot->cubes, cube_t, i);
        ecs_vector_clear(cube->entities);
        memset(cube->nodes, 0, sizeof(cube_t*) * 8);

        if (cube->parent) {
            cube_t **cptr = ecs_vector_add(&ot->free_cubes, cube_t*);
            *cptr = cube;
            cube->parent = NULL;
        }
    }

    /* Clear entities of root */
    ecs_vector_clear(ot->root.entities);
    memset(ot->root.nodes, 0, sizeof(cube_t*) * 8);
    ot->count = 0;
}

void ecs_octree_free(
    ecs_octree_t *ot)
{
}

int32_t ecs_octree_insert(
    ecs_octree_t *ot,
    ecs_entity_t e,
    vec3 e_pos,
    vec3 e_size)
{
    ecs_assert(ot != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_oct_entity_t ce;
    ce.e = e;
    glm_vec3_copy(e_pos, ce.pos);
    glm_vec3_copy(e_size, ce.size);
    cube_t *cube = cube_insert(ot, &ce, &ot->root, ot->center, ot->size);
    if (cube) {
        ot->count ++;
        return cube->id;
    } else {
        return -1;
    }
}

void ecs_octree_findn(
    ecs_octree_t *ot,
    vec3 pos,
    float range,
    ecs_vector_t **result)
{
    ecs_assert(ot != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_vector_clear(*result);
    cube_findn(&ot->root, ot->center, ot->size / 2, pos, range, result);
}

static
int cube_dump(
    cube_t *cube,
    vec3 center,
    float size)
{
    vec3 c;
    glm_vec3_copy(center, c);

    static int indent = 0;
    indent ++;

    size /= 2;
    int i, count = 0;
    for (i = 0; i < 8; i ++) {
        if (cube->nodes[i]) {
            vec3 child_center;
            get_cube_center(c, size, i, child_center);
            count += cube_dump(cube->nodes[i], child_center, size);
        }
    }
    indent --;

    return ecs_vector_count(cube->entities) + count;
}

int32_t ecs_octree_dump(
    ecs_octree_t *ot)
{
    ecs_assert(ot != NULL, ECS_INVALID_PARAMETER, NULL);
    int32_t ret = cube_dump(&ot->root, ot->center, ot->size / 2);
    printf("counted = %d, actual = %d\n", ret, ot->count);
    ecs_assert(ret == ot->count, ECS_INTERNAL_ERROR, NULL);
    return ret;
}

