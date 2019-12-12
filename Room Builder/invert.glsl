#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 offset;
uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
mat4 model_;
void main()
{
	model_ = model;
	model_[3][0] += offset.x;
	model_[3][1] += offset.y;
	model_[3][2] += offset.z;
	gl_Position = projection * view * model_ * vec4(pos, 1.0);
}
