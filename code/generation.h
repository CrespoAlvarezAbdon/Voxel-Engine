#ifndef _GENERATION_
#define _GENERATION_

void stress_test_generation(vertexArray& va, const chunkManager& superchunk, const vertexBufferLayout& layout, glm::mat4& model_matrix, glm::mat4& model_view_projection_matrix, const Camera& main_camera,
    Shader& default_shader, const renderer& renderer);

#endif