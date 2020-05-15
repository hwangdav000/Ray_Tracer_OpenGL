// This template code was originally written by Matt Overby while a TA for CSci5607

// The loaders are included by glfw3 (glcorearb.h) if we are not using glew.
#include "glad/glad.h"
#include "GLFW/glfw3.h"

// Includes
#include "trimesh.hpp"
#include "shader.hpp"
#include <cstring> // memcpy
#include <cmath>

// Constants
#define WIN_WIDTH 500
#define WIN_HEIGHT 500
#define M_PI 3.14159265 

class Mat4x4 {
public:

	float m[16];

	Mat4x4(){ // Default: Identity
		m[0] = 1.f;  m[4] = 0.f;  m[8]  = 0.f;  m[12] = 0.f;
		m[1] = 0.f;  m[5] = 1.f;  m[9]  = 0.f;  m[13] = 0.f;
		m[2] = 0.f;  m[6] = 0.f;  m[10] = 1.f;  m[14] = 0.f;
		m[3] = 0.f;  m[7] = 0.f;  m[11] = 0.f;  m[15] = 1.f;
	}

	void make_identity(){
		m[0] = 1.f;  m[4] = 0.f;  m[8]  = 0.f;  m[12] = 0.f;
		m[1] = 0.f;  m[5] = 1.f;  m[9]  = 0.f;  m[13] = 0.f;
		m[2] = 0.f;  m[6] = 0.f;  m[10] = 1.f;  m[14] = 0.f;
		m[3] = 0.f;  m[7] = 0.f;  m[11] = 0.f;  m[15] = 1.f;
	}

	void print(){
		std::cout << m[0] << ' ' <<  m[4] << ' ' <<  m[8]  << ' ' <<  m[12] << "\n";
		std::cout << m[1] << ' ' <<   m[5] << ' ' <<  m[9]  << ' ' <<   m[13] << "\n";
		std::cout << m[2] << ' ' <<   m[6] << ' ' <<  m[10] << ' ' <<   m[14] << "\n";
		std::cout << m[3] << ' ' <<   m[7] << ' ' <<  m[11] << ' ' <<   m[15] << "\n";
	}

	void make_scale(float x, float y, float z){
		make_identity();
		m[0] = x; m[5] = y; m[10] = x;
	}
};

static inline const Vec3f operator*(const Mat4x4 &m, const Vec3f &v){
	Vec3f r( m.m[0]*v[0]+m.m[4]*v[1]+m.m[8]*v[2],
		m.m[1]*v[0]+m.m[5]*v[1]+m.m[9]*v[2],
		m.m[2]*v[0]+m.m[6]*v[1]+m.m[10]*v[2] );
	return r;
}

//  calculate length of vector
float calcLength(const Vec3f& v) {
	return sqrt(v[0]*v[0]+ v[1] * v[1]+ v[2] * v[2]);
}

