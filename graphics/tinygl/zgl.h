#ifndef _tgl_zgl_h_
#define _tgl_zgl_h_

#include "common/util.h"
#include "common/textconsole.h"

#include "graphics/tinygl/gl.h"
#include "graphics/tinygl/zbuffer.h"
#include "graphics/tinygl/zmath.h"

namespace TinyGL {

enum {

#define ADD_OP(a,b,c) OP_ ## a ,

#include "graphics/tinygl/opinfo.h"

	DUMMY
};

// initially # of allocated GLVertexes (will grow when necessary)
#define POLYGON_MAX_VERTEX 16

// Max # of specular light pow buffers
#define MAX_SPECULAR_BUFFERS 8
// # of entries in specular buffer
#define SPECULAR_BUFFER_SIZE 1024
// specular buffer granularity
#define SPECULAR_BUFFER_RESOLUTION 1024

#define MAX_MODELVIEW_STACK_DEPTH   35
#define MAX_PROJECTION_STACK_DEPTH  8
#define MAX_TEXTURE_STACK_DEPTH     8
#define MAX_NAME_STACK_DEPTH        64
#define MAX_TEXTURE_LEVELS          11
#define T_MAX_LIGHTS                32

#define VERTEX_HASH_SIZE 1031

#define MAX_DISPLAY_LISTS 1024
#define OP_BUFFER_MAX_SIZE 512

#define TGL_OFFSET_FILL    0x1
#define TGL_OFFSET_LINE    0x2
#define TGL_OFFSET_POINT   0x4

struct GLSpecBuf {
	int shininess_i;
	int last_used;
	float buf[SPECULAR_BUFFER_SIZE + 1];
	struct GLSpecBuf *next;
};

struct GLLight {
	Vector4 ambient;
	Vector4 diffuse;
	Vector4 specular;
	Vector4 position;
	Vector3 spot_direction;
	float spot_exponent;
	float spot_cutoff;
	float attenuation[3];
	// precomputed values
	float cos_spot_cutoff;
	Vector3 norm_spot_direction;
	Vector3 norm_position;
	// we use a linked list to know which are the enabled lights
	int enabled;
	struct GLLight *next, *prev;
};

struct GLMaterial {
	Vector4 emission;
	Vector4 ambient;
	Vector4 diffuse;
	Vector4 specular;
	float shininess;

	// computed values
	int shininess_i;
	int do_specular;
};


struct GLViewport {
	int xmin, ymin, xsize, ysize;
	Vector3 scale;
	Vector3 trans;
	int updated;
};

union GLParam {
	int op;
	float f;
	int i;
	unsigned int ui;
	void *p;
};

struct GLParamBuffer {
	GLParam ops[OP_BUFFER_MAX_SIZE];
	struct GLParamBuffer *next;
};

struct GLList {
	GLParamBuffer *first_op_buffer;
	// TODO: extensions for an hash table or a better allocating scheme
};

struct GLVertex {
	int edge_flag;
	Vector3 normal;
	Vector4 coord;
	Vector4 tex_coord;
	Vector4 color;

	// computed values
	Vector4 ec;                // eye coordinates
	Vector4 pc;                // coordinates in the normalized volume
	int clip_code;        // clip code
	ZBufferPoint zp;      // integer coordinates for the rasterization
};

struct GLImage {
	Graphics::PixelBuffer pixmap;
	int xsize, ysize;
};

// textures

#define TEXTURE_HASH_TABLE_SIZE 256
#define BLIT_TEXTURE_MAX_COUNT 128

struct GLTexture {
	GLImage images[MAX_TEXTURE_LEVELS];
	int handle;
	struct GLTexture *next, *prev;
};


// shared state

struct GLSharedState {
	GLList **lists;
	GLTexture **texture_hash_table;
};

struct GLContext;

typedef void (*gl_draw_triangle_func)(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);

// display context

struct GLContext {
	// Z buffer
	FrameBuffer *fb;

	// Internal texture size
	int _textureSize;

	// lights
	GLLight lights[T_MAX_LIGHTS];
	GLLight *first_light;
	Vector4 ambient_light_model;
	int local_light_model;
	int lighting_enabled;
	int light_model_two_side;

	// materials
	GLMaterial materials[2];
	int color_material_enabled;
	int current_color_material_mode;
	int current_color_material_type;

	// textures
	GLTexture *current_texture;
	int texture_2d_enabled;
	void* blitTextures[BLIT_TEXTURE_MAX_COUNT];

	// shared state
	GLSharedState shared_state;

	// current list
	GLParamBuffer *current_op_buffer;
	int current_op_buffer_index;
	int exec_flag, compile_flag, print_flag;

	// matrix
	int matrix_mode;
	Matrix4 *matrix_stack[3];
	Matrix4 *matrix_stack_ptr[3];
	int matrix_stack_depth_max[3];

