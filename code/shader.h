#ifndef _SHADER_
#define _SHADER_

#include <string>
#include <unordered_map>
#include <glm.hpp>
using namespace std;

class Shader
{

public:

	Shader(const string& vertex_shader_path, const string& fragment_shader_path);

	void bind() const; // glUseProgram ¬¬
	void unbind() const;

	void set_uniform_1i(const string& name, int i1);
	void set_uniform_1iv(const string& name, const int * v, int v_size);
	void set_uniform_4f(const string& name, float f1, float f2, float f3, float f4);
	void set_uniform_matrix4f(const string& name, const glm::mat4& matrix);

	~Shader();

private:

	unsigned int renderer_ID_;
	string vertex_shader_path_;
	string fragment_shader_path_;
	mutable unordered_map<string, GLint> uniform_location_cache;

	unsigned int compile_shader(const string& shader_source, unsigned int type);
	unsigned int create_shader(const string& vertex_shader, const string& fragment_shader);
	GLint get_uniform_location(const string& name) const;

};

#endif