//  calculate dot product of vector
float dotProduct(const Vec3f& v1, const Vec3f& v2) {
	return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

//  calculate the cross of 2 vec3f vectors
static const Vec3f calcCross(const Vec3f& v1, const Vec3f& v2) {
	Vec3f res;

	res[0] = v1[1] * v2[2] - v1[2] * v2[1];
	res[1] = v1[2] * v2[0] - v1[0] * v2[2];
	res[2] = v1[0] * v2[1] - v1[1] * v2[0];

	return res;
}

//  calculate the sub of v1 and v2
static const Vec3f sub(const Vec3f& v1, const Vec3f& v2) {
	Vec3f res;

	res[0] = v1[0] - v2[0];
	res[1] = v1[1] - v2[1];
	res[2] = v1[2] - v2[2];

	return res;
}
//  Calculate the add of v1 and v2
static const Vec3f add(const Vec3f& v1, const Vec3f& v2) {
	Vec3f res;

	res[0] = v1[0] + v2[0];
	res[1] = v1[1] + v2[1];
	res[2] = v1[2] + v2[2];

	return res;
}

//  calculate the multiply of v1 and v2 
static const Vec3f multiply(const Vec3f& v1, float c) {
	Vec3f res;

	res[0] = v1[0] * c;
	res[1] = v1[1] * c;
	res[2] = v1[2] * c;

	return res;
}
//  normalize vector 
static const Vec3f normalize(const Vec3f& v1) {
	Vec3f res;
	float length = calcLength(v1);
	res[0] = v1[0] / length;
	res[1] = v1[1] / length;
	res[2] = v1[2] / length;

	return res;
}

//  calculate the multiply of v1 and v2 
static const Vec3f negation(const Vec3f& v1) {
	Vec3f res;

	res[0] = -v1[0];
	res[1] = -v1[1];
	res[2] = -v1[2];

	return res;
}

//
//	Global state variables
//
namespace Globals {
	double cursorX, cursorY; // cursor positions
	float win_width, win_height; // window size
	float aspect;
	float angle = 0; // degrees
	GLuint verts_vbo[1], colors_vbo[1], normals_vbo[1], faces_ibo[1], tris_vao;
	TriMesh mesh;

	//  Model, view and projection matrices, initialized to the identity
	Mat4x4 model;
	Mat4x4 view;
	Mat4x4 projection;

	Vec3f eye;
	Vec3f viewdir;
	Vec3f updir;
	Vec3f lookat;

	float left;
	float right;
	float bottom;
	float top;
	float near;
	float far;
}


//
//	Callbacks
//
static void error_callback(int error, const char* description){ fprintf(stderr, "Error: %s\n", description); }

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	// Close on escape or Q
	if( action == GLFW_PRESS ){
		switch ( key ) {
			case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;
			case GLFW_KEY_Q: glfwSetWindowShouldClose(window, GL_TRUE); break;
            // ToDo: update the viewing transformation matrix according to key presses
			//  move eye back and foward
			case GLFW_KEY_W: Globals::eye[0] = Globals::eye[0] + .05; break;
			case GLFW_KEY_S: Globals::eye[0] = Globals::eye[0] - .05; break;
			//  move eye left and right
			case GLFW_KEY_A: Globals::eye[2] = Globals::eye[2] - .05; break;
			case GLFW_KEY_D: Globals::eye[2] = Globals::eye[2] + .05; break;
			//  move camera angle
			case GLFW_KEY_LEFT: Globals::angle = Globals::angle + 10; break;
			case GLFW_KEY_RIGHT: Globals::angle = Globals::angle - 10; break;
		}
	}
}
//TODO2
static void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	Globals::win_width = float(width);
	Globals::win_height = float(height);
    	Globals::aspect = Globals::win_width/Globals::win_height;
	
    	glViewport(0,0,width,height);

	// ToDo: update the perspective matrix according to the new window size
	//  want to re-define the left, right, bottom, and/or top extents of the viewing frustrum
		/*
				//  Projection USE VARS
		float left = Globals::left;
		float right = Globals::right;
		float bottom = Globals::bottom;
		float top = Globals::top;
		//  clipping in z
		float near = Globals::near;
		float far = Globals::far;
		
		*/
		//  if aspect_ratio is less than 1 then have height > width
		//  window should show more of the scene content in the vertical content
		//  increase the height of the viewing frustrum
		//std::cout << "width: " << Globals::win_width << "height: " << Globals::win_height << "aspect" << Globals::aspect << "\n";
		if (Globals::aspect < 1) {
			// initial width and height are 500
			Globals::top = .1 * (Globals::win_height/500);
			Globals::bottom = -.2 * (Globals::win_height / 500);
			//Globals::top = Globals::top / Globals::aspect;
			//Globals::bottom = Globals::bottom / Globals::aspect;
			Globals::left = -.1;
			Globals::right = .1;
		}
		else if (Globals::aspect > 1) {
		//  aspect_ratio greater than 1 then have width > height
		//  window should show more content horizontally
		//  modify viewing frustrum to increase its width
			Globals::left = -.1 * (Globals::win_width / 500);
			Globals::right = .1 * (Globals::win_width / 500);
			Globals::bottom = -.2;
			Globals::top = 0.1;
		
		}
		else {
		//  aspect_ratio is 1 reset vars
			Globals::left = -.1;
			Globals::right = .1;
			Globals::bottom = -.2;
			Globals::top = 0.1;
			Globals::near = 0.1;
			Globals::far = 2;
		
		}

}


// Function to set up geometry
void init_scene();