	Matrix4 matrix_model_view_inv;
	Matrix4 matrix_model_projection;
	int matrix_model_projection_updated;
	int matrix_model_projection_no_w_transform;
	int apply_texture_matrix;

	// viewport
	GLViewport viewport;

	// current state
	int polygon_mode_back;
	int polygon_mode_front;

	int current_front_face;
	int current_shade_model;
	int current_cull_face;
	int cull_face_enabled;
	int normalize_enabled;
	gl_draw_triangle_func draw_triangle_front, draw_triangle_back;

	// selection
	int render_mode;
	unsigned int *select_buffer;
	int select_size;
	unsigned int *select_ptr, *select_hit;
	int select_overflow;
	int select_hits;

	// names
	unsigned int name_stack[MAX_NAME_STACK_DEPTH];
	int name_stack_size;

	// clear
	float clear_depth;
	Vector4 clear_color;

	// current vertex state
	Vector4 current_color;
	unsigned int longcurrent_color[4]; // precomputed integer color
	Vector4 current_normal;
	Vector4 current_tex_coord;
	int current_edge_flag;

	// glBegin / glEnd
	int in_begin;
	int begin_type;
	int vertex_n, vertex_cnt;
	int vertex_max;
	GLVertex *vertex;

	// opengl 1.1 arrays
	float *vertex_array;
	int vertex_array_size;
	int vertex_array_stride;
	float *normal_array;
	int normal_array_stride;
	float *color_array;
	int color_array_size;
	int color_array_stride;
	float *texcoord_array;
	int texcoord_array_size;
	int texcoord_array_stride;
	int client_states;

	// opengl 1.1 polygon offset
	float offset_factor;
	float offset_units;
	int offset_states;

	int shadow_mode;

	// specular buffer. could probably be shared between contexts,
	// but that wouldn't be 100% thread safe
	GLSpecBuf *specbuf_first;
	int specbuf_used_counter;
	int specbuf_num_buffers;

	// opaque structure for user's use
	void *opaque;
	// resize viewport function
	int (*gl_resize_viewport)(GLContext *c, int *xsize, int *ysize);

	// depth test
	int depth_test;
	int color_mask;

	// alpha test
	bool _alphaTestEnabled;

	// blending
	bool enableBlend;
};

extern GLContext *gl_ctx;

void gl_add_op(GLParam *p);

// clip.c
void gl_transform_to_viewport(GLContext *c, GLVertex *v);
void gl_draw_triangle(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
void gl_draw_line(GLContext *c, GLVertex *p0, GLVertex *p1);
void gl_draw_point(GLContext *c, GLVertex *p0);

void gl_draw_triangle_point(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
void gl_draw_triangle_line(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
void gl_draw_triangle_fill(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
void gl_draw_triangle_select(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);

// matrix.c
void gl_print_matrix(const float *m);

// light.c
void gl_add_select(GLContext *c, unsigned int zmin, unsigned int zmax);
void gl_enable_disable_light(GLContext *c, int light, int v);
void gl_shade_vertex(GLContext *c, GLVertex *v);

void glInitTextures(GLContext *c);
void glEndTextures(GLContext *c);
GLTexture *alloc_texture(GLContext *c, int h);
void free_texture(GLContext *c, int h);

// image_util.c
void gl_resizeImage(unsigned char *dest, int xsize_dest, int ysize_dest,
					unsigned char *src, int xsize_src, int ysize_src);
void gl_resizeImageNoInterpolate(unsigned char *dest, int xsize_dest, int ysize_dest,
								 unsigned char *src, int xsize_src, int ysize_src);

GLContext *gl_get_context();

// specular buffer "api"
GLSpecBuf *specbuf_get_buffer(GLContext *c, const int shininess_i, const float shininess);
void specbuf_cleanup(GLContext *c); // free all memory used

void glInit(void *zbuffer, int textureSize);
void glClose();

#ifdef DEBUG
#define dprintf fprintf
#else
#define dprintf
#endif

#ifndef LOCAL_PI
#define LOCAL_PI    (3.14159265358979323846)
#endif

// glopXXX functions

#define ADD_OP(a,b,c) void glop ## a (GLContext *, GLParam *);
#include "opinfo.h"

// this clip epsilon is needed to avoid some rounding errors after
// several clipping stages

#define CLIP_EPSILON (1E-5)

static inline int gl_clipcode(float x, float y, float z, float w1) {
	float w;

	w = (float)(w1 * (1.0 + CLIP_EPSILON));
	return (x < -w) | ((x > w) << 1) | ((y < -w) << 2) | ((y > w) << 3) | ((z < -w) << 4) | ((z > w) << 5);
}

} // end of namespace TinyGL

#endif