//
//	Main
//
int main(int argc, char *argv[]){

	// Load the mesh
	std::stringstream obj_file; obj_file << MY_DATA_DIR << "sibenik/sibenik.obj";
	if( !Globals::mesh.load_obj( obj_file.str() ) ){ return 0; }
	Globals::mesh.print_details();

	// Scale to fit in (-1,1): a temporary measure to allow the entire model to be visible
    	// Should be replaced by the use of an appropriate projection matrix
    	// Original model dimensions: center = (0,0,0); height: 30.6; length: 40.3; width: 17.0
    	float min, max, scale;
   	 min = Globals::mesh.vertices[0][0]; max = Globals::mesh.vertices[0][0];
	for( int i=0; i<Globals::mesh.vertices.size(); ++i ){
           if (Globals::mesh.vertices[i][0] < min) min = Globals::mesh.vertices[i][0];
           else if (Globals::mesh.vertices[i][0] > max) max = Globals::mesh.vertices[i][0];
           if (Globals::mesh.vertices[i][1] < min) min = Globals::mesh.vertices[i][1];
           else if (Globals::mesh.vertices[i][1] > max) max = Globals::mesh.vertices[i][1];
           if (Globals::mesh.vertices[i][2] < min) min = Globals::mesh.vertices[i][2];
           else if (Globals::mesh.vertices[i][2] > max) max = Globals::mesh.vertices[i][2];
    	}
    	if (min < 0) min = -min;
    	if (max > min) scale = 1/max; else scale = 1/min;
    	
	Mat4x4 mscale; mscale.make_scale( scale, scale, scale );
	for( int i=0; i<Globals::mesh.vertices.size(); ++i ){
           Globals::mesh.vertices[i] = mscale*Globals::mesh.vertices[i];
    	}

	// Set up window
	GLFWwindow* window;
	glfwSetErrorCallback(&error_callback);

	// Initialize the window
	if( !glfwInit() ){ return EXIT_FAILURE; }

	// Ask for OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Create the glfw window
	Globals::win_width = WIN_WIDTH;
	Globals::win_height = WIN_HEIGHT;
	window = glfwCreateWindow(int(Globals::win_width), int(Globals::win_height), "HW2B", NULL, NULL);
	if( !window ){ glfwTerminate(); return EXIT_FAILURE; }

	// Bind callbacks to the window
	glfwSetKeyCallback(window, &key_callback);
	glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);

	// Make current
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Initialize glew AFTER the context creation and before loading the shader.
	// Note we need to use experimental because we're using a modern version of opengl.
	
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to gladLoadGLLoader" << std::endl;
		glfwTerminate();
		return false;
	}

	// Initialize the shader (which uses glew, so we need to init that first).
	// MY_SRC_DIR is a define that was set in CMakeLists.txt which gives
	// the full path to this project's src/ directory.
	mcl::Shader shader;
	std::stringstream ss; ss << MY_SRC_DIR << "shader.";
	shader.init_from_files( ss.str()+"vert", ss.str()+"frag" );

	// Initialize the scene
	// IMPORTANT: Only call after gl context has been created
	init_scene();
	framebuffer_size_callback(window, int(Globals::win_width), int(Globals::win_height)); 

	// Initialize OpenGL
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.f,1.f,1.f,1.f);

	// Enable the shader, this allows us to set uniforms and attributes
	shader.enable();

	//TODO1
    	// Initialize the eye position (set at origin for now; you will need to change this)
	//  INIT Values
	Globals::eye = Vec3f(0.f, -.7f, 0.0f);
	Globals::updir = Vec3f(0.f, 1.f, 0.f);
	Globals::viewdir = Vec3f(-1.f, 0.f, 0.f);
	//  only need either viewdir or lookat
	//Globals::lookat = Vec3f(.3f, .1f, 0.0f);


	// Bind buffers
	glBindVertexArray(Globals::tris_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Globals::faces_ibo[0]);
    
	//  SETUP projection vars

	Globals::left = -.45;
	Globals::right = .45;
	Globals::bottom = -.45;
	Globals::top = 0.45;
	Globals::near = .7;
	Globals::far = 2;


	// Game loop
	while( !glfwWindowShouldClose(window) ){

		// Clear screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//  Projection
		float left = Globals::left;
		float right = Globals::right;
		float bottom = Globals::bottom;
		float top = Globals::top;
		//  clipping in z
		float near = Globals::near;
		float far = Globals::far;
		Mat4x4 p;
		p.m[0] = (2 * near) / (right - left);  p.m[4] = 0.f;  p.m[8] = ((right + left) / (right - left));  p.m[12] = 0.f;
		p.m[1] = 0.f;  p.m[5] = (2 * near) / (top - bottom);  p.m[9] = ((top + bottom) / (top - bottom));  p.m[13] = 0.f;
		p.m[2] = 0.f;  p.m[6] = 0.f;  p.m[10] = -((far + near) / (far - near));  p.m[14] = ((-2 * far * near) / (far - near));
		p.m[3] = 0.f;  p.m[7] = 0.f;  p.m[11] = -1.f;  p.m[15] = 0.f;

		Globals::projection = p;


		//  ANGLE 4x4
		Mat4x4 new_ang;
		float ang_radians = (Globals::angle * M_PI) / 180;
		new_ang.m[0] = cos(ang_radians);  new_ang.m[4] = 0.f;  new_ang.m[8] = sin(ang_radians);  new_ang.m[12] = 0.f;
		new_ang.m[1] = 0.f;  new_ang.m[5] = 1.f;  new_ang.m[9] = 0.f;  new_ang.m[13] = 0.f;
		new_ang.m[2] = -sin(ang_radians);  new_ang.m[6] = 0.f;  cos(ang_radians);  new_ang.m[14] = 0.f;
		new_ang.m[3] = 0.f;  new_ang.m[7] = 0.f;  new_ang.m[11] = 0.f;  new_ang.m[15] = 1.f;

		Vec3f new_vdir = new_ang * Globals::viewdir;
		//std::cout << new_vdir[0] << ' ' << new_vdir[1] << ' ' << new_vdir[2] << ' ' << "does it match" << "\n";


		//  SET UP n (-normalized viewing direction)
		//Vec3f n = negation(normalize(sub(Globals::eye, Globals::lookat)));
		Vec3f n = negation(new_vdir);
		//  SET UP u
		Vec3f u = normalize(calcCross(Globals::updir, n));
		//  SET UP v
		Vec3f v = calcCross(n, u);
		//  SET UP d
		Vec3f d;

		d[0] = -(dotProduct(Globals::eye, u));
		d[1] = -(dotProduct(Globals::eye, v));
		d[2] = -(dotProduct(Globals::eye, n));
		//  SET UP view matrix 

		// set u
		Globals::view.m[0] = u[0];
		Globals::view.m[4] = u[1];
		Globals::view.m[8] = u[2];

		//  set v
		Globals::view.m[1] = v[0];
		Globals::view.m[5] = v[1];
		Globals::view.m[9] = v[2];

		//  set n 
		Globals::view.m[2] = n[0];
		Globals::view.m[6] = n[1];
		Globals::view.m[10] = n[2];

		// set d 
		Globals::view.m[12] = d[0];
		Globals::view.m[13] = d[1];
		Globals::view.m[14] = d[2];

		// Perspective


		// Send updated info to the GPU
		glUniformMatrix4fv( shader.uniform("model"), 1, GL_FALSE, Globals::model.m  ); // model transformation
		glUniformMatrix4fv( shader.uniform("view"), 1, GL_FALSE, Globals::view.m  ); // viewing transformation
		glUniformMatrix4fv( shader.uniform("projection"), 1, GL_FALSE, Globals::projection.m ); // projection matrix

		// Draw
		glDrawElements(GL_TRIANGLES, Globals::mesh.faces.size()*3, GL_UNSIGNED_INT, 0);

		// Finalize
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // end game loop

	// Unbind
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Disable the shader, we're done using it
	shader.disable();
    
	return EXIT_SUCCESS;
}


void init_scene(){

	using namespace Globals;

	// Create the buffer for vertices
	glGenBuffers(1, verts_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size()*sizeof(mesh.vertices[0]), &mesh.vertices[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for colors
	glGenBuffers(1, colors_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.colors.size()*sizeof(mesh.colors[0]), &mesh.colors[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for normals
	glGenBuffers(1, normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.normals.size()*sizeof(mesh.normals[0]), &mesh.normals[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for indices
	glGenBuffers(1, faces_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_ibo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.faces.size()*sizeof(mesh.faces[0]), &mesh.faces[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Create the VAO
	glGenVertexArrays(1, &tris_vao);
	glBindVertexArray(tris_vao);

	int vert_dim = 3;

	// location=0 is the vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
	glVertexAttribPointer(0, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.vertices[0]), 0);

	// location=1 is the color
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
	glVertexAttribPointer(1, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.colors[0]), 0);

	// location=2 is the normal
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
	glVertexAttribPointer(2, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.normals[0]), 0);

	// Done setting data for the vao
	glBindVertexArray(0);

}

